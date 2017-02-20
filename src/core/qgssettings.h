/***************************************************************************
  qgssettings.h
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGS_H
#define QGSSETTINGS_H

#include <QSettings>
#include "qgis_core.h"

/** \ingroup core
 * \class QgsSettings
 *
 * This class is a drop-in replacement of QSettings that wraps an additional QSettings instance.
 * The inherited QSettings instance is the standard User Settings and the second one
 * (Global Settings) is meant to provide read-only pre-configuration and defaults to the
 * first one.
 *
 * Unlike the original QSettings, the keys of QgsSettings are case insensitive.
 *
 * For a given settings key, the function call to value(key, default) will return
 * the first existing setting in the order specified below:
 *  - User Settings
 *  - Global Settings
 *  - Default Value
 *
 * The path to the Global Settings storage can be set before constructing the QgsSettings
 * objects, with a static call to:
 * static bool setGlobalSettingsPath( QString path );
 *
 * QgsSettings provides some shortcuts to get/set namespaced settings from/to a specific section:
 *  - Core
 *  - Gui
 *  - Server
 *  - Plugins
 *  - Misc
 *
 * For example, the following getter and setter, will set/get a value from the Gui section:
 *  - setGuiValue( const QString &key, const QVariant &value );
 *  - QVariant guiValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
 *
 * @note added in QGIS 3
 */
class CORE_EXPORT QgsSettings : public QSettings
{
  public:

    //! Sections for namespaced settings
    enum Section
    {
      Core,
      Gui,
      Server,
      Plugins,
      Misc
    };

    /** Construct a QgsSettings object for accessing settings of the application
     * called application from the organization called organization, and with parent parent.
    */
    explicit QgsSettings( const QString &organization,
                          const QString &application = QString(), QObject *parent = 0 );

    /** Construct a QgsSettings object for accessing settings of the application called application
     * from the organization called organization, and with parent parent.
     * If scope is QSettings::UserScope, the QSettings object searches user-specific settings first,
     * before it searches system-wide settings as a fallback. If scope is QSettings::SystemScope,
     * the QSettings object ignores user-specific settings and provides access to system-wide settings.
     *
     * The storage format is set to QSettings::NativeFormat (i.e. calling setDefaultFormat() before
     * calling this constructor has no effect).
     *
     * If no application name is given, the QSettings object will only access the organization-wide
     * locations.
     */
    QgsSettings( QSettings::Scope scope, const QString &organization,
                 const QString &application = QString(), QObject *parent = 0 );

    /** Construct a QgsSettings object for accessing settings of the application called application
     * from the organization called organization, and with parent parent.
     * If scope is QSettings::UserScope, the QSettings object searches user-specific settings first,
     * before it searches system-wide settings as a fallback. If scope is QSettings::SystemScope,
     * the QSettings object ignores user-specific settings and provides access to system-wide settings.
     * If format is QSettings::NativeFormat, the native API is used for storing settings. If format
     * is QSettings::IniFormat, the INI format is used.
     *
     * If no application name is given, the QSettings object will only access the organization-wide
     * locations.
     */
    QgsSettings( QSettings::Format format, QSettings::Scope scope, const QString &organization,
                 const QString &application = QString(), QObject *parent = 0 );

    /** Construct a QgsSettings object for accessing the settings stored in the file called fileName,
     * with parent parent. If the file doesn't already exist, it is created.
     *
     * If format is QSettings::NativeFormat, the meaning of fileName depends on the platform. On Unix,
     * fileName is the name of an INI file. On macOS and iOS, fileName is the name of a .plist file.
     * On Windows, fileName is a path in the system registry.
     *
     * If format is QSettings::IniFormat, fileName is the name of an INI file.
     *
     * Warning: This function is provided for convenience. It works well for accessing INI or .plist
     * files generated by Qt, but might fail on some syntaxes found in such files originated by
     * other programs. In particular, be aware of the following limitations:
     *  - QgsSettings provides no way of reading INI "path" entries, i.e., entries with unescaped slash characters.
     *    (This is because these entries are ambiguous and cannot be resolved automatically.)
     *  - In INI files, QSettings uses the @ character as a metacharacter in some contexts, to encode
     *     Qt-specific data types (e.g., \@Rect), and might therefore misinterpret it when it occurs
     *     in pure INI files.
     */
    QgsSettings( const QString &fileName, QSettings::Format format, QObject *parent = 0 );

