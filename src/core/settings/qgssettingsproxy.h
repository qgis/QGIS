/***************************************************************************
  qgssettingsproxy.h
  --------------------------------------
  Date                 : January 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSPROXY_H
#define QGSSETTINGSPROXY_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgssettings.h"
#include <optional>

/**
 * \class QgsSettingsProxy
 * \ingroup core
 *
 * \brief A helper class for access to either a temporary QgsSettings object or the thread local object.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSettingsProxy
{
  public:

    /**
     * Constructor for QgsSettingsProxy.
     *
     * If \a settings is set, then this object will proxy calls to that settings object.
     * Otherwise a temporary QgsSettings object will be created for the lifetime of the proxy.
     */
    explicit QgsSettingsProxy( QgsSettings *settings = nullptr );

    /**
     * Returns a pointer to the proxied QgsSettings object.
     */
    QgsSettings *operator->()
    {
      return mOwnedSettings.has_value() ? &( mOwnedSettings.value() ) : mNonOwnedSettings;
    }

    /**
     * Returns a reference to the proxied QgsSettings object.
     */
    QgsSettings &operator* ()
    {
      return mOwnedSettings.has_value() ? mOwnedSettings.value() : *mNonOwnedSettings;
    }

  private:

    QgsSettings *mNonOwnedSettings = nullptr;
    std::optional< QgsSettings > mOwnedSettings;

};
#endif // QGSSETTINGSPROXY_H
