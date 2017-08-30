/***************************************************************************
                         qgslocatormodel.h
                         ------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCATORMODEL_H
#define QGSLOCATORMODEL_H

#include "qgis_core.h"
#include "qgslocatorfilter.h"
#include <QAbstractListModel>
#include <QTimer>
#include <QSet>
#include <QSortFilterProxyModel>

class QgsLocator;
class QgsLocatorModel;
class QgsLocatorProxyModel;

/**
 * \class QgsLocatorModel
 * \ingroup core
 * An abstract list model for displaying the results of locator searches.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLocatorModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum Role
    {
      ResultDataRole = Qt::UserRole + 1, //!< QgsLocatorResult data
      ResultTypeRole, //!< Result type
      ResultFilterPriorityRole, //!< Result priority, used by QgsLocatorProxyModel for sorting roles.
      ResultScoreRole, //!< Result match score, used by QgsLocatorProxyModel for sorting roles.
      ResultFilterNameRole, //!< Associated filter name which created the result
    };

    /**
     * Constructor for QgsLocatorModel.
     */
    QgsLocatorModel( QObject *parent = nullptr );

    /**
     * Resets the model and clears all existing results.
     * \see deferredClear()
     */
    void clear();

    /**
     * Resets the model and clears all existing results after a short delay, or whenever the next result is added to the model
     * (whichever occurs first). Using deferredClear() instead of clear() can avoid the visually distracting frequent clears
     * which may occur if the model is being updated quickly multiple times as a result of users typing in a search query.
     * \see deferredClear()
     */
    void deferredClear();

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  public slots:

    /**
     * Adds a new \a result to the model.
     */
    void addResult( const QgsLocatorResult &result );

  private:

    enum ColumnCount
    {
      Name = 0,
      Description
    };

    struct Entry
    {
      QgsLocatorResult result;
      QString filterTitle;
      QgsLocatorFilter *filter = nullptr;
    };

    QList<Entry> mResults;
    QSet<QString> mFoundResultsFromFilterNames;
    bool mDeferredClear = false;
    QTimer mDeferredClearTimer;
};

/**
 * \class QgsLocatorProxyModel
 * \ingroup core
 * A sort proxy model for QgsLocatorModel, which automatically sorts
 * results by precedence.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLocatorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    explicit QgsLocatorProxyModel( QObject *parent = nullptr );
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
};

#endif // QGSLOCATORMODEL_H


