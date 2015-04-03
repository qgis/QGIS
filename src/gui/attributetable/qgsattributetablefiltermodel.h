/***************************************************************************
  QgsAttributeTableFilterModel.h - Filter Model for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEFILTERMODEL_H
#define QGSATTRIBUTETABLEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "qgsvectorlayer.h" //QgsFeatureIds
#include "qgsattributetablemodel.h"
#include "qgsfeaturemodel.h"

class QgsVectorLayerCache;
class QgsMapCanvas;
class QItemSelectionModel;

class GUI_EXPORT QgsAttributeTableFilterModel: public QSortFilterProxyModel, public QgsFeatureModel
{
    Q_OBJECT

  public:
    enum FilterMode
    {
      ShowAll,
      ShowSelected,
      ShowVisible,
      ShowFilteredList,
      ShowEdited
    };

    /**
     *
     * Make sure, the master model is already loaded, so the selection will get synchronized.
     *
     * @param parent parent object (owner)
     * @param sourceModel The QgsAttributeTableModel to use as source (mostly referred to as master model)
     * @param canvas  The mapCanvas. Used to identify the currently visible features.
     */
    QgsAttributeTableFilterModel( QgsMapCanvas* canvas, QgsAttributeTableModel* sourceModel, QObject* parent = 0 );

    /**
     * Set the attribute table model that backs this model
     *
     * @param sourceModel The model
     *
     * @note added in 2.0
     */
    void setSourceModel( QgsAttributeTableModel* sourceModel );

    /**
     * Changes the sort order of the features. If set to true, selected features
     * will be sorted on top, regardless of the current sort column
     *
     * @param selectedOnTop Specify, if selected features should be sorted on top
     */
    void setSelectedOnTop( bool selectedOnTop );

    /**
     * Returns if selected features are currently shown on top
     *
     * @return True if selected are shown on top
     */
    bool selectedOnTop();

    /**
     * Specify a list of features, which the filter will accept.
     * The filter mode will automatically be adjusted to show only these features (ShowFilteredList).
     *
     * @param ids  The list of feature ids which will be accepted by the filter
     */
    virtual void setFilteredFeatures( QgsFeatureIds ids );

    /**
     * Get a list of currently filtered feature ids
     *
     * @return A list of feature ids
     */
    QgsFeatureIds filteredFeatures();

    /**
     * Set the filter mode the filter will use.
     *
     * @param filterMode Sets the current mode of the filter
     */
    void setFilterMode( FilterMode filterMode );

    /**
     * The current filterModel
     *
     * @return Mode
     */
    FilterMode filterMode() { return mFilterMode; }

    /**
     * Returns the layer this filter acts on.
     *
     * @return Abovementioned layer
     */
    inline QgsVectorLayer *layer() const { return masterModel()->layer(); }

    /**
     * Returns the layerCache this filter acts on.
     *
     * @return The layer cache
     */
    inline QgsVectorLayerCache *layerCache() const { return masterModel()->layerCache(); }

    /**
     * Returns the table model this filter is using
     *
     * @return the table model in quesion
     */
    inline QgsAttributeTableModel *masterModel() const { return mTableModel; }

    /**
     * Returns the feature id for a given model index.
     *
     * @param row A model index of the row in question
     *
     * @return The feature id of the feature visible in the provided row
     */
    QgsFeatureId rowToId( const QModelIndex& row );

    QModelIndex fidToIndex( QgsFeatureId fid ) override;
    QModelIndexList fidToIndexList( QgsFeatureId fid );

    virtual QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;

    virtual QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const;

    /**
     * Sort by the given column using the given order.
     * Prefetches all the data from the layer to speed up sorting.
     *
     * @param column The column which should be sorted
     * @param order  The order ( Qt::AscendingOrder or Qt::DescendingOrder )
     */
    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;

  protected:
    /**
     * Returns true if the source row will be accepted
     *
     * @param sourceRow row from the source model
     * @param sourceParent parent index in the source model
     */
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Updates the list of currently visible features on the map canvas.
     * Is called automatically when the filter mode is adjusted or the extents changed.
     */
    void generateListOfVisibleFeatures();

    /**
     * Used by the sorting algorithm. Compares the two model indices. Will also consider the
     * selection state of the feature in case selected features are to be shown on top.
     */
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  public slots:
    /**
     * Is called upon every change of the visible extents on the map canvas.
     * When a change is signalled, the filter is updated and invalidated if needed.
     *
     */
    void extentsChanged();

  private slots:
    void selectionChanged();

  private:
    QgsFeatureIds mFilteredFeatures;
    QgsMapCanvas* mCanvas;
    FilterMode mFilterMode;
    bool mSelectedOnTop;
    QgsAttributeTableModel* mTableModel;
};

#endif
