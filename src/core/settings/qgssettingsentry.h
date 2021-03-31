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

/**
 * \ingroup core
 * \class QgsSettingsEntry
 *
 * Represent settings entry and provides methods for reading and writing settings values.
 * Different subclasses are provided for different settings types with metainformations
 * to validate set values and provide more accurate settings description for the gui.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryBase
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsSettingsEntryVariant * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryVariant;
    else if ( dynamic_cast< QgsSettingsEntryString * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryString;
    else if ( dynamic_cast< QgsSettingsEntryStringList * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryStringList;
    else if ( dynamic_cast< QgsSettingsEntryBool * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryBool;
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
     *
     * The \a key argument specifies the key of the settings.
     * The \a section argument specifies the section of the settings.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBase( QString key,
                          QgsSettings::Section section,
                          QVariant defaultValue = QVariant(),
                          QString description = QString() );

    /**
     * Destructor for QgsSettingsEntry.
     */
    virtual ~QgsSettingsEntryBase();

    /**
     * Get settings entry key.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QString key( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns true if a part of the settings key is built dynamically.
     */
    bool hasDynamicKey() const;

    /**
     * Returns true if the settings is contained in the underlying QSettings.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool exists( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Removes the settings from the underlying QSettings.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    void remove( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings section. The settings section of the parent group is returned if available.
     */
    QgsSettings::Section section() const;

    /**
     * Set settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    virtual bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QVariant valueAsVariant( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    QVariant defaultValueAsVariant() const;

    /**
     * Get the settings entry type.
     */
    virtual SettingsType settingsType() const = 0;

    /**
     * Get the settings entry description.
     */
    QString description() const;

  private:

    QString mKey;
    QVariant mDefaultValue;
    QgsSettings::Section mSection;
    QString mDescription;

};


/**
 * \class QgsSettingsEntryVariant
 * \ingroup core
 * A variant settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryVariant : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryVariant.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryVariant( const QString &key,
                             QgsSettings::Section section,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString() );

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QVariant value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    QVariant defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;
};


/**
 * \class QgsSettingsEntryString
 * \ingroup core
 * A string settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryString : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryString.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a minLength argument specifies the minimal length of the string value.
     * The \a maxLength argument specifies the maximal length of the string value.
     * By -1 the there is no limit
     */
    QgsSettingsEntryString( const QString &key,
                            QgsSettings::Section section,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            int minLength = 0,
                            int maxLength = -1 );

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QString value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    QString defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
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
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryStringList : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryStringList.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                QgsSettings::Section section,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString() );

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QStringList value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    QStringList defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;

};


/**
 * \class QgsSettingsEntryBool
 * \ingroup core
 * A boolean settings entry.
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryBool : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryBool.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          QgsSettings::Section section,
                          bool defaultValue = false,
                          const QString &description = QString() );

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    bool defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;

};


/**
 * \class QgsSettingsEntryInteger
 * \ingroup core
 * An integer settings entry.
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryInteger : public QgsSettingsEntryBase
{
  public:


#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryInteger.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
     */
    QgsSettingsEntryInteger( const QString &key,
                             QgsSettings::Section section,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             qlonglong minValue = std::numeric_limits<qlonglong>::min(),
                             qlonglong maxValue = std::numeric_limits<qlonglong>::max() );
#else

    /**
     * Constructor for QgsSettingsEntryInteger.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryInteger( const QString &key,
                             QgsSettings::Section section,
                             qlonglong defaultValue = 0,
                             const QString &description = QString() );

#endif

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    qlonglong value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    qlonglong defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;

    /**
     * Set the minimum value.
     *
     * minValue The minimum value.
     */
    void setMinValue( qlonglong minValue );

    /**
     * Returns the minimum value.
     */
    qlonglong minValue();

    /**
     * Set the maximum value.
     *
     * maxValue The maximum value.
     */
    void setMaxValue( qlonglong maxValue );

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
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryDouble : public QgsSettingsEntryBase
{
  public:

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryDouble.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
     * The \a displayDecimals specifies an hint for the gui about how much decimals to show
     * for example for a QDoubleSpinBox.
     */
    QgsSettingsEntryDouble( const QString &key,
                            QgsSettings::Section section,
                            double defaultValue = 0.0,
                            const QString &description = QString(),
                            double minValue = std::numeric_limits<double>::min(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 );

#else

    /**
     * Constructor for QgsSettingsEntryDouble.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryDouble( const QString &key,
                            QgsSettings::Section section,
                            double defaultValue = 0.0,
                            const QString &description = QString() );

#endif

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    double value( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Get settings default value.
     */
    double defaultValue() const;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;

    /**
     * Set the minimum value.
     *
     * minValue The minimum value.
     */
    void setMinValue( double minValue );

    /**
     * Returns the minimum value.
     */
    double minValue() const;

    /**
     * Set the maximum value.
     *
     * maxValue The maximum value.
     */
    void setMaxValue( double maxValue );

    /**
     * Returns the maximum value.
     */
    double maxValue() const;

    /**
     * Set the display hint decimals.
     *
     * displayHintDecimals The number of decimals that should be shown in the Gui.
     */
    void setDisplayHintDecimals( int displayHintDecimals );

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
  * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryEnum : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryEnum.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    template <class T>
    QgsSettingsEntryEnum( const QString &key,
                          QgsSettings::Section section,
                          const T &defaultValue,
                          const QString &description = QString() )
      : QgsSettingsEntryBase( key,
                              section,
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

    //! \copydoc QgsSettingsEntry::setValue
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const override;

    //! \copydoc QgsSettingsEntry::settingsType
    virtual SettingsType settingsType() const override;

  private:

    QMetaEnum mMetaEnum;

};
#endif



#endif // QGSSETTINGSENTRY_H
