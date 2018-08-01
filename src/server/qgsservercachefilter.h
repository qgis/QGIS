/***************************************************************************
                          qgsservercachefilter.h
                          ------------------------
 Cache interface for Qgis Server plugins

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

#ifndef QGSSERVERCACHEPLUGIN_H
#define QGSSERVERCACHEPLUGIN_H

#include <QMultiMap>
#include <QDomDocument>
#include <QObject>
#include "qgsproject.h"
#include "qgsserverrequest.h"
#include "qgis_server.h"
#include "qgis_sip.h"

SIP_IF_MODULE( HAVE_SERVER_PYTHON_PLUGINS )

class QgsServerInterface;


/**
 * \ingroup server
 * \class QgsServerCacheFilter
 * \brief Class defining cache interface for QGIS Server plugins.
 */
class SERVER_EXPORT QgsServerCacheFilter
{

  public:

    /**
     * Constructor
     * QgsServerInterface passed to plugins constructors
     * and must be passed to QgsServerCacheFilter instances.
     */
    QgsServerCacheFilter( const QgsServerInterface *serverInterface );

    virtual ~QgsServerCacheFilter() = default;

    /**
     * Returns cached document (or 0 if document not in cache) like capabilities
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param key the key provided by the access control to identify differents documents for the same request
     * \returns QByteArray of the cached document or an empty one if no corresponding document found
     */
    virtual QByteArray getCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Updates or inserts the document in cache like capabilities
     * \param doc the document to cache
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param key the key provided by the access control to identify differents documents for the same request
     * \returns true if the document has been cached
     */
    virtual bool setCachedDocument( const QDomDocument *doc, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Deletes the cached document
     * \param project the project used to generate the document to provide path
     * \param request the request used to generate the document to provider parameters or data
     * \param key the key provided by the access control to identify differents documents for the same request
     * \returns true if the document has been deleted
     */
    virtual bool deleteCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Deletes all cached documents for a QGIS project
     * \param project the project used to generate the documents to provide path
     * \returns true if the documents have been deleted
     */
    virtual bool deleteCachedDocuments( const QgsProject *project ) const;

    /**
     * Returns cached image (or 0 if document not in cache) like tiles
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param key the key provided by the access control to identify differents images for the same request
     * \returns QByteArray of the cached image or an empty one if no corresponding image found
     */
    virtual QByteArray getCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Updates or inserts the image in cache like tiles
     * \param img the document to cache
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param key the key provided by the access control to identify differents images for the same request
     * \returns true if the image has been cached
     */
    virtual bool setCachedImage( const QByteArray *img, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Deletes the cached image
     * \param project the project used to generate the image to provide path
     * \param request the request used to generate the image to provider parameters or data
     * \param key the key provided by the access control to identify differents images for the same request
     * \returns true if the image has been deleted
     */
    virtual bool deleteCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const;

    /**
     * Deletes all cached images for a QGIS project
     * \param project the project used to generate the images to provide path
     * \returns true if the images have been deleted
     */
    virtual bool deleteCachedImages( const QgsProject *project ) const;

  private:

    //! The server interface
    const QgsServerInterface *mServerInterface = nullptr;

};

//! The registry definition
typedef QMultiMap<int, QgsServerCacheFilter *> QgsServerCacheFilterMap;

#endif // QGSSERVERSECURITY_H