    /** Constructs a QgsSettings object for accessing settings of the application and organization
     * set previously with a call to QCoreApplication::setOrganizationName(),
     * QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
     *
     * The scope is QSettings::UserScope and the format is defaultFormat() (QSettings::NativeFormat
     * by default). Use setDefaultFormat() before calling this constructor to change the default
     * format used by this constructor.
     */
    explicit QgsSettings( QObject *parent = 0 );
    ~QgsSettings();

    /** Appends prefix to the current group.
     * The current group is automatically prepended to all keys specified to QSettings.
     * In addition, query functions such as childGroups(), childKeys(), and allKeys()
     * are based on the group. By default, no group is set.
     */
    void beginGroup( const QString &prefix );
    //! Resets the group to what it was before the corresponding beginGroup() call.
    void endGroup();
    //! Returns a list of all keys, including subkeys, that can be read using the QSettings object.
    QStringList allKeys() const;
    //! Returns a list of all top-level keys that can be read using the QSettings object.
    QStringList childKeys() const;
    //! Returns a list of all key top-level groups that contain keys that can be read using the QSettings object.
    QStringList childGroups() const;
    //! Return the path to the Global Settings QSettings storage file
    static QString globalSettingsPath() { return sGlobalSettingsPath; }
    //! Set the Global Settings QSettings storage file
    static bool setGlobalSettingsPath( QString path );
    //! Adds prefix to the current group and starts reading from an array. Returns the size of the array.
    int beginReadArray( const QString &prefix );
    //! Closes the array that was started using beginReadArray() or beginWriteArray().
    void endArray();
    //! Sets the current array index to i. Calls to functions such as setValue(), value(), remove(), and contains() will operate on the array entry at that index.
    void setArrayIndex( int i );

    /** Returns the value for setting key. If the setting doesn't exist, it will be
     * searched in the Global Settings and if not found, returns defaultValue.
     *If no default value is specified, a default QVariant is returned.
     */
    QVariant value( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that accepts an additional Section argument
    QVariant sectionValue( const QString &key, const Section section, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that gets a core prefix
    QVariant coreValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that gets a server prefix
    QVariant serverValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that gets a gui prefix
    QVariant guiValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that gets a plugins prefix
    QVariant pluginsValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded getter that gets a misc prefix
    QVariant miscValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Overloaded setValue that accepts an additional Section argument
    void setSectionValue( const QString &key, const Section section, const QVariant &value );
    //! Overloaded setValue that sets a core prefix
    void setCoreValue( const QString &key, const QVariant &value );
    //! Overloaded setValue that sets a server prefix
    void setServerValue( const QString &key, const QVariant &value );
    //! Overloaded setValue that sets a gui prefix
    void setGuiValue( const QString &key, const QVariant &value );
    //! Overloaded setValue that sets a plugins prefix
    void setPluginsValue( const QString &key, const QVariant &value );
    //! Overloaded setValue that sets a misc prefix
    void setMiscValue( const QString &key, const QVariant &value );
    //! Return the sanitized and prefixed key
    QString prefixedKey( const QString &key, const Section section ) const;

  private:

    void init( );
    QString sanitizeKey( QString key ) const;
    QSettings* mGlobalSettings = nullptr;
    static QString sGlobalSettingsPath;
    bool mUsingGlobalArray = false;
    Q_DISABLE_COPY( QgsSettings )

};

#endif // QGSSETTINGS_H
