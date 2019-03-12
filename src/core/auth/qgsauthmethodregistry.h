/***************************************************************************
    qgsauthmethodregistry.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHMETHODREGISTRY_H
#define QGSAUTHMETHODREGISTRY_H

#define SIP_NO_FILE

#include <QDir>
#include <QLibrary>
#include <QMap>
#include <QString>
#include <memory>

#include "qgis_core.h"

class QgsAuthMethod;
class QgsAuthMethodMetadata;


/**
 * \ingroup core
  * A registry / canonical manager of authentication methods.

  This is a Singleton class that manages authentication method plugin access.

  Loaded auth methods may be restricted using QGIS_AUTHMETHOD_FILE environment variable.
  QGIS_AUTHMETHOD_FILE is regexp pattern applied to auth method file name (not auth method key).
  For example, if the variable is set to basic|pkipaths it will load only auth methods
  basic, and pkipaths.
  \note not available in Python bindings
*/
class CORE_EXPORT QgsAuthMethodRegistry
{

  public:
    //! Means of accessing canonical single instance
    static QgsAuthMethodRegistry *instance( const QString &pluginPath = QString() );

    //! Virtual dectructor
    virtual ~QgsAuthMethodRegistry();

    //! Returns path for the library of the auth method
    QString library( const QString &authMethodKey ) const;

    //! Returns list of auth method plugins found
    QString pluginList( bool asHtml = false ) const;

    //! Returns library directory where plugins are found
    QDir libraryDirectory() const;

    //! Sets library directory where to search for plugins
    void setLibraryDirectory( const QDir &path );

    /**
     * Create an instance of the auth method
        \param authMethodKey identificator of the auth method
        \returns instance of auth method or nullptr on error
     */
    std::unique_ptr< QgsAuthMethod > authMethod( const QString &authMethodKey );

    /**
     * Returns the auth method capabilities
        \param authMethodKey identificator of the auth method
     */
    // int authMethodCapabilities( const QString& authMethodKey ) const;

    /**
     * Returns the GUI edit widget associated with the auth method
     * \param parent Parent widget
     * \param authMethodKey identificator of the auth method
     */
    QWidget *editWidget( const QString &authMethodKey, QWidget *parent = nullptr );

    /**
     * Gets pointer to auth method function
        \param authMethodKey identificator of the auth method
        \param functionName name of function
        \returns pointer to function or nullptr on error
     */
    QFunctionPointer function( const QString &authMethodKey,
                               const QString &functionName );

    //! Returns the library object associated with an auth method key
    std::unique_ptr< QLibrary > authMethodLibrary( const QString &authMethodKey ) const;

    //! Returns list of available auth methods by their keys
    QStringList authMethodList() const;

    //! Returns metadata of the auth method or NULLPTR if not found
    const QgsAuthMethodMetadata *authMethodMetadata( const QString &authMethodKey ) const;

//    void registerGuis( QWidget *widget );

    //! Type for auth method metadata associative container
    typedef std::map<QString, QgsAuthMethodMetadata *> AuthMethods;

  private:
    //! Ctor private since instance() creates it
    QgsAuthMethodRegistry( const QString &pluginPath );

    //! Associative container of auth method metadata handles
    AuthMethods mAuthMethods;

    //! Directory in which auth method plugins are installed
    QDir mLibraryDirectory;
};

#endif // QGSAUTHMETHODREGISTRY_H
