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
#include "qgis_sip.h"

class QgsAuthMethod;
class QgsAuthMethodMetadata;


/**
 * \ingroup core
 * \brief A registry / canonical manager of authentication methods.
 *
 * This is a Singleton class that manages authentication method plugin access.
 *
 * Loaded auth methods may be restricted using QGIS_AUTHMETHOD_FILE environment variable.
 * QGIS_AUTHMETHOD_FILE is regexp pattern applied to auth method file name (not auth method key).
 * For example, if the variable is set to basic|pkipaths it will load only auth methods
 * basic, and pkipaths.
 * \note not available in Python bindings
*/
class CORE_EXPORT QgsAuthMethodRegistry
{

  public:
    //! Means of accessing canonical single instance
    static QgsAuthMethodRegistry *instance( const QString &pluginPath = QString() );

    virtual ~QgsAuthMethodRegistry();

    /**
    * Returns path for the library of the auth method
    * \deprecated QGIS 3.22. Methods may not need to be loaded from a library (empty string returned).
    */
    Q_DECL_DEPRECATED QString library( const QString &authMethodKey ) const SIP_DEPRECATED;

    //! Returns list of auth method plugins found
    QString pluginList( bool asHtml = false ) const;

    //! Returns library directory where plugins are found
    QDir libraryDirectory() const;

    //! Sets library directory where to search for plugins
    void setLibraryDirectory( const QDir &path );

    /**
     * Create an instance of the auth method
     * \param authMethodKey identificator of the auth method
     * \returns instance of auth method or nullptr on error
     */
    QgsAuthMethod *createAuthMethod( const QString &authMethodKey );

#if 0

    /**
     * Returns the auth method capabilities
     * \param authMethodKey identificator of the auth method
     */
    // int authMethodCapabilities( const QString& authMethodKey ) const;
#endif

    /**
     * Returns the GUI edit widget associated with the auth method
     * \param parent Parent widget
     * \param authMethodKey identificator of the auth method
     */
    QWidget *editWidget( const QString &authMethodKey, QWidget *parent = nullptr );

    //! Returns list of available auth methods by their keys
    QStringList authMethodList() const;

    //! Returns metadata of the auth method or NULLPTR if not found
    const QgsAuthMethodMetadata *authMethodMetadata( const QString &authMethodKey ) const;

    //! Type for auth method metadata associative container
    typedef std::map<QString, QgsAuthMethodMetadata *> AuthMethods;

  private:

    //! Ctor private since instance() creates it
    QgsAuthMethodRegistry( const QString &pluginPath );

    void init();
    void clean();

    //! Associative container of auth method metadata handles
    AuthMethods mAuthMethods;

    //! Directory in which auth method plugins are installed
    QDir mLibraryDirectory;
};

#endif // QGSAUTHMETHODREGISTRY_H
