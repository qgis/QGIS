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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options argument specifies the options for the settings entry.
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
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryVariant( const QString &key,
                             const QString &pluginName,
                             const QVariant &defaultValue = QVariant(),
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions() );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryVariant( QgsSettingsEntryVariant( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), *a2, *a3, *a4 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     * The \a minLength argument specifies the minimal length of the string value. 0 means no limit.
     * The \a maxLength argument specifies the maximal length of the string value. -1 means no limit.
     */
    QgsSettingsEntryString( const QString &key,
                            const QString &section,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            int minLength = 0,
                            int maxLength = -1 ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference<QString>( key, section, defaultValue, description, options )
    , mMinLength( minLength )
    , mMaxLength( maxLength )
    {
    }

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryString.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryString( const QString &key,
                            const QString &pluginName,
                            const QString &defaultValue = QString(),
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            int minLength = 0,
                            int maxLength = -1 );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryString( QgsSettingsEntryString( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), *a2, *a3, *a4 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                const QString &section,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    {
    }

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryStringList.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryStringList( const QString &key,
                                const QString &pluginName,
                                const QStringList &defaultValue = QStringList(),
                                const QString &description = QString(),
                                Qgis::SettingsOptions options = Qgis::SettingsOptions() );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryStringList( QgsSettingsEntryStringList( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), *a2, *a3, *a4 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
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
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryBool( const QString &key,
                          const QString &pluginName,
                          bool defaultValue = false,
                          const QString &description = QString(),
                          Qgis::SettingsOptions options = Qgis::SettingsOptions() );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryBool( QgsSettingsEntryBool( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), a2, *a3, *a4 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
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
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is used to define the key of the setting
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
     */
    QgsSettingsEntryInteger( const QString &key,
                             const QString &pluginName,
                             qlonglong defaultValue = 0,
                             const QString &description = QString(),
                             Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                             qlonglong minValue = std::numeric_limits<qlonglong>::min(),
                             qlonglong maxValue = std::numeric_limits<qlonglong>::max() );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryInteger( QgsSettingsEntryInteger( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), a2, *a3, *a4, a5, a6 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
     * The \a displayDecimals specifies an hint for the gui about how much decimals to show
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
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is used to define the key of the setting
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a minValue argument specifies the minimal value.
     * The \a maxValue argument specifies the maximal value.
     * The \a displayDecimals specifies an hint for the gui about how much decimals to show
     */
    QgsSettingsEntryDouble( const QString &key,
                            const QString &pluginName,
                            double defaultValue,
                            const QString &description = QString(),
                            Qgis::SettingsOptions options = Qgis::SettingsOptions(),
                            double minValue = std::numeric_limits<double>::lowest(),
                            double maxValue = std::numeric_limits<double>::max(),
                            int displayDecimals = 1 );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryDouble( QgsSettingsEntryDouble( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), a2, *a3, *a4, a5, a6, a7 ) );
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
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryColor( const QString &key,
                           const QString &section,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString(),
                           Qgis::SettingsOptions options = Qgis::SettingsOptions() ) SIP_MAKE_PRIVATE
  : QgsSettingsEntryByReference( key, section, defaultValue, description, options )
    {}

#ifdef SIP_RUN

    /**
     * Constructor for QgsSettingsEntryColor.
     * This constructor is intended to be used from plugins.
     *
     * The \a key argument specifies the key of the settings.
     * The \a pluginName argument is inserted in the key after the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     * The \a options arguments specifies the options for the settings entry.
     */
    QgsSettingsEntryColor( const QString &key,
                           const QString &pluginName,
                           const QColor &defaultValue = QColor(),
                           const QString &description = QString(),
                           Qgis::SettingsOptions options = Qgis::SettingsOptions() );
    % MethodCode
    sipCpp = new sipQgsSettingsEntryColor( QgsSettingsEntryColor( *a0, QStringLiteral( "plugins/%1" ).arg( *a1 ), *a2, *a3, *a4 ) );
    % End
#endif

    virtual Qgis::SettingsType settingsType() const override;

  private:
    QColor convertFromVariant( const QVariant &value ) const override SIP_FORCE;

};

#endif // QGSSETTINGSENTRYIMPL_H
