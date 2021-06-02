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
#include <QColor>
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
    else if ( dynamic_cast< QgsSettingsEntryColor * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryColor;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    //! Types of settings entries
    enum class SettingsType : int
    {
      Variant, //!< Generic variant
      String, //!< String
      StringList, //!< List of strings
      Bool, //!< Boolean
      Integer, //!< Integer
      Double, //!< Double precision numer
      EnumFlag, //!< Enum or Flag
      Color //!< Color
    };

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntry.
     *
     * The \a key argument specifies the key of the settings.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBase( const QString &key,
                          QgsSettings::Section section,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString() );

#endif

    /**
     * Constructor for QgsSettingsEntry.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBase( const QString &key,
                          const QString &pluginName,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString() );

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
     * Get settings entry key.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    QString key( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns TRUE if the provided key match the settings entry.
     *
     * This is useful for settings with dynamic keys. For example this permits to check that
     * the settings key "NewsFeed/httpsfeedqgisorg/27/content" is valid for the settings entry
     * defined with the key "NewsFeed/%1/%2/content"
     *
     * The \a key to check
     */
    bool keyIsValid( const QString &key ) const;

    /**
     * Returns settings entry defining key.
     * For dynamic settings it return the key with the placeholder for dynamic part
     * included. For non-dynamic settings returns the same as key().
     */
    QString definitionKey() const;

    /**
     * Returns TRUE if a part of the settings key is built dynamically.
     */
    bool hasDynamicKey() const;

    /**
     * Returns TRUE if the settings is contained in the underlying QSettings.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool exists( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns TRUE if the settings is contained in the underlying QSettings.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool exists( const QStringList &dynamicKeyPartList ) const;

    /**
     * Removes the settings from the underlying QSettings.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    void remove( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Removes the settings from the underlying QSettings.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    void remove( const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings section. The settings section of the parent group is returned if available.
     */
    QgsSettings::Section section() const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    virtual bool setVariantValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    virtual bool setVariantValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QVariant valueAsVariant( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const QVariant &defaultValueOverride = QVariant() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QVariant valueAsVariant( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const QVariant &defaultValueOverride = QVariant() ) const;

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
    QString mPluginName;
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

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryVariant.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryVariant( const QString &key,
                             QgsSettings::Section section,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString() );

#endif

    /**
     * Constructor for QgsSettingsEntryVariant.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryVariant( const QString &key,
                             const QString &pluginName,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QVariant value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const QVariant &defaultValueOverride = QVariant() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QVariant value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const QVariant &defaultValueOverride = QVariant() ) const;

    /**
     * Get settings default value.
     */
    QVariant defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
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

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryString.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
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

#endif

    /**
     * Constructor for QgsSettingsEntryString.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryString( const QString &key,
                            const QString &pluginName,
                            const QString &defaultValue = QString(),
                            const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const QString &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const QString &value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QString value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const QString &defaultValueOverride = QString() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QString value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const QString &defaultValueOverride = QString() ) const;

    /**
     * Get settings default value.
     */
    QString defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
    virtual SettingsType settingsType() const override;

    /**
     * Set the string minimum length.
     *
     * minLength The string minimum length.
     */
    void setMinLength( int minLength );

    /**
     * Returns the string minimum length.
     */
    int minLength();

    /**
     * Set the string maximum length.
     *
     * maxLength The string maximum length.
     */
    void setMaxLength( int maxLength );

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

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryStringList.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                QgsSettings::Section section,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString() );

#endif

    /**
     * Constructor for QgsSettingsEntryStringList.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                const QString &pluginName,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const QStringList &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const QStringList &value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QStringList value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const QStringList &defaultValueOverride = QStringList() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QStringList value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const QStringList &defaultValueOverride = QStringList() ) const;

    /**
     * Get settings default value.
     */
    QStringList defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
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

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryBool.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          QgsSettings::Section section,
                          bool defaultValue = false,
                          const QString &description = QString() );

#endif

    /**
     * Constructor for QgsSettingsEntryBool.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          const QString &pluginName,
                          bool defaultValue = false,
                          const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( bool value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( bool value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    bool value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, bool defaultValueOverride = false ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    bool value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, bool defaultValueOverride = false ) const;

    /**
     * Get settings default value.
     */
    bool defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
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
     * The \a section argument specifies the section.
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

#endif

    /**
     * Constructor for QgsSettingsEntryInteger.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryInteger( const QString &key,
                             const QString &pluginName,
                             qlonglong defaultValue = 0,
                             const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( qlonglong value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( qlonglong value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    qlonglong value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, qlonglong defaultValueOverride = 0 ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    qlonglong value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, qlonglong defaultValueOverride = 0 ) const;

    /**
     * Get settings default value.
     */
    qlonglong defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
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
     * The \a section argument specifies the section.
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
                            double minValue = std::numeric_limits<double>::lowest(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 );

