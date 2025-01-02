/***************************************************************************
                        qgsservercachemanager.h
                        -----------------------

  begin                : 2018-07-05
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERCACHEMANAGER_H
#define QGSSERVERCACHEMANAGER_H

#include "qgsservercachefilter.h"
#include "qgsaccesscontrol.h"
#include "qgsserverrequest.h"

#include <QDomDocument>
#include "qgis_server.h"
#include "qgis_sip.h"
#include "qgsserversettings.h"

class QgsProject;

SIP_IF_MODULE( HAVE_SERVER_PYTHON_PLUGINS )

/**
 * \ingroup server
 * \class QgsServerCacheManager
 * \brief A helper class that centralizes caches accesses given by all the server cache filter plugins.
 * \since QGIS 3.4
 */
class SERVER_EXPORT QgsServerCacheManager
{
#ifdef SIP_RUN
#include "qgsservercachefilter.h"
#endif

  public:
    //! Constructor
    QgsServerCacheManager( const QgsServerSettings &settings = QgsServerSettings() );

    QgsServerCacheManager( const QgsServerCacheManager &copy );
    QgsServerCacheManager &operator=( const QgsServerCacheManager &copy );
    ~QgsServerCacheManager();

    /**
     * Returns cached document (or 0 if document not in cache) like capabilities
     * \param doc the document to update by content found in cache
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns TRUE if the document has been found in cache and the document's content set
     */
    bool getCachedDocument( QDomDocument *doc, const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Updates or inserts the document in cache like capabilities
     * \param doc the document to cache
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns TRUE if the document has been cached
     */
    bool setCachedDocument( const QDomDocument *doc, const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Deletes the cached document
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns TRUE if the document has been deleted
     */
    bool deleteCachedDocument( const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Deletes all cached documents for a QGIS project
     * \param project the project used to generate the document to provide path
     * \returns TRUE if the document has been deleted
     */
    bool deleteCachedDocuments( const QgsProject *project ) const;

    /**
     * Returns cached image (or 0 if image not in cache) like tiles
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns the cached image or 0 if no corresponding image found
     */
    QByteArray getCachedImage( const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Updates or inserts the image in cache like tiles
     * \param img the image to cache
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns TRUE if the image has been cached
     */
    bool setCachedImage( const QByteArray *img, const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Deletes the cached image
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param accessControl the access control to identify different documents for the same request provided by server interface
     * \returns TRUE if the image has been deleted
     */
    bool deleteCachedImage( const QgsProject *project, const QgsServerRequest &request, QgsAccessControl *accessControl ) const;

    /**
     * Deletes all cached images for a QGIS project
     * \param project the project used to generate the images to provide path
     * \returns TRUE if the images have been deleted
     */
    bool deleteCachedImages( const QgsProject *project ) const;

    /**
     * Register a server cache filter
     * \param serverCache the server cache to add
     * \param priority the priority used to define the order
     */
    void registerServerCache( QgsServerCacheFilter *serverCache, int priority = 0 );

  private:
    QString getCacheKey( bool &cache, QgsAccessControl *accessControl, const QgsServerRequest &request ) const;
    //! The ServerCache plugins registry
    std::unique_ptr<QgsServerCacheFilterMap> mPluginsServerCaches = nullptr;
    const QgsServerSettings &mSettings;
};

#endif
