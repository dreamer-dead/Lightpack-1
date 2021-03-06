#include "LightpackPluginInterface.hpp"

#include <QtGui>
#include <QtWidgets/QApplication>

#include "common/DebugOut.hpp"
#include "Plugin.hpp"
#include "Settings.hpp"
#include "SettingsDefaults.hpp"
#include "version.h"

using namespace SettingsScope;

namespace {
static const int kSignalWaitTimeoutMs = 1000;  // 1 second
}

LightpackPluginInterface::LightpackPluginInterface(QObject *parent) :
    QObject(parent)
{
    m_isRequestBacklightStatusDone = true;
    m_backlightStatusResult = Backlight::StatusUnknown;
    initColors(10);
    m_timerLock = new QTimer(this);
    m_timerLock->start(5000); // check in 5000 ms
    connect(m_timerLock, SIGNAL(timeout()), this, SLOT(timeoutLock()));
    _plugins.clear();
}

LightpackPluginInterface::~LightpackPluginInterface()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    delete m_timerLock;
}

//TODO timeout lock
void LightpackPluginInterface::timeoutLock()
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    if (lockAlive)
    {
        lockAlive = false;
    }
    else
    {
        if (!lockSessionKeys.isEmpty())
            if (lockSessionKeys[0].indexOf("API", 0) == -1)
                UnLock(lockSessionKeys[0]);
    }
}

void LightpackPluginInterface::updatePlugin(QList<Plugin*> plugins)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    if (!lockSessionKeys.isEmpty())
        UnLock(lockSessionKeys[0]);
    lockSessionKeys.clear();
    //emit updateDeviceLockStatus(DeviceLocked::Unlocked, lockSessionKeys);
    _plugins = plugins;

}

bool LightpackPluginInterface::VerifySessionKey(QString sessionKey)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << sessionKey;

    foreach(Plugin* plugin, _plugins){
            if (plugin->Guid() == sessionKey)
                return true;
        }
    return false;
}

Plugin* LightpackPluginInterface::findName(QString name)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << name;
    foreach(Plugin* plugin, _plugins){
        if (plugin->Name() == name)
            return plugin;
    }

    return NULL;
}

Plugin* LightpackPluginInterface::findSessionKey(QString sessionKey)
{
    foreach(Plugin* plugin, _plugins){
        if (plugin->Guid() == sessionKey)
            return plugin;
    }

    return NULL;
}


void LightpackPluginInterface::initColors(int numberOfLeds)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;
    m_curColors.clear();
    m_setColors.clear();
    for (int i = 0; i < numberOfLeds; i++)
    {
        m_setColors << 0;
        m_curColors << 0;
    }
}

void LightpackPluginInterface::setNumberOfLeds(int numberOfLeds)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

    initColors(numberOfLeds);
}

void LightpackPluginInterface::resultBacklightStatus(Backlight::Status status)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << status;
    m_backlightStatusResult = status;
    m_isRequestBacklightStatusDone = true;
    switch (m_backlightStatusResult)
    {
        case Backlight::StatusOn:
            emit ChangeStatus(1);
            break;
        case Backlight::StatusOff:
            emit ChangeStatus(0);
            break;
        case Backlight::StatusDeviceError:
        default:
            emit ChangeStatus(-1);
            break;
    }
}

void LightpackPluginInterface::changeProfile(QString profile)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << profile;

    emit ChangeProfile(profile);
}

void LightpackPluginInterface::refreshAmbilightEvaluated(double updateResultMs)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << updateResultMs;

    double secs = updateResultMs / 1000;
    hz = 0;

    if(secs != 0){
        hz = 1 / secs;
    }
}

void LightpackPluginInterface::refreshScreenRect(QRect rect)
{
    screen = rect;
}

void LightpackPluginInterface::updateColors(const QList<QRgb> & colors)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO;
    m_curColors = colors;
}

QString LightpackPluginInterface::Version()
{
    return API_VERSION;
}