#endif

    /**
     * Constructor for QgsSettingsEntryDouble.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryDouble( const QString &key,
                            const QString &pluginName,
                            double defaultValue,
                            const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( double value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( double value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    double value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, double defaultValueOverride = 0.0 ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    double value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, double defaultValueOverride = 0.0 ) const;

    /**
     * Get settings default value.
     */
    double defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
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


/**
 * \class QgsSettingsEntryEnumFlag
 * \ingroup core
 * A template class for enum and flag settings entry.
 *
 * \note This template class has a dedicated handling in sipify.pl
 * \since QGIS 3.20
 */
template <typename T>
class CORE_EXPORT QgsSettingsEntryEnumFlag : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryEnumFlagBase.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     *
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly
     */
    QgsSettingsEntryEnumFlag( const QString &key, QgsSettings::Section section, const T &defaultValue, const QString &description = QString() )
      : QgsSettingsEntryBase( key, section, QMetaEnum::fromType<T>().isFlag() ? QVariant( QMetaEnum::fromType<T>().valueToKeys( defaultValue ) ) : QVariant( QMetaEnum::fromType<T>().valueToKey( defaultValue ) ), description )
    {
      mMetaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( mMetaEnum.isValid() );
      if ( !mMetaEnum.isValid() )
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( QgsSettingsEntryBase::key() ) );
    }

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const T &defaultValueOverride = T() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isEmpty() )
        dynamicKeyPartList.append( dynamicKeyPart );

      return value( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride );
    }

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const T &defaultValueOverride = T() ) const
    {
      T defaultVal = defaultValue();
      if ( useDefaultValueOverride )
        defaultVal = defaultValueOverride;

      if ( !mMetaEnum.isFlag() )
        return QgsSettings().enumValue( key( dynamicKeyPartList ),
                                        defaultVal,
                                        section() );
      else
        return QgsSettings().flagValue( key( dynamicKeyPartList ),
                                        defaultVal,
                                        section() );
    }

    /**
     * Get settings default value.
     */
    T defaultValue() const
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( key() ) );
        return T();
      }

      bool ok = false;
      T defaultValue;
      if ( !mMetaEnum.isFlag() )
        defaultValue = static_cast<T>( mMetaEnum.keyToValue( defaultValueAsVariant().toByteArray(), &ok ) );
      else
        defaultValue = static_cast<T>( mMetaEnum.keysToValue( defaultValueAsVariant().toByteArray(), &ok ) );

      if ( !ok )
      {
        QgsDebugMsg( QStringLiteral( "Invalid enum/flag key/s '%1' for settings key '%2'" ).arg( defaultValueAsVariant().toString(), key() ) );
        return T();
      }

      return defaultValue;
    }

    //! \copydoc QgsSettingsEntryBase::setValue
    bool setValue( const T &value, const QString &dynamicKeyPart = QString() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isEmpty() )
        dynamicKeyPartList.append( dynamicKeyPart );

      return setValue( value, dynamicKeyPartList );
    }

    //! \copydoc QgsSettingsEntryBase::setValue
    bool setValue( const T &value, const QStringList &dynamicKeyPartList ) const
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( key( dynamicKeyPartList ) ) );
        return false;
      }

      if ( !mMetaEnum.isFlag() )
      {
        const char *enumKey = mMetaEnum.valueToKey( value );
        if ( enumKey == nullptr )
        {
          QgsDebugMsg( QStringLiteral( "Invalid enum value '%1'." ).arg( value ) );
          return false;
        }

        return QgsSettingsEntryBase::setVariantValue( enumKey, dynamicKeyPartList );
      }
      else
      {
        QByteArray flagKeys = mMetaEnum.valueToKeys( value );
        if ( flagKeys.isEmpty() )
        {
          QgsDebugMsg( QStringLiteral( "Invalid flag value '%1'." ).arg( value ) );
          return false;
        }
        return QgsSettingsEntryBase::setVariantValue( flagKeys, dynamicKeyPartList );
      }
    }

    //! \copydoc QgsSettingsEntryBase::settingsType
    virtual QgsSettingsEntryBase::SettingsType settingsType() const override
    {
      return QgsSettingsEntryBase::SettingsType::EnumFlag;
    }

  private:

    QMetaEnum mMetaEnum;

};


/**
 * \class QgsSettingsEntryColor
 * \ingroup core
 * A color settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryColor : public QgsSettingsEntryBase
{
  public:

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryColor.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryColor( const QString &key,
                           QgsSettings::Section section,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString() );

#endif

    /**
     * Constructor for QgsSettingsEntryColor.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a default value argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     */
    QgsSettingsEntryColor( const QString &key,
                           const QString &pluginName,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString() );

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const QColor &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const QColor &value, const QStringList &dynamicKeyPartList ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QColor value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const QString &defaultValueOverride = QString() ) const;

    /**
     * Get settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    QColor value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const QString &defaultValueOverride = QString() ) const;

    /**
     * Get settings default value.
     */
    QColor defaultValue() const;

    //! \copydoc QgsSettingsEntryBase::settingsType
    virtual SettingsType settingsType() const override;

};

#endif // QGSSETTINGSENTRY_H
