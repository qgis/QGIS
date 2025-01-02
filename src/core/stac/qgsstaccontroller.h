/***************************************************************************
    qgsstaccontroller.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACCONTROLLER_H
#define QGSSTACCONTROLLER_H

#define SIP_NO_FILE

#include <QObject>
#include <nlohmann/json.hpp>

#include "qgis_core.h"
#include "qgshttpheaders.h"
#include "qgsnetworkreply.h"

class QgsStacObject;
class QgsStacCatalog;
class QgsStacCollection;
class QgsStacCollections;
class QgsStacItem;
class QgsStacItemCollection;
class QNetworkReply;

/**
 * \ingroup core
 * \brief The QgsStacController class handles STAC requests
 *
 * Contains methods to generate STAC objects from local and remote urls
 *
 * \note not available in Python bindings
 * \since QGIS 3.40
*/
class CORE_EXPORT QgsStacController : public QObject
{
    Q_OBJECT
  public:
    //! Default constructor
    explicit QgsStacController() = default;

    //! Default destructor
    ~QgsStacController();

    /**
     *  Returns a STAC Catalog by parsing a local file
     *  The caller takes ownership of the returned catalog
     */
    QgsStacCatalog *openLocalCatalog( const QString &fileName ) const;

    /**
     *  Returns a STAC Collection by parsing a local file
     *  The caller takes ownership of the returned collection
     */
    QgsStacCollection *openLocalCollection( const QString &fileName ) const;

    /**
     *  Returns a STAC Item by parsing a local file
     *  The caller takes ownership of the returned item
     */
    QgsStacItem *openLocalItem( const QString &fileName ) const;

    /**
     * Fetches a STAC object from \a url using a blocking network request.
     * An optional \a error parameter will be populated with any network error information.
     * The caller takes ownership of the returned object
     */
    QgsStacObject *fetchStacObject( const QUrl &url, QString *error = nullptr );

    /**
     * Fetches a feature collection from \a url using a blocking network request.
     * An optional \a error parameter will be populated with any network error information.
     * The caller takes ownership of the returned feature collection
     */
    QgsStacItemCollection *fetchItemCollection( const QUrl &url, QString *error = nullptr );

    /**
     * Fetches collections from \a url using a blocking network request.
     * An optional \a error parameter will be populated with any network error information.
     * The caller takes ownership of the returned feature collection
     */
    QgsStacCollections *fetchCollections( const QUrl &url, QString *error = nullptr );

    /**
     * Initiates an asynchronous request for a STAC object using the \a url
     * and returns an associated request id.
     * When the request is completed, the finishedStacObjectRequest() signal is fired
     * and the stac object can be accessed with takeStacObject()
     */
    int fetchStacObjectAsync( const QUrl &url );

    /**
     * Initiates an asynchronous request for a feature collection using the \a url
     * and returns an associated request id.
     * When the request is completed, the finishedItemCollectionRequest() signal is fired
     * and the feature collection can be accessed with takeItemCollection()
     */
    int fetchItemCollectionAsync( const QUrl &url );

    /**
     * Initiates an asynchronous request for a Collections collection using the \a url
     * and returns an associated request id.
     * When the request is completed, the finishedCollectionsRequest() signal is fired
     * and the collections can be accessed with takeCollections()
     * \since QGIS 3.42
     */
    int fetchCollectionsAsync( const QUrl &url );

    /**
     * Cancels all pending async requests
     * \since QGIS 3.42
     */
    void cancelPendingAsyncRequests();

    /**
     * Returns the STAC object fetched with the specified \a requestId.
     * It should be used after the finishedStacObjectRequest signal is fired to get the fetched STAC object.
     * Returns NULLPTR if the requestId was not found, request was canceled, request failed or parsing the STAC object failed.
     * The caller takes ownership of the returned object
     * \see fetchStacObjectAsync
     * \see finishedStacObjectRequest
     */
    QgsStacObject *takeStacObject( int requestId );

    /**
     * Returns the feature collection fetched with the specified \a requestId
     * It should be used after the finishedItemCollectionRequest signal is fired to get the fetched STAC item collection.
     * Returns NULLPTR if the requestId was not found, request was canceled, request failed or parsing the STAC object failed.
     * The caller takes ownership of the returned feature collection
     * \see fetchItemCollectionAsync
     * \see finishedItemCollectionRequest
     */
    QgsStacItemCollection *takeItemCollection( int requestId );

    /**
     * Returns the collections collection fetched with the specified \a requestId
     * It should be used after the finishedCollectionsRequest signal is fired to get the fetched STAC collections.
     * Returns NULLPTR if the requestId was not found, request was canceled, request failed or parsing the STAC object failed.
     * The caller takes ownership of the returned collections
     * \see fetchCollectionsAsync
     * \see finishedCollectionsRequest
     * \since QGIS 3.42
     */
    QgsStacCollections *takeCollections( int requestId );

    /**
     * Returns the authentication config id which will be used during the request.
     * \see setAuthCfg()
     */
    QString authCfg() const;

    /**
     * Sets the authentication config id which should be used during the request.
     * \see authCfg()
     */
    void setAuthCfg( const QString &authCfg );

  signals:

    /**
     * This signal is fired when an async request initiated with fetchStacObjectAsync is finished.
     * The parsed STAC object can be retrieved using takeStacObject
     * \param id The requestId attribute of the finished request
     * \param errorMessage Reason the request or parsing of the STAC object may have failed
     * \see fetchStacObjectAsync
     * \see takeStacObject
     */
    void finishedStacObjectRequest( int id, QString errorMessage );

    /**
     * This signal is fired when an async request initiated with fetchItemCollectionAsync is finished.
     * The parsed STAC item collection can be retrieved using takeItemCollection
     * \param id The requestId attribute of the finished request
     * \param errorMessage Reason the request or parsing of the STAC item collection may have failed
     * \see fetchItemCollectionAsync
     * \see takeItemCollection
     */
    void finishedItemCollectionRequest( int id, QString errorMessage );

    /**
     * This signal is fired when an async request initiated with fetchCollectionsAsync is finished.
     * The parsed STAC collections collection can be retrieved using takeCollections
     * \param id The requestId attribute of the finished request
     * \param errorMessage Reason the request or parsing of the STAC collections may have failed
     * \see fetchCollectionsAsync
     * \see takeCollections
     * \since QGIS 3.42
     */
    void finishedCollectionsRequest( int id, QString errorMessage );

  private slots:
    void handleStacObjectReply();
    void handleItemCollectionReply();
    void handleCollectionsReply();

  private:
    QNetworkReply *fetchAsync( const QUrl &url );
    QgsNetworkReplyContent fetchBlocking( const QUrl &url );

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QMap< int, QgsStacObject *> mFetchedStacObjects;
    QMap< int, QgsStacItemCollection *> mFetchedItemCollections;
    QMap< int, QgsStacCollections *> mFetchedCollections;
    QVector<QNetworkReply *> mReplies;
    QString mError;
};

#endif // QGSSTACCONTROLLER_H
