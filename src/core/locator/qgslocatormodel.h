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
 *
 * Note that this class should generally be used with a QgsLocatorProxyModel
 * in order to ensure correct sorting of results by priority and match level.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLocatorModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    static const int NoGroup = 9999;

    //! Custom model roles
    enum Role
    {
      ResultDataRole = Qt::UserRole + 1, //!< QgsLocatorResult data
      ResultTypeRole, //!< Result type
      ResultFilterPriorityRole, //!< Result priority, used by QgsLocatorProxyModel for sorting roles.
      ResultScoreRole, //!< Result match score, used by QgsLocatorProxyModel for sorting roles.
      ResultFilterNameRole, //!< Associated filter name which created the result
      ResultFilterGroupSortingRole, //!< Group results within the same filter results
      ResultActionsRole, //!< The actions to be shown for the given result in a context menu
    };

    /**
     * Constructor for QgsLocatorModel.
     */
    QgsLocatorModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

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
    QHash<int, QByteArray> roleNames() const override;

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
      QString groupTitle = QString();
      int groupSorting = 0;
    };

    QList<Entry> mResults;
    QSet<QString> mFoundResultsFromFilterNames;
    QMap<QgsLocatorFilter *, QStringList> mFoundResultsFilterGroups;
    bool mDeferredClear = false;
    QTimer mDeferredClearTimer;
};

/**
 * \class QgsLocatorAutomaticModel
 * \ingroup core
 * A QgsLocatorModel which has is associated directly with a
 * QgsLocator, and is automatically populated with results
 * from locator searches.
 *
 * Use this QgsLocatorModel subclass when you want the connections
 * between a QgsLocator and the model to be automatically created
 * for you. If more flexibility in model behavior is required,
 * use the base QgsLocatorModel class instead and setup the
 * connections manually.
 *
 * Note that this class should generally be used with a QgsLocatorProxyModel
 * in order to ensure correct sorting of results by priority and match level.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLocatorAutomaticModel : public QgsLocatorModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLocatorAutomaticModel, linked with the specified \a locator.
     *
     * The \a locator is used as the model's parent.
    */
    explicit QgsLocatorAutomaticModel( QgsLocator *locator SIP_TRANSFERTHIS );

    /**
     * Returns a pointer to the locator utilized by this model.
     */
    QgsLocator *locator();

    /**
     * Enqueues a search for a specified \a string within the model.
     *
     * Note that the search may not begin immediately if an existing search request
     * is still running. In this case the existing search must be completely
     * terminated before the new search can begin. The model handles this
     * situation automatically, and will trigger a search for the new
     * search string as soon as possible.
     */
    void search( const QString &string );

    /**
     * Returns a new locator context for searches. The default implementation
     * returns a default constructed QgsLocatorContext. Subclasses can override
     * this method to implement custom context creation logic.
     */
    virtual QgsLocatorContext createContext();

  private slots:

    void searchFinished();

  private:

    QgsLocator *mLocator = nullptr;

    QString mNextRequestedString;
    bool mHasQueuedRequest = false;
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

    /**
     * Constructor for QgsLocatorProxyModel, with the specified \a parent object.
     */
    explicit QgsLocatorProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
};

#endif // QGSLOCATORMODEL_H


