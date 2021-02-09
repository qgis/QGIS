/***************************************************************************
  qgssettingsentry.h
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

#ifndef QGSSETTINGSENTRY_H
#define QGSSETTINGSENTRY_H

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"

/**
 * \ingroup core
 * \class QgsSettingsEntry
 *
 * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsEntry : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSettingsEntry.
     */
    QgsSettingsEntry( QString settingsName = QString(),
                      QgsSettings::Section settingsSection = QgsSettings::NoSection,
                      QVariant defaultValue = QVariant(),
                      QString description = QString(),
                      QObject *parent = nullptr );

    QgsSettingsEntry( const QString &settingsName,
                      QgsSettings::Section settingsSection,
                      const QString &defaultValue,
                      const QString &description = QString(),
                      int minLength = 0,
                      int maxLength = -1,
                      QObject *parent = nullptr );

    QgsSettingsEntry( const QgsSettingsEntry &other );

    QgsSettingsEntry &operator=( const QgsSettingsEntry &other );

    void setValue( const QVariant &value );

#ifdef SIP_RUN
    QVariant value() const;
#else
    template <class T>
    T value() const
    {
      QVariant variantValue = QgsSettings().value( mSettingsName,
                              mDefaultValue,
                              mSettingsSection );
      if ( variantValue.canConvert<T>() == false )
        QgsDebugMsg( QObject::tr( "Can't convert setting '%1' to type '%2'" )
                     .arg( mSettingsName )
                     .arg( typeid( T ).name() ) );

      return variantValue.value<T>();
    }
#endif

#ifdef SIP_RUN
    QVariant defaultValue() const;
#else
    template <class T>
    T defaultValue() const
    {
      if ( mDefaultValue.canConvert<T>() == false )
        QgsDebugMsg( QObject::tr( "Can't convert default value of setting '%1' to type '%2'" )
                     .arg( mSettingsName )
                     .arg( typeid( T ).name() ) );

      return mDefaultValue.value<T>();
    }
#endif

    QString description() const;

  private:

    QString mSettingsName;
    QVariant mDefaultValue;
    QgsSettings::Section mSettingsSection;
    QString mDescription;

    int mValueStringMinLength;
    int mValueStringMaxLength;
};

#endif // QGSSETTINGSENTRY_H
