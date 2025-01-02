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
 * \brief An abstract list model for displaying the results of locator searches.
 *
 * Note that this class should generally be used with a QgsLocatorProxyModel
 * in order to ensure correct sorting of results by priority and match level.
 *
 */
class CORE_EXPORT QgsLocatorModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    static const int NoGroup = -1;
    static const int UnorderedGroup = 0;

    //! Custom model roles
    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsLocatorModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLocatorModel, Role ) : int
    {
      ResultData SIP_MONKEYPATCH_COMPAT_NAME(ResultDataRole) = Qt::UserRole + 1, //!< QgsLocatorResult data
      ResultType SIP_MONKEYPATCH_COMPAT_NAME(ResultTypeRole), //!< Result type
      ResultFilterPriority SIP_MONKEYPATCH_COMPAT_NAME(ResultFilterPriorityRole), //!< Result priority, used by QgsLocatorProxyModel for sorting roles.
      ResultScore SIP_MONKEYPATCH_COMPAT_NAME(ResultScoreRole), //!< Result match score, used by QgsLocatorProxyModel for sorting roles.
      ResultFilterName SIP_MONKEYPATCH_COMPAT_NAME(ResultFilterNameRole), //!< Associated filter name which created the result
      ResultFilterGroupSorting SIP_MONKEYPATCH_COMPAT_NAME(ResultFilterGroupSortingRole), //!< Custom value for sorting \deprecated QGIS 3.40. No longer used.
      ResultFilterGroupTitle, //!< Group title
      ResultFilterGroupScore, //!< Group score
      ResultActions SIP_MONKEYPATCH_COMPAT_NAME(ResultActionsRole), //!< The actions to be shown for the given result in a context menu
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

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

    enum Column
    {
      Name = 0,
      Description
    };


    // sorting is made on these values!
    enum class EntryType : int
    {
      Filter = 0,
      Group = 1,
      Result = 2,
    };

    struct Entry
    {
      EntryType type = EntryType::Result;
      QgsLocatorResult result;
      QString filterTitle;
      QgsLocatorFilter *filter = nullptr;
      QString groupTitle = QString();
      double groupScore = UnorderedGroup;
    };

    QList<Entry> mResults;
    QSet<QString> mFoundResultsFromFilterNames;
    // maps locator with pair of group title and group score
    QMap<QgsLocatorFilter *, QList<std::pair<QString, double>>> mFoundResultsFilterGroups;
    bool mDeferredClear = false;
    QTimer mDeferredClearTimer;
};

/**
 * \class QgsLocatorAutomaticModel
 * \ingroup core
 * \brief A QgsLocatorModel which has is associated directly with a
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
 * \brief A sort proxy model for QgsLocatorModel, which automatically sorts
 * results by precedence.
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


