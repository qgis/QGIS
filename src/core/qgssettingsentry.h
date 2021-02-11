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
    QgsSettingsEntry( QString key = QString(),
                      QgsSettings::Section settingsSection = QgsSettings::NoSection,
                      QVariant defaultValue = QVariant(),
                      QString description = QString(),
                      QObject *parent = nullptr );

    QgsSettingsEntry( const QgsSettingsEntry &other );

    /**
     * Copy constructor for QgsSettingsEntry.
     */
    QgsSettingsEntry &operator=( const QgsSettingsEntry &other );

    /**
     * Set settings value.
     */
    virtual bool setValue( const QVariant &value );

    /**
     * Get settings value.
     */
    QVariant valueFromPython() const SIP_PYNAME( value );

#ifndef SIP_RUN
    template <class T>
    T value() const
    {
      QVariant variantValue = QgsSettings().value( mKey,
                              mDefaultValue,
                              mSection );
      if ( variantValue.canConvert<T>() == false )
        QgsDebugMsg( QObject::tr( "Can't convert setting '%1' to type '%2'" )
                     .arg( mKey )
                     .arg( typeid( T ).name() ) );

      return variantValue.value<T>();
    }
#endif

    /**
     * Get settings default value.
     */
    QVariant defaultValueFromPython() const SIP_PYNAME( defaultValue );

#ifndef SIP_RUN
    template <class T>
    T defaultValue() const
    {
      if ( mDefaultValue.canConvert<T>() == false )
        QgsDebugMsg( QObject::tr( "Can't convert default value of setting '%1' to type '%2'" )
                     .arg( mKey )
                     .arg( typeid( T ).name() ) );

      return mDefaultValue.value<T>();
    }
#endif

    QString description() const;

    void remove();

  private:

    QString mKey;
    QVariant mDefaultValue;
    QgsSettings::Section mSection;
    QString mDescription;

};

/**
 * \class QgsSettingsEntryString
 * \ingroup core
 * A string settings entry.
  * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsEntryString : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryString.
     */
    QgsSettingsEntryString( const QString &key,
                            QgsSettings::Section section,
                            const QString &defaultValue,
                            const QString &description = QString(),
                            int minLength = 0,
                            int maxLength = -1,
                            QObject *parent = nullptr );

    bool setValue( const QVariant &value ) override;

    /**
     * Returns the string minimum length.
     */
    int minLength();

    /**
     * Returns the string maximum length. By -1 there is no limitation.
     */
    int maxLength();

  private:

    int mMinLength;
    int mMaxLength;

};




#endif // QGSSETTINGSENTRY_H
