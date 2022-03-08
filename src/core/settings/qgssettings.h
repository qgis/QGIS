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
#include <QMetaEnum>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslogger.h"

/**
 * \ingroup core
 * \class QgsSettings
 *
 * \brief This class is a composition of two QSettings instances:
 *
 * - the main QSettings instance is the standard User Settings and
 * - the second one (Global Settings) is meant to provide read-only
 *   pre-configuration and defaults to the first one.
 *
 * For a given settings key, the function call to value(key, default) will return
 * the first existing setting in the order specified below:
 *
 * - User Settings
 * - Global Settings
 * - Default Value
 *
 * The path to the Global Settings storage can be set before constructing the QgsSettings
 * objects, with a static call to:
 * static bool setGlobalSettingsPath( QString path );
 *
 * QgsSettings provides some shortcuts to get/set namespaced settings from/to a specific section:
 *
 * - Core
 * - Gui
 * - Server
 * - Plugins
 * - Auth
 * - App
 * - Providers
 * - Misc
 *
 * \since QGIS 3.0
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
      Expressions,
      Misc,
      Gps, //!< GPS section, since QGIS 3.22
    };

    /**
     * \ingroup core
     * \brief Prefixes for the settings keys
     * \since QGIS 3.26
     */
    class Prefix SIP_SKIP
    {
      public:
        static const inline char *APP_GEOREFERENCER = "app/georeferencer";
        static const inline char *CORE = "core";
        static const inline char *CORE_LAYOUT = "core/Layout";
        static const inline char *GEOMETRYVALIDATION = "geometry_validation";
        static const inline char *GPS = "gps";
        static const inline char *GUI_LOCATORFILTERS = "gui/locator_filters";
        static const inline char *GUI_QGIS = "gui/qgis";
        static const inline char *LOCALE = "locale";
        static const inline char *MAP = "Map";
        static const inline char *PLUGINS = "plugins";
        static const inline char *PROCESSING_CONFIGURATION = "Processing/Configuration";
        static const inline char *QGIS = "qgis";
        static const inline char *QGIS_DIGITIZING = "qgis/digitizing";
        static const inline char *QGIS_DIGITIZING_SHAPEMAPTOOLS = "qgis/digitizing/shape-map-tools";
        static const inline char *QGIS_NETWORKANDPROXY = "qgis/networkAndProxy";
        static const inline char *SVG = "svg";
    };

    /**
     * Constructs a QgsSettings object for accessing settings of the application
     * called application from the organization called organization, and with parent parent.
     */
    explicit QgsSettings( const QString &organization,
                          const QString &application = QString(), QObject *parent = nullptr );

    /**
     * Constructs a QgsSettings object for accessing settings of the application called application
     * from the organization called organization, and with parent parent.
     *
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
     * Constructs a QgsSettings object for accessing settings of the application called application
     * from the organization called organization, and with parent parent.
     *
     * If scope is QSettings::UserScope, the QSettings object searches user-specific settings first,
     * before it searches system-wide settings as a fallback. If scope is QSettings::SystemScope,
     * the QSettings object ignores user-specific settings and provides access to system-wide settings.
     *
     * If format is QSettings::NativeFormat, the native API is used for storing settings. If format
     * is QSettings::IniFormat, the INI format is used.
     *
     * If no application name is given, the QSettings object will only access the organization-wide
     * locations.
     */
    QgsSettings( QSettings::Format format, QSettings::Scope scope, const QString &organization,
                 const QString &application = QString(), QObject *parent = nullptr );

    /**
     * Constructs a QgsSettings object for accessing the settings stored in the file called fileName,
     * with parent parent. If the file doesn't already exist, it is created.
     *
     * If format is QSettings::NativeFormat, the meaning of fileName depends on the platform. On Unix,
     * fileName is the name of an INI file. On macOS and iOS, fileName is the name of a .plist file.
     * On Windows, fileName is a path in the system registry.
     *
     * If format is QSettings::IniFormat, fileName is the name of an INI file.
     *
     * \warning This function is provided for convenience. It works well for accessing INI or .plist
     * files generated by Qt, but might fail on some syntaxes found in such files originated by
     * other programs. In particular, be aware of the following limitations:
     *
     * - QgsSettings provides no way of reading INI "path" entries, i.e., entries with unescaped slash characters.
     *   (This is because these entries are ambiguous and cannot be resolved automatically.)
     * - In INI files, QSettings uses the @ character as a metacharacter in some contexts, to encode
     *   Qt-specific data types (e.g., \@Rect), and might therefore misinterpret it when it occurs
     *   in pure INI files.
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
    explicit QgsSettings( QObject *parent = nullptr );
    ~QgsSettings() override;

    /**
     * Appends prefix to the current group.
     * The current group is automatically prepended to all keys specified to QSettings.
     * In addition, query functions such as childGroups(), childKeys(), and allKeys()
     * are based on the group. By default, no group is set.
     */
    void beginGroup( const QString &prefix, QgsSettings::Section section = QgsSettings::NoSection );
    //! Resets the group to what it was before the corresponding beginGroup() call.
    void endGroup();

    /**
     * Returns the current group.
     * \see beginGroup()
     * \see endGroup()
     * \since QGIS 3.6
     */
    QString group() const;

    //! Returns a list of all keys, including subkeys, that can be read using the QSettings object.
    QStringList allKeys() const;
    //! Returns a list of all top-level keys that can be read using the QSettings object.
    QStringList childKeys() const;
    //! Returns a list of all key top-level groups that contain keys that can be read using the QSettings object.
    QStringList childGroups() const;
    //! Returns a list of all key top-level groups (same as childGroups) but only for groups defined in global settings.
    QStringList globalChildGroups() const;
    //! Returns the path to the Global Settings QSettings storage file
    static QString globalSettingsPath();
    //! Sets the Global Settings QSettings storage file
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
    void setValue( const QString &key, const QVariant &value, QgsSettings::Section section = QgsSettings::NoSection );

    /**
     * Returns the value for setting key. If the setting doesn't exist, it will be
     * searched in the Global Settings and if not found, returns defaultValue.
     * If no default value is specified, a default QVariant is returned.
     * An optional Section argument can be used to get a value from a specific Section.
     */