// TODO identification plugin locked
QString LightpackPluginInterface::GetSessionKey(QString module)
{
    if (module=="API") return "Lock";
    Plugin* plugin = findName(module);
    if (plugin == NULL) return "";
        return plugin->Guid();

}

int LightpackPluginInterface::CheckLock(QString sessionKey)
{
    if (lockSessionKeys.isEmpty())
        return 0;
    if (lockSessionKeys[0]==sessionKey)
        return 1;
    return -1;
}


//TODO: lock unlock
bool LightpackPluginInterface::Lock(QString sessionKey)
{
    if (sessionKey == "") return false;
        if (lockSessionKeys.contains(sessionKey)) return true;
        if (sessionKey.indexOf("API", 0) != -1)
            {
                if (lockSessionKeys.count()>0)
                    return false;
                lockSessionKeys.insert(0,sessionKey);
                emit updateDeviceLockStatus(DeviceLocked::Api,lockSessionKeys);
            }
            else
            {
              Plugin* plugin = findSessionKey(sessionKey);
              if (plugin == NULL) return false;

                  foreach (QString key,lockSessionKeys)
                  {
                      if (key.indexOf("API", 0) != -1) break;

                             Plugin* pluginLock = findSessionKey(key);
                             if (pluginLock == NULL) return false;
                             DEBUG_LOW_LEVEL << Q_FUNC_INFO << lockSessionKeys.indexOf(key);
                             if (plugin->getPriority() > pluginLock->getPriority())
                                 lockSessionKeys.insert(lockSessionKeys.indexOf(key),sessionKey);
                             DEBUG_LOW_LEVEL << Q_FUNC_INFO << lockSessionKeys.indexOf(sessionKey);

                  }
                  if (!lockSessionKeys.contains(sessionKey))
                  {
                      DEBUG_LOW_LEVEL << Q_FUNC_INFO << "add to end";
                      lockSessionKeys.insert(lockSessionKeys.count(),sessionKey);
                  }
                  if (lockSessionKeys[0].indexOf("API", 0) != -1)
                      emit updateDeviceLockStatus(DeviceLocked::Api,lockSessionKeys);
                  else
                      emit updateDeviceLockStatus(DeviceLocked::Plugin, lockSessionKeys);
}
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "lock end";
        lockAlive = true;
        emit ChangeLockStatus (true);
        return true;

}

bool LightpackPluginInterface::UnLock(QString sessionKey)
{
    if (lockSessionKeys.isEmpty()) return false;
        if (lockSessionKeys[0]==sessionKey)
        {
            lockSessionKeys.removeFirst();
            if (lockSessionKeys.count()==0)
            {
                emit updateDeviceLockStatus(DeviceLocked::Unlocked, lockSessionKeys);
                emit ChangeLockStatus(false);
            }
            else
            {
                if (lockSessionKeys[0].indexOf("API", 0) != -1)
                    emit updateDeviceLockStatus(DeviceLocked::Api,lockSessionKeys);
                else
                    emit updateDeviceLockStatus(DeviceLocked::Plugin, lockSessionKeys);
            }
            return true;
        }
        else
            if (lockSessionKeys.indexOf(sessionKey)!= -1)
            {
               lockSessionKeys.removeOne(sessionKey);
               if (lockSessionKeys[0].indexOf("API", 0) != -1)
                   emit updateDeviceLockStatus(DeviceLocked::Api,lockSessionKeys);
               else
                   emit updateDeviceLockStatus(DeviceLocked::Plugin, lockSessionKeys);
               return true;
            }
        return false;

}


void LightpackPluginInterface::SetLockAlive(QString sessionKey)
{
    if (lockSessionKeys.isEmpty()) return;
    if (lockSessionKeys[0]!=sessionKey) return;
       lockAlive = true;
}

