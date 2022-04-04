/***************************************************************************
    qgsnetworklogger.h
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNETWORKLOGGER_H
#define QGSNETWORKLOGGER_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QElapsedTimer>
#include "qgsnetworkaccessmanager.h"

class QgsNetworkLoggerNode;
class QgsNetworkLoggerRequestGroup;
class QgsNetworkLoggerRootNode;
class QAction;

/**
 * \ingroup app
 * \class QgsNetworkLogger
 * \brief Logs network requests from a QgsNetworkAccessManager, converting them
 * to a QAbstractItemModel representing the request and response details.
 *
 * \since QGIS 3.14
 */
class QgsNetworkLogger : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNetworkLogger, logging requests from the specified \a manager.
     *
     * \warning QgsNetworkLogger must be created on the main thread, using the main thread's
     * QgsNetworkAccessManager instance.
     */
    QgsNetworkLogger( QgsNetworkAccessManager *manager, QObject *parent );
    ~QgsNetworkLogger() override;

    /**
     * Returns TRUE if the logger is currently logging activity.
     */
    bool isLogging() const;

    // Implementation of virtual functions from QAbstractItemModel

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /**
     * Returns node for given index. Returns root node for invalid index.
     */
    QgsNetworkLoggerNode *index2node( const QModelIndex &index ) const;

    /**
     * Returns a list of actions corresponding to the item at the specified \a index.
     *
     * The actions should be parented to \a parent.
     */
    QList< QAction * > actions( const QModelIndex &index, QObject *parent );

    /**
     * Removes a list of request \a rows from the log.
    */
    void removeRequestRows( const QList< int > &rows );

    /**
     * Returns the root node of the log.
     */
    QgsNetworkLoggerRootNode *rootGroup();

    static constexpr int MAX_LOGGED_REQUESTS = 1000;

  public slots:

    /**
     * Enables or disables logging, depending on the value of \a enabled.
     */
    void enableLogging( bool enabled );

    /**
     * Clears all logged entries.
     */
    void clear();

  private slots:
    void requestAboutToBeCreated( QgsNetworkRequestParameters parameters );
    void requestFinished( QgsNetworkReplyContent content );
    void requestTimedOut( QgsNetworkRequestParameters parameters );
    void downloadProgress( int requestId, qint64 bytesReceived, qint64 bytesTotal );
    void requestEncounteredSslErrors( int requestId, const QList<QSslError> &errors );

  private:

    //! Returns index for a given node
    QModelIndex node2index( QgsNetworkLoggerNode *node ) const;
    QModelIndex indexOfParentLayerTreeNode( QgsNetworkLoggerNode *parentNode ) const;

    QgsNetworkAccessManager *mNam = nullptr;
    bool mIsLogging = false;

    std::unique_ptr< QgsNetworkLoggerRootNode > mRootNode;

    QHash< int, QgsNetworkLoggerRequestGroup * > mRequestGroups;

};

/**
 * \ingroup app
 * \class QgsNetworkLoggerProxyModel
 * \brief A proxy model for filtering QgsNetworkLogger models by url string subsets
 * or request status.
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerProxyModel : public QSortFilterProxyModel
{
  public:

    /**
     * Constructor for QgsNetworkLoggerProxyModel, filtering the specified network \a logger.
     */
    QgsNetworkLoggerProxyModel( QgsNetworkLogger *logger, QObject *parent );

    /**
     * Sets a filter \a string to apply to request URLs.
     */
    void setFilterString( const QString &string );

    /**
     * Sets whether successful requests should be shown.
     */
    void setShowSuccessful( bool show );

    /**
     * Sets whether timed out requests should be shown.
     */
    void setShowTimeouts( bool show );

    /**
     * Sets whether requests served directly from cache are shown
     */
    void setShowCached( bool show );

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:

    QgsNetworkLogger *mLogger = nullptr;

    QString mFilterString;
    bool mShowSuccessful = true;
    bool mShowTimeouts = true;
    bool mShowCached = true;
};

#endif // QGSNETWORKLOGGER_H
