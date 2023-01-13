/***************************************************************************
  qgssettingsentryimpl.h
  --------------------------------------
  Date                 : February 2022
  Copyright            : (C) 2022 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSENTRYIMPL_H
#define QGSSETTINGSENTRYIMPL_H

#include "qgssettingsentry.h"


/**
 * \class QgsSettingsEntryVariant
 * \ingroup core
 *
 * \brief A variant settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryVariant : public QgsSettingsEntryByReference<QVariant>
{
  public:


    /**
     * Constructor for QgsSettingsEntryVariant.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \since QGIS 3.30
     */
    QgsSettingsEntryVariant( const QString &key,
                             QgsSettingsTreeElement *parent,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryByReference( key, parent, defaultValue, description, options ) SIP_THROW( QgsSettingsException )
    {}

    /**
     * Constructor for QgsSettingsEntryVariant.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \since QGIS 3.30
     */
    QgsSettingsEntryVariant( const QString &key,
                             const QString &section,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryVariant.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryVariant( const QString &key,
                             const QString &pluginName,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryVariant( QgsSettingsEntryVariant( *a0, QgsSettings::createPluginTreeElement( *a1 ), *a2, *a3, *a4 ) );
    % End
#endif

#ifdef SIP_RUN

    /**
     * Returns settings value.
     *
     * \param type is the Python type of the value to be returned
     */

    SIP_PYOBJECT valueAs( SIP_PYOBJECT type ) const;
    % MethodCode
    typedef PyObject *( *pyqt5_from_qvariant_by_type )( QVariant &value, PyObject *type );
    QVariant value;

    // QSettings has an internal mutex so release the GIL to avoid the possibility of deadlocks.
    value = sipCpp->value();

    pyqt5_from_qvariant_by_type f = ( pyqt5_from_qvariant_by_type ) sipImportSymbol( "pyqt5_from_qvariant_by_type" );
    sipRes = f( value, a0 );

    sipIsErr = !sipRes;
    % End
#endif


    virtual Qgis::SettingsType settingsType() const override;

  private:
    QVariant convertFromVariant( const QVariant &value ) const override SIP_FORCE {return value;}
};

/**
 * \class QgsSettingsEntryString
 * \ingroup core
 *
 * \brief A string settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryString : public QgsSettingsEntryByReference<QString>
{
  public:

    /**
     * Constructor for QgsSettingsEntryString.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minLength specifies the minimal length of the string value. 0 means no limit.
     * \param maxLength specifies the maximal length of the string value. -1 means no limit.
     */
    QgsSettingsEntryString( const QString &key,
                            QgsSettingsTreeElement *parent,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            int minLength = 0,
                            int maxLength = -1 ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByReference<QString>( key, parent, defaultValue, description, options )
      , mMinLength( minLength )
      , mMaxLength( maxLength )
    {}

    /**
     * Constructor for QgsSettingsEntryString.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minLength specifies the minimal length of the string value. 0 means no limit.
     * \param maxLength specifies the maximal length of the string value. -1 means no limit.
     */
    QgsSettingsEntryString( const QString &key,
                            const QString &section,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            int minLength = 0,
                            int maxLength = -1 ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference<QString>( key, section, defaultValue, description, options ) SIP_MAKE_PRIVATE
    , mMinLength( minLength )
    , mMaxLength( maxLength )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryString.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryString( const QString &key,
                            const QString &pluginName,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            int minLength = 0,
                            int maxLength = -1 ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryString( QgsSettingsEntryString( *a0, QgsSettings::createPluginTreeElement( *a1 ), *a2, *a3, *a4 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

    /**
     * Set the string minimum length.
     *
     * minLength The string minimum length.
     */
    void setMinLength( int minLength );

    /**
     * Returns the string minimum length.
     */
    int minLength() const;

    /**
     * Set the string maximum length.
     *
     * maxLength The string maximum length.
     */
    void setMaxLength( int maxLength );

    /**
     * Returns the string maximum length. By -1 there is no limitation.
     */
    int maxLength() const;

  private:
    bool checkValue( const QString &value ) const override SIP_FORCE;
    QString convertFromVariant( const QVariant &value ) const override SIP_FORCE;

    int mMinLength;
    int mMaxLength;

};


/**
 * \class QgsSettingsEntryStringList
 * \ingroup core
 *
 * \brief A string list settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryStringList : public QgsSettingsEntryByReference<QStringList>
{
  public:

    /**
     * Constructor for QgsSettingsEntryStringList.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                QgsSettingsTreeElement *parent,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryByReference( key, parent, defaultValue, description, options ) SIP_THROW( QgsSettingsException )
    {}

    /**
     * Constructor for QgsSettingsEntryStringList.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                const QString &section,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    {}


#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryStringList.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                const QString &pluginName,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryStringList( QgsSettingsEntryStringList( *a0, QgsSettings::createPluginTreeElement( *a1 ), *a2, *a3, *a4 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

  private:
    QStringList convertFromVariant( const QVariant &value ) const override SIP_FORCE;

};


/**
 * \class QgsSettingsEntryBool
 * \ingroup core
 *
 * \brief A boolean settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryBool : public QgsSettingsEntryByValue<bool>
{
  public:

    /**
     * Constructor for QgsSettingsEntryBool.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          QgsSettingsTreeElement *parent,
                          bool defaultValue = false,
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByValue( key, parent, defaultValue, description, options )
    {}

    /**
     * Constructor for QgsSettingsEntryBool.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          const QString &section,
                          bool defaultValue = false,
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByValue( key, section, defaultValue, description, options )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryBool.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          const QString &pluginName,
                          bool defaultValue = false,
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryBool( QgsSettingsEntryBool( *a0, QgsSettings::createPluginTreeElement( *a1 ), a2, *a3, *a4 ) );
    % End
#endif


    virtual Qgis::SettingsType settingsType() const override;

  private:
    bool convertFromVariant( const QVariant &value ) const override SIP_FORCE;
};


/**
 * \class QgsSettingsEntryInteger
 * \ingroup core
 *
 * \brief An integer settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryInteger : public QgsSettingsEntryByValue<qlonglong>
{
  public:

    /**
     * Constructor for QgsSettingsEntryInteger.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     */
    QgsSettingsEntryInteger( const QString &key,
                             QgsSettingsTreeElement *parent,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                             qlonglong minValue = std::numeric_limits<qlonglong>::min(),
                             qlonglong maxValue = std::numeric_limits<qlonglong>::max() ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByValue( key, parent, defaultValue, description, options )
      , mMinValue( minValue )
      , mMaxValue( maxValue )
    { }

    /**
     * Constructor for QgsSettingsEntryInteger.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     */
    QgsSettingsEntryInteger( const QString &key,
                             const QString &section,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                             qlonglong minValue = std::numeric_limits<qlonglong>::min(),
                             qlonglong maxValue = std::numeric_limits<qlonglong>::max() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByValue( key, section, defaultValue, description, options )
    , mMinValue( minValue )
    , mMaxValue( maxValue )
    { }

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryInteger.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is used to define the key of the setting
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     */
    QgsSettingsEntryInteger( const QString &key,
                             const QString &pluginName,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                             qlonglong minValue = std::numeric_limits<qlonglong>::min(),
                             qlonglong maxValue = std::numeric_limits<qlonglong>::max() ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryInteger( QgsSettingsEntryInteger( *a0, QgsSettings::createPluginTreeElement( *a1 ), a2, *a3, *a4, a5, a6 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

    /**
     * Set the minimum value.
     *
     * minValue The minimum value.
     */
    void setMinValue( qlonglong minValue );

    /**
     * Returns the minimum value.
     */
    qlonglong minValue() const;

    /**
     * Set the maximum value.
     *
     * maxValue The maximum value.
     */
    void setMaxValue( qlonglong maxValue );

    /**
     * Returns the maximum value.
     */
    qlonglong maxValue() const;

  private:
    bool checkValue( qlonglong value ) const override SIP_FORCE;
    qlonglong convertFromVariant( const QVariant &value ) const override SIP_FORCE;
    qlonglong mMinValue;
    qlonglong mMaxValue;

};


/**
 * \class QgsSettingsEntryDouble
 * \ingroup core
 *
 * \brief A double settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryDouble : public QgsSettingsEntryByValue<double>
{
  public:

    /**
     * Constructor for QgsSettingsEntryDouble.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     * \param displayDecimals specifies an hint for the gui about how much decimals to show
     * for example for a QDoubleSpinBox.
     */
    QgsSettingsEntryDouble( const QString &key,
                            QgsSettingsTreeElement *parent,
                            double defaultValue = 0.0,
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            double minValue = std::numeric_limits<double>::lowest(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByValue( key, parent, defaultValue, description, options )
      , mMinValue( minValue )
      , mMaxValue( maxValue )
      , mDisplayHintDecimals( displayDecimals )
    {}

    /**
     * Constructor for QgsSettingsEntryDouble.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     * \param displayDecimals specifies an hint for the gui about how much decimals to show
     * for example for a QDoubleSpinBox.
     */
    QgsSettingsEntryDouble( const QString &key,
                            const QString &section,
                            double defaultValue = 0.0,
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            double minValue = std::numeric_limits<double>::lowest(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByValue( key, section, defaultValue, description, options )
    , mMinValue( minValue )
    , mMaxValue( maxValue )
    , mDisplayHintDecimals( displayDecimals )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryDouble.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is used to define the key of the setting
     * \param defaultValue specifies the default value for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param minValue specifies the minimal value.
     * \param maxValue specifies the maximal value.
     * \param displayDecimals specifies an hint for the gui about how much decimals to show
     */
    QgsSettingsEntryDouble( const QString &key,
                            const QString &pluginName,
                            double defaultValue,
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            double minValue = std::numeric_limits<double>::lowest(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryDouble( QgsSettingsEntryDouble( *a0, QgsSettings::createPluginTreeElement( *a1 ), a2, *a3, *a4, a5, a6, a7 ) );
    % End
#endif


    virtual Qgis::SettingsType settingsType() const override;

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
    bool checkValue( double value ) const override SIP_FORCE;
    double convertFromVariant( const QVariant &value ) const override SIP_FORCE;
    double mMinValue;
    double mMaxValue;

    int mDisplayHintDecimals;

};



/**
 * \class QgsSettingsEntryColor
 * \ingroup core
 *
 * \brief A color settings entry.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsEntryColor : public QgsSettingsEntryByReference<QColor>
{
  public:

    /**
     * Constructor for QgsSettingsEntryColor.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param allowAlpha specifies if the color can have transparency.
     */
    QgsSettingsEntryColor( const QString &key,
                           QgsSettingsTreeElement *parent,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString(),
                           Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                           bool allowAlpha = true ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByReference( key, parent, defaultValue, description, options )
      , mAllowAlpha( allowAlpha )
    {}

    /**
     * Constructor for QgsSettingsEntryColor.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     * \param allowAlpha specifies if the color can have transparency.
     */
    QgsSettingsEntryColor( const QString &key,
                           const QString &section,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString(),
                           Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                           bool allowAlpha = true ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    , mAllowAlpha( allowAlpha )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryColor.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryColor( const QString &key,
                           const QString &pluginName,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString(),
                           Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                           bool allowAlpha = true ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryColor( QgsSettingsEntryColor( *a0, QgsSettings::createPluginTreeElement( *a1 ), *a2, *a3, *a4, a5 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

    /**
     * Returns TRUE if transparency is allowed for the color
     * \since QGIS 3.30
     */
    bool allowAlpha() const {return mAllowAlpha;}

    /**
     * Copies the value from the given keys if they exist.
     * \returns TRUE if the keys exist and the settings values could be copied
     * \since QGIS 3.30
     */
    bool copyValueFromKeys( const QString &redKey, const QString &greenKey, const QString &blueKey, const QString &alphaKey = QString(), bool removeSettingAtKey = false ) const SIP_SKIP;

    /**
     * Copies the settings to the given keys
     * \since QGIS 3.30
     */
    void copyValueToKeys( const QString &redKey, const QString &greenKey, const QString &blueKey, const QString &alphaKey = QString() ) const SIP_SKIP;


  private:
    QColor convertFromVariant( const QVariant &value ) const override SIP_FORCE;
    bool checkValue( const QColor &value ) const override SIP_FORCE;
    bool mAllowAlpha = true;
};

/**
 * \class QgsSettingsEntryVariantMap
 * \ingroup core
 *
 * \brief A string list settings entry.
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsEntryVariantMap : public QgsSettingsEntryByReference<QVariantMap>
{
  public:


    /**
     * Constructor for QgsSettingsEntryVariantMap.
     *
     * \param key specifies the final part of the settings key.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryVariantMap( const QString &key,
                                QgsSettingsTreeElement *parent,
                                const QVariantMap &defaultValue = QVariantMap(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException )
      : QgsSettingsEntryByReference( key, parent, defaultValue, description, options )
    {
    }

    /**
     * Constructor for QgsSettingsEntryVariantMap.
     *
     * \param key specifies the final part of the settings key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryVariantMap( const QString &key,
                                const QString &section,
                                const QVariantMap &defaultValue = QVariantMap(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    {
    }

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryStringMap.
     * This constructor is intended to be used from plugins.
     *
     * \param key specifies the key of the settings.
     * \param pluginName is inserted in the key after the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     */
    QgsSettingsEntryVariantMap( const QString &key,
                                const QString &pluginName,
                                const QVariantMap &defaultValue = QVariantMap(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_THROW( QgsSettingsException ) SIP_TRANSFER;
    % MethodCode
    sipCpp = new sipQgsSettingsEntryVariantMap( QgsSettingsEntryVariantMap( *a0, QgsSettings::createPluginTreeElement( *a1 ), *a2, *a3, *a4 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

  private:
    QVariantMap convertFromVariant( const QVariant &value ) const override SIP_FORCE;

};

#endif // QGSSETTINGSENTRYIMPL_H