#ifndef SIP_RUN
    QVariant value( const QString &key, const QVariant &defaultValue = QVariant(),
                    Section section = NoSection ) const;
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

#ifndef SIP_RUN

    /**
     * Returns the setting value for a setting based on an enum.
     * This forces the output to be a valid and existing entry of the enum.
     * Hence if the setting value is incorrect, the given default value is returned.
     * This tries first with setting as a string (as the enum) and then as an integer value.
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly
     * \see setEnumValue
     * \see flagValue
     */
    template <class T>
    T enumValue( const QString &key, const T &defaultValue,
                 const Section section = NoSection )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( !metaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }

      T v;
      bool ok = false;

      if ( metaEnum.isValid() )
      {
        // read as string
        QByteArray ba = value( key, metaEnum.valueToKey( static_cast<const int>( defaultValue ) ), section ).toString().toUtf8();
        const char *vs = ba.data();
        v = static_cast<T>( metaEnum.keyToValue( vs, &ok ) );
        if ( ok )
          return v;
      }

      // if failed, try to read as int (old behavior)
      // this code shall be removed later (probably after QGIS 3.4 LTR for 3.6)
      // then the method could be marked as const
      v = static_cast<T>( value( key, static_cast<const int>( defaultValue ), section ).toInt( &ok ) );
      if ( metaEnum.isValid() )
      {
        if ( !ok || !metaEnum.valueToKey( static_cast<int>( v ) ) )
        {
          v = defaultValue;
        }
        else
        {
          // found setting as an integer
          // convert the setting to the new form (string)
          setEnumValue( key, v, section );
        }
      }

      return v;
    }

    /**
     * Set the value of a setting based on an enum.
     * The setting will be saved as string.
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \see enumValue
     * \see setFlagValue
     */
    template <class T>
    void setEnumValue( const QString &key, const T &value,
                       const Section section = NoSection )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( metaEnum.isValid() )
      {
        setValue( key, metaEnum.valueToKey( static_cast<const int>( value ) ), section );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }
    }

    /**
     * Returns the setting value for a setting based on a flag.
     * This forces the output to be a valid and existing entry of the flag.
     * Hence if the setting value is incorrect, the given default value is returned.
     * This tries first with setting as a string (using a byte array) and then as an integer value.
     * \note The flag needs to be declared with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly.
     * \see setFlagValue
     * \see enumValue
     */
    template <class T>
    T flagValue( const QString &key, const T &defaultValue,
                 const Section section = NoSection )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( !metaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }

      T v = defaultValue;
      bool ok = false;

      if ( metaEnum.isValid() )
      {
        // read as string
        QByteArray ba = value( key, metaEnum.valueToKeys( static_cast< const int >( defaultValue ) ) ).toString().toUtf8();
        const char *vs = ba.data();
        v = static_cast<T>( metaEnum.keysToValue( vs, &ok ) );
      }
      if ( !ok )
      {
        // if failed, try to read as int
        const int intValue = value( key, static_cast<const int>( defaultValue ), section ).toInt( &ok );
        if ( metaEnum.isValid() )
        {
          if ( ok )
          {
            // check that the int value does correspond to a flag
            // see https://stackoverflow.com/a/68495949/1548052
            const QByteArray keys = metaEnum.valueToKeys( intValue );
            const int intValueCheck = metaEnum.keysToValue( keys );
            if ( intValue != intValueCheck )
            {
              v = defaultValue;
            }
            else
            {
              // found property as an integer
              v = T( intValue );
              // convert the property to the new form (string)
              // this code could be removed
              // then the method could be marked as const
              setFlagValue( key, v );
            }
          }
          else
          {
            v = defaultValue;
          }
        }
      }

      return v;
    }

    /**
     * Set the value of a setting based on a flag.
     * The setting will be saved as string.
     * \note The flag needs to be declared with Q_FLAG (not Q_FLAGS).
     * \see flagValue
     * \see setEnumValue
     */
    template <class T>
    void setFlagValue( const QString &key, const T &value,
                       const Section section = NoSection )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( metaEnum.isValid() )
      {
        setValue( key, metaEnum.valueToKeys( static_cast< const int >( value ) ), section );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }
    }
#endif

    /**
     * Returns TRUE if there exists a setting called key; returns FALSE otherwise.
     * If a group is set using beginGroup(), key is taken to be relative to that group.
     */
    bool contains( const QString &key, QgsSettings::Section section = QgsSettings::NoSection ) const;
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
    void remove( const QString &key, QgsSettings::Section section = QgsSettings::NoSection );
    //! Returns the sanitized and prefixed key
    QString prefixedKey( const QString &key, QgsSettings::Section section ) const;
    //! Removes all entries in the user settings
    void clear();

  private:
    void init();
    QString sanitizeKey( const QString &key ) const;
    QSettings *mUserSettings = nullptr;
    QSettings *mGlobalSettings = nullptr;
    bool mUsingGlobalArray = false;
    Q_DISABLE_COPY( QgsSettings )

};

#endif // QGSSETTINGS_H
