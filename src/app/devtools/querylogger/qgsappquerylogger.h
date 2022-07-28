/***************************************************************************
    qgsappquerylogger.h
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPQUERYLOGGER_H
#define QGSAPPQUERYLOGGER_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QElapsedTimer>
#include "qgsdbquerylog.h"
#include <memory>

class QgsDevToolsModelNode;
class QgsDevToolsModelGroup;
class QgsDatabaseQueryLoggerRootNode;
class QgsDatabaseQueryLoggerQueryGroup;
class QAction;

/**
 * \ingroup app
 * \class QgsAppQueryLogger
 * \brief Logs sql queries, converting them
 * to a QAbstractItemModel representing the request and response details.
 */
class QgsAppQueryLogger : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAppQueryLogger, logging requests from the specified \a manager.
     *
     * \warning QgsAppQueryLogger must be created on the main thread.
     */
    QgsAppQueryLogger( QObject *parent );
    ~QgsAppQueryLogger() override;

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
    QgsDevToolsModelNode *index2node( const QModelIndex &index ) const;

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
    QgsDatabaseQueryLoggerRootNode *rootGroup();

    static constexpr int MAX_LOGGED_REQUESTS = 1000;

  public slots:

    /**
     * Clears all logged entries.
     */
    void clear();

  private slots:
    void queryLogged( const QgsDatabaseQueryLogEntry &query );
    void queryFinished( const QgsDatabaseQueryLogEntry &query );

  private:

    //! Returns index for a given node
    QModelIndex node2index( QgsDevToolsModelNode *node ) const;
    QModelIndex indexOfParentLayerTreeNode( QgsDevToolsModelNode *parentNode ) const;

    std::unique_ptr< QgsDatabaseQueryLoggerRootNode > mRootNode;

    QHash< int, QgsDatabaseQueryLoggerQueryGroup * > mQueryGroups;

};

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerProxyModel
 * \brief A proxy model for filtering QgsNetworkLogger models by url string subsets
 * or request status.
 */
class QgsDatabaseQueryLoggerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDatabaseQueryLoggerProxyModel, filtering the specified network \a logger.
     */
    QgsDatabaseQueryLoggerProxyModel( QgsAppQueryLogger *logger, QObject *parent );

    /**
     * Sets a filter \a string to apply to request queries.
     */
    void setFilterString( const QString &string );

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:

    QgsAppQueryLogger *mLogger = nullptr;

    QString mFilterString;
};

#endif // QGSAPPQUERYLOGGER_H
