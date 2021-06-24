/***************************************************************************
    qgsauthmethodmetadata.h
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

#ifndef QGSAUTHMETHODMETADATA_H
#define QGSAUTHMETHODMETADATA_H

#define SIP_NO_FILE

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsAuthMethod;

/**
 * \ingroup core
 * \brief Holds data auth method key, description, and associated shared library file information.
 *
 * The metadata class is used in a lazy load implementation in
 * QgsAuthMethodRegistry.  To save memory, auth methods are only actually
 * loaded via QLibrary calls if they're to be used.  (Though they're all
 * iteratively loaded once to get their metadata information, and then
 * unloaded when the QgsAuthMethodRegistry is created.)  QgsProviderMetadata
 * supplies enough information to be able to later load the associated shared
 * library object.
 *
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsAuthMethodMetadata
{
  public:

    /**
     * Construct an authentication method metadata container
     * \param key Textual key of the library plugin
     * \param description Description of the library plugin
     * \param library File name of library plugin (empty if the provider is not loaded from a library)

     */
    QgsAuthMethodMetadata( const QString &key, const QString &description, const QString &library = QString() )
      : mKey( key )
      , mDescription( description )
      , mLibrary( library )
    {}

    virtual ~QgsAuthMethodMetadata() = default;

    /**
     * Returns the unique key associated with the method
     *
     * This key string is used for the associative container in QgsAtuhMethodRegistry
     */
    QString key() const;

    /**
     * Returns descriptive text for the method.
     *
     * This is used to provide a descriptive list of available data methods.
     */
    QString description() const;

    /**
     * Returns the library file name.
     *
     * This is used to QLibrary calls to load the method.
     */
    QString library() const;

    /**
     * Class factory to return a pointer to a newly created QgsDataProvider object
     * \since QGIS 3.22
     */
    virtual QgsAuthMethod *createAuthMethod() const SIP_FACTORY; // TODO QGIS 4 = 0

    //virtual QStringList supportedDataProviders() const; // TODO QGIS 4 = 0;

  private:

    /// unique key for method
    QString mKey;

    /// associated terse description
    QString mDescription;

    /// file path
    QString mLibrary;
};

#endif // QGSAUTHMETHODMETADATA_H
