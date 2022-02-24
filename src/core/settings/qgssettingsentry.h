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
#include "qgssettings.h"

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

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryBase.
     *
     * The \a key argument specifies the key of the settings.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryBase( const QString &key,
                          QgsSettings::Section section,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : mKey( key )
      , mDefaultValue( defaultValue )
      , mSection( section )
      , mDescription( description )
      , mPluginName()
      , mOptions( options )
    {
    }

#endif

    /**
     * Constructor for QgsSettingsEntryBase.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryBase( const QString &key,
                          const QString &pluginName,
                          const QVariant &defaultValue = QVariant(),
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : mKey( key )
      , mDefaultValue( defaultValue )
      , mSection( QgsSettings::Plugins )
      , mDescription( description )
      , mPluginName( pluginName )
      , mOptions( options )
    {
    }

    /**
     * Destructor for QgsSettingsEntryBase.
     */
    virtual ~QgsSettingsEntryBase() {}

    /**
     * Returns settings entry key.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    QString key( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Returns settings entry key.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    QString key( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns TRUE if the provided key match the settings entry.
     *
     * This is useful for settings with dynamic keys. For example this permits one to check that
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
     * Returns settings section. The settings section of the parent group is returned if available.
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

    //! Returns settings value with the \a dynamicKeyPart argument specifying the dynamic part of the settings key.
    QVariant valueAsVariant( const QString &dynamicKeyPart = QString() ) const;

    //! Returns settings value with the \a dynamicKeyPart argument specifying the dynamic part of the settings key.
    QVariant valueAsVariant( const QStringList &dynamicKeyPartList ) const;

    /**
     * Returns settings value with a \a defaultValueOverride
     * \since QGIS 3.26
     */
    QVariant valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride ) const;

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     * \since QGIS 3.26
     */
    QVariant valueAsVariantWithDefaultOverride( const QString &dynamicKeyPart, const QVariant &defaultValueOverride ) const;

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPartList argument specifies the list of dynamic parts of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     * \since QGIS 3.26
     */
    QVariant valueAsVariantWithDefaultOverride( const QStringList &dynamicKeyPartList, const QVariant &defaultValueOverride ) const;

    /**
     * Returns settings value with an optional default value override
     * \deprecated since QGIS 3.26 use valueAsVariantWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED QVariant valueAsVariant( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const SIP_DEPRECATED;

    /**
     * Returns settings value with an optional default value override
     * \deprecated since QGIS 3.26 use valueAsVariantWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED QVariant valueAsVariant( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const SIP_DEPRECATED;


    /**
     * Returns settings default value.
     */
    QVariant defaultValueAsVariant() const;

    /**
     * Returns the settings entry type.
     */
    virtual Qgis::SettingsType settingsType() const = 0;

    /**
     * Returns the settings entry description.
     */
    QString description() const;

    /**
     * Returns the former value of the settings if it has been enabled in the options
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

  private:
    QString formerValuekey( const QStringList &dynamicKeyPartList ) const;

    QString mKey;
    QVariant mDefaultValue;
    QgsSettings::Section mSection;
    QString mDescription;
    QString mPluginName;
    Qgis::SettingsOptions mOptions;
};


/**
 * \ingroup core
 * \class QgsSettingsEntryByReference
 *
 * \brief Base abstract class for settings entry which are passed by reference
 * \see QgsSettingsEntryBase
 * \see QgsSettingsEntryByValue
 *
 * \since QGIS 3.26
 */
