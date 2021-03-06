/*
 * SettingsSource.hpp
 *
 *  Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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

#ifndef SETTINGSPROFILES_HPP
#define SETTINGSPROFILES_HPP

#include "ConfigurationProfile.hpp"

namespace SettingsScope {

class SettingsProfiles {
public:
    QVariant valueMain(const QString & key) const;
    QVariant valueMain(const QString & key, const QVariant& defaultValue) const;
    QVariant value(const QString & key) const;
    QVariant value(const QString & key, const QVariant& defaultValue) const;

protected:
    // forwarding to m_mainConfig object
    void setValueMain(const QString & key, const QVariant & value);
    // forwarding to m_currentProfile object
    void setValue(const QString & key, const QVariant & value);

    ConfigurationProfile m_mainProfile;
    ConfigurationProfile m_currentProfile;
};

}  // namespace SettingsScope

#endif // SETTINGSPROFILES_HPP
