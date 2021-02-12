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
  public:

    /**
     * Constructor for QgsSettingsRegistry.
     */
    QgsSettingsRegistry( QgsSettings::Section section );
    ~QgsSettingsRegistry() override;

    void registerSettings( const QString &key,
                           const QVariant &defaultValue = QVariant(),
                           const QString &description = QString() );

    void registerSettingsString( const QString &key,
                                 const QString &defaultValue = QString(),
                                 const QString &description = QString(),
                                 int minLength = 0,
                                 int maxLength = -1 );

    void registerSettingsInteger( const QString &key,
                                  qlonglong defaultValue = 0,
                                  const QString &description = QString(),
                                  qlonglong minValue = -__LONG_LONG_MAX__ + 1,
                                  qlonglong maxValue = __LONG_LONG_MAX__ );

    void registerSettingsDouble( const QString &key,
                                 double defaultValue = 0.0,
                                 const QString &description = QString(),
                                 double minValue = __DBL_MIN__,
                                 double maxValue = __DBL_MAX__,
                                 double displayDecimals = 1 );

#ifndef SIP_RUN
    template <class T>
    void registerSettingsEnum( const QString &key,
                               const T &defaultValue,
                               const QString &description = QString() )
    {
      if ( isRegistered( key ) == true )
      {
        QgsDebugMsg( QObject::tr( "Settings key '%1' already registered" ).arg( key ) );
        return;
      }

      mMapSettingsEntry.insert( key,
                                new QgsSettingsEntryEnum(
                                  key,
                                  mSection,
                                  defaultValue,
                                  description ) );
    }
#endif

    bool isRegistered( const QString &key ) const;

    void unregister( const QString &key );

    QgsSettingsEntry *settingsEntry( const QString &key ) const;

    bool setValue( const QString &key,
                   const QVariant &value );

    QVariant valueFromPython( const QString &key ) const SIP_PYNAME( value );

#ifndef SIP_RUN
    template <class T>
    T value( const QString &key ) const
    {
      if ( isRegistered( key ) == false )
      {
        QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( key ) );
        return T();
      }

      return mMapSettingsEntry.value( key )->value <T> ();
    }
#endif

    QVariant defaultValueFromPython( const QString &key ) const SIP_PYNAME( defaultValue );

#ifndef SIP_RUN
    template <class T>
    T defaultValue( const QString &key ) const
    {
      if ( isRegistered( key ) == false )
      {
        QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( key ) );
        return T();
      }

      return mMapSettingsEntry.value( key )->defaultValue <T> ();
    }
#endif

    QString description( const QString &key ) const;

  private:

    QgsSettings::Section mSection;

    QMap<QString, QgsSettingsEntry * > mMapSettingsEntry;
};

#endif // QGSSETTINGSREGISTRY_H
