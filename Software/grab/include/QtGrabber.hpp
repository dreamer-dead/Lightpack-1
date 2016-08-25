/*
 * QtGrabber.hpp
 *
 *  Created on: 25.07.11
 *     Project: Lightpack
 *
 *  Copyright (c) 2011 Timur Sattarov, Mike Shatohin
 *
 *  Lightpack a USB content-driving ambient lighting system
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "GrabberBase.hpp"

#ifdef QT_GRAB_SUPPORT

using namespace Grab;

class QtGrabber : public GrabberBase
{
public:
    QtGrabber(QObject *parent, GrabberContext * context);
    virtual ~QtGrabber();

    DECLARE_GRABBER_NAME("QtGrabber")
    virtual void updateGrabMonitor( QWidget * widget );

protected:
    virtual GrabResult _grab(QList<QRgb> &grabResult, const QList<GrabWidget *> &grabWidget);

private:
    QRgb getColor(QPixmap pixmap, const QWidget * grabme);
    QRgb getColor(QPixmap pixmap, int x, int y, int width, int height);

    QRect screenres;
    int screen;
};

#endif // QT_GRAB_SUPPORT
