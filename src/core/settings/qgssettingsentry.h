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
#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"

class QgsSettingsGroup;

/**
 * \ingroup core
 * \class QgsSettingsEntry
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntry
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsSettingsEntryString * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryString;
    if ( dynamic_cast< QgsSettingsEntryStringList * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryStringList;
    else if ( dynamic_cast< QgsSettingsEntryInteger * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryInteger;
    else if ( dynamic_cast< QgsSettingsEntryDouble * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryDouble;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    enum SettingsType
    {
      Variant,
      String,
      StringList,
      Bool,
      Integer,
      Double,
      Enum
    };

    /**
     * Constructor for QgsSettingsEntry.
     */
    QgsSettingsEntry( QString key = QString(),
                      QgsSettingsGroup *settingsGroupParent = nullptr,
                      QVariant defaultValue = QVariant(),
                      QString description = QString() );

    /**
     * Destructor for QgsSettingsEntry.
     */
    virtual ~QgsSettingsEntry();

    /**
     * Get settings key.
     */
    QString key() const;

    /**
     * Get settings section.
     */
    QgsSettings::Section section();

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

    virtual SettingsType settingsType() const;

    QString description() const;

  private:

    QString mKey;
    QgsSettingsGroup *mSettingsGroupParent;
    QVariant mDefaultValue;
    QgsSettings::Section mSection;
    QString mDescription;

};

/**
 * \class QgsSettingsEntryString
 * \ingroup core
 * A string settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryString : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryString.
     */
    QgsSettingsEntryString( const QString &key = QString(),
                            QgsSettingsGroup *settingsGroupParent = nullptr,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            int minLength = 0,
                            int maxLength = -1 );

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

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

/**
 * \class QgsSettingsEntryStringList
 * \ingroup core
 * A string list settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryStringList : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryStringList.
     */
    QgsSettingsEntryStringList( const QString &key = QString(),
                                QgsSettingsGroup *settingsGroupParent = nullptr,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString() );

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

};

/**
 * \class QgsSettingsEntryBool
 * \ingroup core
 * A boolean settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryBool : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryBool.
     */
    QgsSettingsEntryBool( const QString &key = QString(),
                          QgsSettingsGroup *settingsGroupParent = nullptr,
                          bool defaultValue = false,
                          const QString &description = QString() );

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

};

/**
 * \class QgsSettingsEntryInteger
 * \ingroup core
 * An integer settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryInteger : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryInteger.
     */
    QgsSettingsEntryInteger( const QString &key = QString(),
                             QgsSettingsGroup *settingsGroupParent = nullptr,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             qlonglong minValue = -__LONG_LONG_MAX__ + 1,
                             qlonglong maxValue = __LONG_LONG_MAX__ );

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

    /**
     * Returns the minimum value.
     */
    qlonglong minValue();

    /**
     * Returns the maximum value.
     */
    qlonglong maxValue();

  private:

    qlonglong mMinValue;
    qlonglong mMaxValue;

};

/**
 * \class QgsSettingsEntryDouble
 * \ingroup core
 * A double settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryDouble : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryDouble.
     */
    QgsSettingsEntryDouble( const QString &key = QString(),
                            QgsSettingsGroup *settingsGroupParent = nullptr,
                            double defaultValue = 0.0,
                            const QString &description = QString(),
                            double minValue = __DBL_MIN__,
                            double maxValue = __DBL_MAX__,
                            double displayDecimals = 1 );

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

    /**
     * Returns the minimum value.
     */
    double minValue() const;

    /**
     * Returns the maximum value.
     */
    double maxValue() const;

    /**
     * Returns how much decimals should be shown in the Gui.
     */
    int displayHintDecimals() const;

  private:

    double mMinValue;
    double mMaxValue;

    int mDisplayHintDecimals;

};

#ifndef SIP_RUN

/**
 * \class QgsSettingsEntryEnum
 * \ingroup core
 * An enum settings entry.
  * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsEntryEnum : public QgsSettingsEntry
{
  public:

    /**
     * Constructor for QgsSettingsEntryEnum.
     */
    template <class T>
    QgsSettingsEntryEnum( const QString &key,
                          QgsSettingsGroup *settingsGroupParent,
                          const T &defaultValue,
                          const QString &description = QString() )
      : QgsSettingsEntry( key,
                          settingsGroupParent,
                          defaultValue,
                          description )
    {
      mMetaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( mMetaEnum.isValid() );
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }
    }

    bool setValue( const QVariant &value ) override;

    virtual SettingsType settingsType() const override;

  private:

    QMetaEnum mMetaEnum;

};
#endif



#endif // QGSSETTINGSENTRY_H
