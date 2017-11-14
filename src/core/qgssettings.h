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
#include "qgis.h"

/**
 * \ingroup core
 * \class QgsSettings
 *
 * This class is a composition of two QSettings instances:
 * - the main QSettings instance is the standard User Settings and
 * - the second one (Global Settings) is meant to provide read-only
 *   pre-configuration and defaults to the first one.
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
 *  - Auth
 *  - App
 *  - Providers
 *  - Misc
 *
 * \since QGIS 3
 */
class CORE_EXPORT QgsSettings : public QObject
{
    Q_OBJECT
  public:

    //! Sections for namespaced settings
    enum Section
    {
      NoSection,
      Core,
      Gui,
      Server,
      Plugins,
      Auth,
      App,
      Providers,
      Misc
    };

    /**
     * Construct a QgsSettings object for accessing settings of the application
     * called application from the organization called organization, and with parent parent.
     */
    explicit QgsSettings( const QString &organization,
                          const QString &application = QString(), QObject *parent = nullptr );

    /**
     * Construct a QgsSettings object for accessing settings of the application called application
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
                 const QString &application = QString(), QObject *parent = nullptr );

    /**
     * Construct a QgsSettings object for accessing settings of the application called application
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
                 const QString &application = QString(), QObject *parent = nullptr );

    /**
     * Construct a QgsSettings object for accessing the settings stored in the file called fileName,
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
    QgsSettings( const QString &fileName, QSettings::Format format, QObject *parent = nullptr );

    /**
     * Constructs a QgsSettings object for accessing settings of the application and organization
     * set previously with a call to QCoreApplication::setOrganizationName(),
     * QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
     *
     * The scope is QSettings::UserScope and the format is defaultFormat() (QSettings::NativeFormat
     * by default). Use setDefaultFormat() before calling this constructor to change the default
     * format used by this constructor.
     */
    explicit QgsSettings( QObject *parent = 0 );
    ~QgsSettings();

    /**
     * Appends prefix to the current group.
     * The current group is automatically prepended to all keys specified to QSettings.
     * In addition, query functions such as childGroups(), childKeys(), and allKeys()
     * are based on the group. By default, no group is set.
     */
    void beginGroup( const QString &prefix, const QgsSettings::Section section = QgsSettings::NoSection );
    //! Resets the group to what it was before the corresponding beginGroup() call.
    void endGroup();
    //! Returns a list of all keys, including subkeys, that can be read using the QSettings object.
    QStringList allKeys() const;
    //! Returns a list of all top-level keys that can be read using the QSettings object.
    QStringList childKeys() const;
    //! Returns a list of all key top-level groups that contain keys that can be read using the QSettings object.
    QStringList childGroups() const;
    //! Returns a list of all key top-level groups (same as childGroups) but only for groups defined in global settings.
    QStringList globalChildGroups() const;
    //! Return the path to the Global Settings QSettings storage file
    static QString globalSettingsPath() { return sGlobalSettingsPath; }
    //! Set the Global Settings QSettings storage file
    static bool setGlobalSettingsPath( const QString &path );
    //! Adds prefix to the current group and starts reading from an array. Returns the size of the array.
    int beginReadArray( const QString &prefix );

    /**
     * Adds prefix to the current group and starts writing an array of size size.
     * If size is -1 (the default), it is automatically determined based on the indexes of the entries written.
     * \note This will completely shadow any existing array with the same name in the global settings
     */
    void beginWriteArray( const QString &prefix, int size = -1 );
    //! Closes the array that was started using beginReadArray() or beginWriteArray().
    void endArray();

    /**
     * Sets the current array index to i. Calls to functions such as setValue(), value(),
     * remove(), and contains() will operate on the array entry at that index.
     */
    void setArrayIndex( int i );

    /**
     * Sets the value of setting key to value. If the key already exists, the previous value is overwritten.
     * An optional Section argument can be used to set a value to a specific Section.
     */
    void setValue( const QString &key, const QVariant &value, const QgsSettings::Section section = QgsSettings::NoSection );

    /**
     * Returns the value for setting key. If the setting doesn't exist, it will be
     * searched in the Global Settings and if not found, returns defaultValue.
     * If no default value is specified, a default QVariant is returned.
     * An optional Section argument can be used to get a value from a specific Section.
     */
#ifndef SIP_RUN
    QVariant value( const QString &key, const QVariant &defaultValue = QVariant(),
                    const Section section = NoSection ) const;
#else
    SIP_PYOBJECT value( const QString &key, const QVariant &defaultValue = QVariant(),
                        SIP_PYOBJECT type = 0,
                        QgsSettings::Section section = QgsSettings::NoSection ) const / ReleaseGIL /;
    % MethodCode
    typedef PyObject *( *pyqt5_from_qvariant_by_type )( QVariant &value, PyObject *type );
    QVariant value;

    // QSettings has an internal mutex so release the GIL to avoid the possibility of deadlocks.
    Py_BEGIN_ALLOW_THREADS
    value = sipCpp->value( *a0, *a1, a3 );
    Py_END_ALLOW_THREADS

    pyqt5_from_qvariant_by_type f = ( pyqt5_from_qvariant_by_type ) sipImportSymbol( "pyqt5_from_qvariant_by_type" );
    sipRes = f( value, a2 );

    sipIsErr = !sipRes;
    % End
#endif

    /**
     * Returns true if there exists a setting called key; returns false otherwise.
     * If a group is set using beginGroup(), key is taken to be relative to that group.
     */
    bool contains( const QString &key, const QgsSettings::Section section = QgsSettings::NoSection ) const;
    //! Returns the path where settings written using this QSettings object are stored.
    QString fileName() const;

    /**
     * Writes any unsaved changes to permanent storage, and reloads any settings that have been
     * changed in the meantime by another application.
     * This function is called automatically from QSettings's destructor and by the event
     * loop at regular intervals, so you normally don't need to call it yourself.
     */
    void sync();
    //! Removes the setting key and any sub-settings of key in a section.
    void remove( const QString &key, const QgsSettings::Section section = QgsSettings::NoSection );
    //! Return the sanitized and prefixed key
    QString prefixedKey( const QString &key, const QgsSettings::Section section ) const;
    //! Removes all entries in the user settings
    void clear();

  private:

    static QString sGlobalSettingsPath;
    void init();
    QString sanitizeKey( const QString &key ) const;
    QSettings *mUserSettings = nullptr;
    QSettings *mGlobalSettings = nullptr;
    bool mUsingGlobalArray = false;
    Q_DISABLE_COPY( QgsSettings )

};

#endif // QGSSETTINGS_H
