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

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"


class QgsSettingsTreeNode;


static const inline QMetaEnum sSettingsTypeMetaEnum = QMetaEnum::fromType<Qgis::SettingsType>() SIP_SKIP;


/**
 * \ingroup core
 * \class QgsSettingsEntryBase
 *
 * \brief Represent settings entry and provides methods for reading and writing settings values.
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
    else if ( dynamic_cast< QgsSettingsEntryVariantMap * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryVariantMap;
    else if ( dynamic_cast< QgsSettingsEntryBool * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryBool;
    else if ( dynamic_cast< QgsSettingsEntryInteger * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryInteger;
    else if ( dynamic_cast< QgsSettingsEntryDouble * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryDouble;
    else if ( dynamic_cast< QgsSettingsEntryColor * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryColor;
    else if ( dynamic_cast< QgsSettingsEntryBase * >( sipCpp ) )
      sipType = sipType_QgsSettingsEntryBase;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
     * Transforms a dynamic key part string to list
     * \since QGIS 3.26
     */
    static QStringList dynamicKeyPartToList( const QString &dynamicKeyPart );

    /**
     * Constructor for QgsSettingsEntryBase.
     *
     * \param key specifies the key of the settings.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryBase( const QString &key,
                          const QString &section,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : mName( key )
      , mKey( QStringLiteral( "%1/%2" ).arg( section, key ) )
      , mDefaultValue( defaultValue )
      , mDescription( description )
      , mOptions( options )
    {}

    /**
     * Constructor for QgsSettingsEntryBase.
     *
     * \param name specifies the name of the setting.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     *
     * \since QGIS 3.30
     */
    QgsSettingsEntryBase( const QString &name,
                          QgsSettingsTreeNode *parent,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException );

    virtual ~QgsSettingsEntryBase();

    /**
     * Returns the id of the type of settings
     * This can be re-implemented in a custom implementation of a setting
     * \since QGIS 3.32
     */
    virtual QString typeId() const;

    /**
     * Returns the name of the settings
     * \since QGIS 3.30
     */
    QString name() const {return mName;}

    /**
     * Returns settings entry key.
     *
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    QString key( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns settings entry key.
     *
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    QString key( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns TRUE if the provided key match the settings entry.
     *
     * This is useful for settings with dynamic keys. For example this permits one to check that
     * the settings key "NewsFeed/httpsfeedqgisorg/27/content" is valid for the settings entry
     * defined with the key "NewsFeed/%1/%2/content"
     *
     * \param key to check
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
     * Returns the settings options
     * \since QGIS 3.26
     */
    Qgis::SettingsOptions options() const {return mOptions;}

    /**
     * Returns TRUE if the settings is contained in the underlying QSettings.
     *
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    bool exists( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns TRUE if the settings is contained in the underlying QSettings.
     *
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    bool exists( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns the origin of the setting if it exists
     * \note it will return Qgis::SettingsOrigin::Any if the key doesn't exist
     * \since QGIS 3.30
     */
    Qgis::SettingsOrigin origin( const QStringList &dynamicKeyPartList ) const;

    /**
     * Removes the settings from the underlying QSettings.
     *
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    void remove( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Removes the settings from the underlying QSettings.
     *
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    void remove( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns settings section. The settings section of the parent group is returned if available.
     * \deprecated QGIS 3.26. The key is entirely self-defined.
     */
    Q_DECL_DEPRECATED int section() const;

    /**
     * Set settings value.
     *
     * \param value specifies the value to set.
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    bool setVariantValue( const QVariant &value, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Set settings value.
     * This should be called from any implementation as it takes care of actually calling QSettings
    * \param value specifies the value to set.
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    bool setVariantValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const;

    //! Returns settings value with \param dynamicKeyPart specifying the dynamic part of the settings key.
    QVariant valueAsVariant( const QString &dynamicKeyPart = QString() ) const;

    //! Returns settings value with \param dynamicKeyPartList specifying the dynamic part of the settings key.
    QVariant valueAsVariant( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns settings value with a \a defaultValueOverride
     * \since QGIS 3.26
     */
    QVariant valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns settings value.
     *
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     * \param defaultValueOverride if valid is used instead of the normal default value.
     * \since QGIS 3.26
     */
    QVariant valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns settings value with an optional default value override
     * \deprecated QGIS 3.26. Use valueAsVariantWithDefaultOverride() instead.
     */
    Q_DECL_DEPRECATED QVariant valueAsVariant( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const SIP_DEPRECATED;

    /**
     * Returns settings value with an optional default value override
     * \deprecated QGIS 3.26. Use valueAsVariantWithDefaultOverride() instead.
     */
    Q_DECL_DEPRECATED QVariant valueAsVariant( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const SIP_DEPRECATED;


    /**
     * Returns settings default value.
     */
    QVariant defaultValueAsVariant() const;

    /**
     * Returns the settings entry type.
     */
    virtual Qgis::SettingsType settingsType() const {return Qgis::SettingsType::Custom;}
    // This cannot be pure virtual otherwise SIP is failing

    /**
     * Returns the settings entry description.
     */
    QString description() const;

    /**
     * Returns the former value of the settings if it has been enabled in the options.
     * Returns the current value (or default) if there is no former value.
     * \since QGIS 3.26
     */
    QVariant formerValueAsVariant( const QString &dynamicKeyPart ) const;

    /**
     * Returns the former value of the settings if it has been enabled in the options
     * Returns the current value (or default) if there is no former value.
     * \since QGIS 3.26
     */
    QVariant formerValueAsVariant( const QStringList &dynamicKeyPartList ) const;

    /**
     * Copies the value from a given key if it exists.
     * \param key the key to copy the setting value from.
     * \param removeSettingAtKey if TRUE, the setting at the old key will be removed.
     * \returns TRUE if the key exists and the setting value could be copied.
     * \since QGIS 3.30
     */
    bool copyValueFromKey( const QString &key, bool removeSettingAtKey = false ) const {return copyValueFromKey( key, {}, removeSettingAtKey );}

    /**
     * Copies the value from a given key if it exists.
     * \param key the key to copy the setting value from.
     * \param dynamicKeyPartList is the optional dynamic key part to determine the key. It must be the same for origin and destination keys.
     * \param removeSettingAtKey if TRUE, the setting at the old key will be removed.
     * \returns TRUE if the key exists and the setting value could be copied.
     * \since QGIS 3.30
     */
    bool copyValueFromKey( const QString &key, const QStringList &dynamicKeyPartList, bool removeSettingAtKey = false ) const;

    /**
     * Copies the settings to the given key.
     * \param key the key to copy the setting value to.
     * \param dynamicKeyPartList is the optional dynamic key part to determine the key. It must be the same for origin and destination keys.
     * \since QGIS 3.30
     */
    void copyValueToKey( const QString &key, const QStringList &dynamicKeyPartList = QStringList() ) const;

    /**
     * Copies the settings to the given key, if it has changed during the current QGIS session (see hasChanged()).
     *
     * \param key the key to copy the setting value to.
     * \param dynamicKeyPartList is the optional dynamic key part to determine the key. It must be the same for origin and destination keys.
     *
     * \since QGIS 3.36
     */
    void copyValueToKeyIfChanged( const QString &key, const QStringList &dynamicKeyPartList = QStringList() ) const;

    /**
    * Returns the parent tree element
    * \since QGIS 3.30
    */
    QgsSettingsTreeNode *parent() const {return mParentTreeElement;}

    //! Returns TRUE if the given \a value is valid towards the setting definition
    virtual bool checkValueVariant( const QVariant &value ) const
    {
      Q_UNUSED( value )
      return true;
    }

    /**
     * Returns TRUE if the setting was changed during the current QGIS session.
     *
     * \since QGIS 3.36
     */
    bool hasChanged() const { return mHasChanged; }

  private:
    QString formerValuekey( const QStringList &dynamicKeyPartList ) const;

    QString completeKeyPrivate( const QString &key, const QStringList &dynamicKeyPartList ) const;

    QgsSettingsTreeNode *mParentTreeElement = nullptr;
    QString mName;
    QString mKey;
    QVariant mDefaultValue;
    QString mDescription;
    Qgis::SettingsOptions mOptions;
    mutable bool mHasChanged = false;
};

/**
 * \ingroup core
 * \class QgsSettingsEntryBaseTemplate
 *
 * \brief Base abstract class for settings entries with typed get and set methods
 * \see QgsSettingsEntryBase
 *
 * \since QGIS 3.32
 */
template<class T>
class QgsSettingsEntryBaseTemplate : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryByReference.
     *
     * \param name specifies the key of the settings.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     *
     * \since QGIS 3.30
     */
    QgsSettingsEntryBaseTemplate( const QString &name,
                                  QgsSettingsTreeNode *parent,
                                  const QVariant &defaultValue,
                                  const QString &description = QString(),
                                  Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( name, parent, defaultValue, description, options )
    {}

    /**
     * Constructor for QgsSettingsEntryByReference.
     *
     * \param key specifies the key of the settings.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryBaseTemplate( const QString &key,
                                  const QString &section,
                                  const QVariant &defaultValue,
                                  const QString &description = QString(),
                                  Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( key, section, defaultValue, description, options )
    {}


    virtual Qgis::SettingsType settingsType() const override = 0;

    /**
     * Returns settings value.
     *
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    T value( const QString &dynamicKeyPart = QString() ) const { return this->convertFromVariant( valueAsVariant( dynamicKeyPart ) );}

    /**
     * Returns settings value.
     *
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    T value( const QStringList &dynamicKeyPartList )  const { return this->convertFromVariant( valueAsVariant( dynamicKeyPartList ) );}


    //! Returns the settings value with a \a defaultValueOverride and with an optional \a dynamicKeyPart
    inline T valueWithDefaultOverride( const T &defaultValueOverride, const QString &dynamicKeyPart = QString() ) const
    {
      return this->convertFromVariant( valueAsVariantWithDefaultOverride( convertToVariant( defaultValueOverride ), dynamicKeyPart ) );
    }

    //! Returns the settings value with a \a defaultValueOverride for the \a dynamicKeyPartList
    inline T valueWithDefaultOverride( const T &defaultValueOverride, const QStringList &dynamicKeyPartList ) const
    {
      return this->convertFromVariant( valueAsVariantWithDefaultOverride( convertToVariant( defaultValueOverride ), dynamicKeyPartList ) );
    }

    /**
     * Set settings value.
     *
     * \param value specifies the value to set.
     * \param dynamicKeyPart specifies the dynamic part of the settings key.
     */
    bool setValue( const T &value, const QString &dynamicKeyPart = QString() ) const
    {
      return setValuePrivate( value, dynamicKeyPartToList( dynamicKeyPart ) );
    }

    /**
     * Set settings value.
     *
     * \param value specifies the value to set.
     * \param dynamicKeyPartList specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const T &value, const QStringList &dynamicKeyPartList ) const
    {
      return setValuePrivate( value, dynamicKeyPartList );
    }

    //! Returns settings default value.
    T defaultValue() const {return convertFromVariant( defaultValueAsVariant() );}

    /**
     * Returns the former value.
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QString &dynamicKeyPart = QString() ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPart ) );}

    /**
     * Returns the former value
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QStringList &dynamicKeyPartList ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPartList ) );}

    bool checkValueVariant( const QVariant &value ) const override
    {
      return checkValuePrivate( convertFromVariant( value ) );
    }

    //! Converts the variant value to the value type of the setting
    virtual T convertFromVariant( const QVariant &value ) const = 0;

  protected:
    //! Sets the settings value with an optional list of dynamic parts
    virtual bool setValuePrivate( const T &value, const QStringList &dynamicKeyPartList ) const
    {
      if ( checkValuePrivate( value ) )
        return QgsSettingsEntryBase::setVariantValue( convertToVariant( value ), dynamicKeyPartList );
      else
        return false;
    }

    //! Converts the value to a variant
    virtual QVariant convertToVariant( const T &value ) const
    {
      return QVariant::fromValue( value );
    }

    //! Check if the value is valid
    virtual bool checkValuePrivate( const T &value ) const
    {
      Q_UNUSED( value )
      return true;
    }
};




#endif // QGSSETTINGSENTRY_H