// TODO: setcolor
bool LightpackPluginInterface::SetColors(QString sessionKey, int r, int g, int b)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
     lockAlive = true;
    for (int i = 0; i < m_setColors.size(); i++)
    {
            m_setColors[i] = qRgb(r,g,b);
    }
    m_curColors = m_setColors;
    emit updateLedsColors(m_setColors);
    return true;
}

bool LightpackPluginInterface::SetFrame(QString sessionKey, QList<QColor> colors)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    lockAlive = true;
    int availSize = colors.size() < m_setColors.size() ? colors.size() : m_setColors.size();
    for (int i = 0; i < availSize; i++)
    {
            m_setColors[i] = colors[i].rgb();
    }
    m_curColors = m_setColors;
    emit updateLedsColors(m_setColors);
    return true;
}

bool LightpackPluginInterface::SetColor(QString sessionKey, int ind,int r, int g, int b)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << sessionKey;
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    lockAlive = true;
    if (ind>m_setColors.size()-1) return false;
    m_setColors[ind] = qRgb(r,g,b);
    m_curColors = m_setColors;
    emit updateLedsColors(m_setColors);
    return true;
}

bool LightpackPluginInterface::SetGamma(QString sessionKey, double gamma)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
     if (gamma >= Profile::Device::GammaMin && gamma <= Profile::Device::GammaMax)
     {
         emit updateGamma(gamma);
         return true;
     } else
         return false;
}

bool LightpackPluginInterface::SetBrightness(QString sessionKey, int brightness)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
     if (brightness >= Profile::Device::BrightnessMin && brightness <= Profile::Device::BrightnessMax)
     {
         emit updateBrightness(brightness);
         return true;
     } else
         return false;
}

bool LightpackPluginInterface::SetSmooth(QString sessionKey, int smooth)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
     if (smooth >= Profile::Device::SmoothMin && smooth <= Profile::Device::SmoothMax)
     {
             emit updateSmooth(smooth);
             return true;
      } else
         return false;
}

bool LightpackPluginInterface::SetProfile(QString sessionKey,QString profile)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    QStringList profiles = Settings::instance()->findAllProfiles();
    if (profiles.contains(profile))
    {
        emit updateProfile(profile);
        return true;
    } else
        return false;
}

bool LightpackPluginInterface::SetDevice(QString sessionKey,QString device)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    QStringList devices = Settings::instance()->getSupportedDevices();
    if (devices.contains(device))
    {
        //Settings::instance()->setConnectedDeviceName(device);
        emit changeDevice(device);
        return true;
    } else
        return false;
}

bool LightpackPluginInterface::SetStatus(QString sessionKey, int status)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << status;
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
     Backlight::Status statusSet = Backlight::StatusUnknown;

     if (status == 1)
         statusSet = Backlight::StatusOn;
     else if (status == 0)
         statusSet = Backlight::StatusOff;

     if (statusSet != Backlight::StatusUnknown)
     {
         emit updateStatus(statusSet);
         return true;
     } else
         return false;
}

bool LightpackPluginInterface::SetLeds(QString sessionKey, QList<QRect> leds)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    int num =0;
     foreach(QRect rectLed, leds){
        Settings::instance()->setLedPosition(num, QPoint(rectLed.x(),rectLed.y()));
        Settings::instance()->setLedSize(num,QSize(rectLed.width(),rectLed.height()));
        ++num;
     }
    emit updateCountLeds(leds.count());
    QString profile = Settings::instance()->getCurrentProfileName();
    emit updateProfile(profile);
    return true;
}

bool LightpackPluginInterface::NewProfile(QString sessionKey, QString profile)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;

    Settings::instance()->loadOrCreateProfile(profile);
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "OK:" << profile;
    emit updateProfile(profile);

    return true;
}

bool LightpackPluginInterface::DeleteProfile(QString sessionKey, QString profile)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    QStringList profiles = Settings::instance()->findAllProfiles();
    if (profiles.contains(profile))
    {
        Settings::instance()->loadOrCreateProfile(profile);
        Settings::instance()->removeCurrentProfile();
        QString profileLast = Settings::instance()->getLastProfileName();
        emit updateProfile(profileLast);
        return true;
    }
    else
        return false;
}

