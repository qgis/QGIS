/***************************************************************************
  qgssettingsregistry.h
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGSREGISTRY_H
#define QGSSETTINGSREGISTRY_H

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"
#include "qgssettingsentry.h"

/**
 * \ingroup core
 * \class QgsSettingsRegistry
 *
 * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsRegistry : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSettingsRegistry.
     */
    QgsSettingsRegistry( QgsSettings::Section settingsSection,
                         QObject *parent = nullptr );

    void registerValue( const QString &settingsName,
                        const QVariant &defaultValue = QVariant(),
                        const QString &description = QString() );

    void registerValueString( const QString &settingsName,
                              const QString &defaultValue = QString(),
                              const QString &description = QString(),
                              int minLength = 0,
                              int maxLength = 1 << 30 );

    bool isRegistered( const QString &settingsName ) const;

    void unregister( const QString &settingsName );

    bool setValue( const QString &settingsName,
                   const QVariant &value );

#ifdef SIP_RUN
    QVariant value( const QString &settingsName ) const;
#else
    template <class T>
    T value( const QString &settingsName ) const
    {
      if ( isRegistered( settingsName ) == false )
      {
        QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
        return T();
      }

      return mMapSettingsEntry.value( settingsName ).value <T> ();
    }
#endif

#ifdef SIP_RUN
    QVariant defaultValue( const QString &settingsName ) const;
#else
    template <class T>
    T defaultValue( const QString &settingsName ) const
    {
      if ( isRegistered( settingsName ) == false )
      {
        QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
        return T();
      }

      return mMapSettingsEntry.value( settingsName ).defaultValue <T> ();
    }
#endif

    QString description( const QString &settingsName ) const;

  private:

    QgsSettings::Section mSettingsSection;

    QMap<QString, QgsSettingsEntry> mMapSettingsEntry;
};

#endif // QGSSETTINGSREGISTRY_H