template<class T>
class CORE_EXPORT QgsSettingsEntryByReference : public QgsSettingsEntryBase
{
  public:

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryByReference.
     *
     * The \a key argument specifies the key of the settings.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryByReference( const QString &key,
                                 QgsSettings::Section section,
                                 const T &defaultValue,
                                 const QString &description = QString(),
                                 Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( key, section, defaultValue, description, options )
    {}

#endif

    /**
     * Constructor for QgsSettingsEntryByReference.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryByReference( const QString &key,
                                 const QString &pluginName,
                                 const T &defaultValue,
                                 const QString &description = QString(),
                                 Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( key, pluginName, QVariant::fromValue<T>( defaultValue ), description, options )
    {}

    virtual Qgis::SettingsType settingsType() const override = 0;

    //! Returns the settings value with a \a defaultValueOverride
    T valueWithDefaultOverride( const T &defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( defaultValueOverride ) );}
    //! Returns the settings value for the \a dynamicKeyPart and with a \a defaultValueOverride
    T valueWithDefaultOverride( const QString &dynamicKeyPart, const T &defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( dynamicKeyPart, defaultValueOverride ) );}
    //! Returns the settings value for the \a dynamicKeyPartList and  with a \a defaultValueOverride
    T valueWithDefaultOverride( const QStringList &dynamicKeyPartList, const T &defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( dynamicKeyPartList, defaultValueOverride ) );}

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QString &dynamicKeyPart = QString() ) const { return this->convertFromVariant( valueAsVariant( dynamicKeyPart ) );}


    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPartList argument specifies the list of dynamic parts of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QStringList &dynamicKeyPartList )  const { return this->convertFromVariant( valueAsVariant( dynamicKeyPartList ) );}

    /**
     * Returns the settings value for the \a dynamicKeyPart and  with a \a defaultValueOverride
     * \deprecated since QGIS 3.26 use valueAsVariantWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED T value( const QString &dynamicKeyPart, bool useDefaultValueOverride, const T &defaultValueOverride ) const SIP_DEPRECATED { Q_NOWARN_DEPRECATED_PUSH return this->convertFromVariant( valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ) ); Q_NOWARN_DEPRECATED_POP}

    /**
     * Returns the settings value for the \a dynamicKeyPartList and  with a \a defaultValueOverride
     * \deprecated since QGIS 3.26 use valueAsVariantWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED T value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const T &defaultValueOverride ) const SIP_DEPRECATED { Q_NOWARN_DEPRECATED_PUSH return this->convertFromVariant( valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ) ); Q_NOWARN_DEPRECATED_POP}


    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const T &value, const QString &dynamicKeyPart = QString() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isNull() )
        dynamicKeyPartList.append( dynamicKeyPart );
      return this->setValuePrivate( value, dynamicKeyPartList );
    }

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const T &value, const QStringList &dynamicKeyPartList ) const
    {
      return this->setValuePrivate( value, dynamicKeyPartList );
    }

    //! Returns settings default value.
    T defaultValue() const {return convertFromVariant( defaultValueAsVariant() );}

    /**
     * Returns the former value
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QString &dynamicKeyPart = QString() ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPart ) );}

    /**
     * Returns the former value
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QStringList &dynamicKeyPartList ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPartList ) );}

  protected:
    virtual bool setValuePrivate( const T &value, const QStringList &dynamicKeyPartList ) const = 0;
    virtual T convertFromVariant( const QVariant &value ) const = 0;
};


/**
 * \ingroup core
 * \class QgsSettingsEntryByValue
 *
 * \brief Base abstract class for settings entry which are passed by value
 * \see QgsSettingsEntryBase
 * \see QgsSettingsEntryByReference
 *
 * \since QGIS 3.26
 */
template<class T>
class CORE_EXPORT QgsSettingsEntryByValue : public QgsSettingsEntryBase
{
  public:

#ifndef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryByValue.
     *
     * The \a key argument specifies the key of the settings.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryByValue( const QString &key, QgsSettings::Section section, QVariant defaultValue, const QString &description = QString(), Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( key, section, defaultValue, description, options )
    {}

#endif

    /**
     * Constructor for QgsSettingsEntryByValue.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryByValue( const QString &key,
                             const QString &pluginName,
                             T defaultValue,
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBase( key, pluginName, defaultValue, description, options )
    {}

    virtual Qgis::SettingsType settingsType() const override = 0;

    //! Returns the settings value with a \a defaultValueOverride
    T valueWithDefaultOverride( T defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( defaultValueOverride ) );}
    //! Returns the settings value for the \a dynamicKeyPart and  with a \a defaultValueOverride
    T valueWithDefaultOverride( const QString &dynamicKeyPart, T defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( dynamicKeyPart, defaultValueOverride ) );}
    //! Returns the settings value for the \a dynamicKeyPartList and  with a \a defaultValueOverride
    T valueWithDefaultOverride( const QStringList &dynamicKeyPartList, T defaultValueOverride ) const { return this->convertFromVariant( valueAsVariantWithDefaultOverride( dynamicKeyPartList, defaultValueOverride ) );}

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QString &dynamicKeyPart = QString() ) const { return this->convertFromVariant( valueAsVariant( dynamicKeyPart ) );}


    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPartList argument specifies the list of dynamic parts of the settings key.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QStringList &dynamicKeyPartList )  const { return this->convertFromVariant( valueAsVariant( dynamicKeyPartList ) );}


    /**
     * Returns the settings value for the \a dynamicKeyPart and  with a \a defaultValueOverride
     * \deprecated since QGIS 3.26 use valueWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED T value( const QString &dynamicKeyPart, bool useDefaultValueOverride, T defaultValueOverride ) const SIP_DEPRECATED { Q_NOWARN_DEPRECATED_PUSH return this->convertFromVariant( valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ) ); Q_NOWARN_DEPRECATED_POP}

    /**
     * Returns the settings value for the \a dynamicKeyPartList and  with a \a defaultValueOverride
     * \deprecated since QGIS 3.26 use valueWithDefaultOverride instead
     */
    Q_DECL_DEPRECATED T value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, T defaultValueOverride ) const  SIP_DEPRECATED { Q_NOWARN_DEPRECATED_PUSH return this->convertFromVariant( valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ) ); Q_NOWARN_DEPRECATED_POP}

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    virtual bool setValue( T value, const QString &dynamicKeyPart = QString() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isNull() )
        dynamicKeyPartList.append( dynamicKeyPart );
      return this->setValuePrivate( value, dynamicKeyPartList );
    }

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    virtual bool setValue( T value, const QStringList &dynamicKeyPartList ) const
    {
      return this->setValuePrivate( value, dynamicKeyPartList );
    }

    //! Returns settings default value.
    T defaultValue() const {return convertFromVariant( defaultValueAsVariant() );}

    /**
     * Returns the former value
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QString &dynamicKeyPart = QString() ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPart ) );}

    /**
     * Returns the former value
     * Returns the current value (or default) if there is no former value.
     */
    T formerValue( const QStringList &dynamicKeyPartList ) const {return convertFromVariant( formerValueAsVariant( dynamicKeyPartList ) );}

  protected:
    virtual bool setValuePrivate( T value, const QStringList &dynamicKeyPartList ) const = 0;
    virtual T convertFromVariant( const QVariant &value ) const = 0;
};


#endif // QGSSETTINGSENTRY_H