bool LightpackPluginInterface::SetBacklight(QString sessionKey, int backlight)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;
    Lightpack::Mode status =  Lightpack::UnknownMode;

    if (backlight == 1)
        status = Lightpack::AmbilightMode;
    else if (backlight == 2)
        status = Lightpack::MoodLampMode;

    if (status != Lightpack::UnknownMode)
    {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "OK:" << status;

        emit updateBacklight(status);
        return true;
    }
    return false;
}

bool LightpackPluginInterface::SetCountLeds(QString sessionKey, int countLeds)
{
    if (lockSessionKeys.isEmpty()) return false;
    if (lockSessionKeys[0]!=sessionKey) return false;

    Settings::instance()->setNumberOfLeds(Settings::instance()->getConnectedDevice(), countLeds);
    emit updateCountLeds(countLeds);
    return true;

}

int LightpackPluginInterface::GetCountLeds()
{
    return Settings::instance()->getNumberOfLeds(Settings::instance()->getConnectedDevice());
}

int LightpackPluginInterface::GetStatus()
{
    if (m_isRequestBacklightStatusDone)
    {
        m_isRequestBacklightStatusDone = false;
        m_backlightStatusResult = Backlight::StatusUnknown;

        emit requestBacklightStatus();

        // Wait signal from SettingsWindow with status of backlight
        // or if timeout -- result will be unknown
        m_time.restart();
        while (m_isRequestBacklightStatusDone == false && m_time.elapsed() < kSignalWaitTimeoutMs)
        {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents, kSignalWaitTimeoutMs);
        }

        if (m_isRequestBacklightStatusDone)
        {
            switch (m_backlightStatusResult)
            {
                case Backlight::StatusOn:
                    return 1;
                    break;
                case Backlight::StatusOff:
                    return 0;
                    break;
                case Backlight::StatusDeviceError:
                    return -1;
                    break;
                default:
                    return -2;
                    break;
            }
        } else {
            m_isRequestBacklightStatusDone = true;
            return -2;
        }
    }
   return -2;
}

bool LightpackPluginInterface::GetStatusAPI()
{
    return (!lockSessionKeys.isEmpty());
}

QStringList LightpackPluginInterface::GetProfiles()
{
    return Settings::instance()->findAllProfiles();
}

QString LightpackPluginInterface::GetProfile()
{
    return Settings::instance()->getCurrentProfileName();
}

QList<QRect> LightpackPluginInterface::GetLeds()
{
    QList<QRect> leds;
    for (int i = 0; i < Settings::instance()->getNumberOfLeds(Settings::instance()->getConnectedDevice()); i++)
    {
        QPoint top = Settings::instance()->getLedPosition(i);
        QSize size = Settings::instance()->getLedSize(i);
        leds << QRect(top.x(),top.y(),size.width(),size.height());
    }
    return leds;
}

QList<QRgb> LightpackPluginInterface::GetColors()
{
   return m_curColors;
}

double LightpackPluginInterface::GetFPS()
{
    return hz;
}

QRect LightpackPluginInterface::GetScreenSize()
{
    return screen;
}

int LightpackPluginInterface::GetBacklight()
{
    const Lightpack::Mode mode =  Settings::instance()->getLightpackMode();

    switch (mode)
    {
    case Lightpack::AmbilightMode:
        return 1;
        break;
    case Lightpack::MoodLampMode:
        return 2;
        break;
    default:
        qWarning() << "Unsupported Lightpack::Mode: " << (int)mode;
        // TODO: use more suitable value.
        return 0;
    }
}

QString LightpackPluginInterface::GetPluginsDir()
{
    return QString(Settings::instance()->getApplicationDirPath() + "Plugins");
}

