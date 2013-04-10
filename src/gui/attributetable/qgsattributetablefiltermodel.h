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

class QgsVectorLayerCache;
class QgsMapCanvas;
class QItemSelectionModel;

class GUI_EXPORT QgsAttributeTableFilterModel: public QSortFilterProxyModel
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
     *
     * Make sure, the master model is already loaded, so the selection will get synchronized.
     *
     * @param parent parent object (owner)
     * @param sourceModel The QgsAttributeTableModel to use as source (mostly referred to as master model)
     * @param canvas  The mapCanvas. Used to identify the currently visible features.
     */
    QgsAttributeTableFilterModel( QgsMapCanvas* canvas, QgsAttributeTableModel* sourceModel, QObject* parent = NULL );

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
     * Set the filter mode the filter will use.
     *
     * @param filterMode Sets the current mode of the filter
     */
    void setFilterMode( FilterMode filterMode );

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

    /**
     * Returns a selection model which is mapped to the sourceModel (tableModel) of this proxy.
     * This selection also contains the features not visible because of the current filter.
     * Views using this filter model may update this selection and subscribe to changes in
     * this selection. This selection will synchronize itself with the selection on the map
     * canvas.
     *
     * @return The master selection
     */
    QItemSelectionModel* masterSelection();

    /**
     * Disables selection synchronisation with the map canvas. Changes to the selection in the master
     * model are propagated to the layer, but no redraw is requested until @link enableSelectionSync() @endlink
     * is called.
     */
    void disableSelectionSync();

    /**
     * Enables selection synchronisation with the map canvas. Changes to the selection in the master
     * are propagated and upon every change, a redraw will be requested. This method will update the
     * selection to account for any cached selection change since @link disableSelectionSync() @endlink
     * was called.
     */
    void enableSelectionSync();

    virtual QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;

    virtual QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const;

    virtual QItemSelection mapSelectionFromMaster( const QItemSelection& sourceSelection ) const;

  protected:
    /**
     * Returns true if the source row will be accepted
     *
     * @param sourceRow row from the source model
     * @param sourceParent parent index in the source model
     */
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

    /**
     * Updates the list of currently visible features on the map canvas.
     * Is called automatically when the filter mode is adjusted or the extents changed.
     */
    void generateListOfVisibleFeatures();

    /**
     * Used by the sorting algorithm. Compares the two model indices. Will also consider the
     * selection state of the feature in case selected features are to be shown on top.
     */
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

    /**
     * Calls invalidateFilter on the underlying QSortFilterProxyModel, but emits the signals
     * filterAboutToBeInvalidated before and the signal filterInvalidated after the changes on the
     * filter happen.
     */
    void announcedInvalidateFilter();



  public slots:
    /**
     * Is called upon every change of the selection on the map canvas.
     * When an update is signalled, the filter is updated and invalidated if needed.
     *
     */
    void selectionChanged();

    void masterSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

    /**
     * Is called upon every change of the visible extents on the map canvas.
     * When a change is signalled, the filter is updated and invalidated if needed.
     *
     */
    void extentsChanged();

  signals:
    /**
     * This signal is emitted, before the filter is invalidated. With the help of this signal,
     * selections of views attached to this can disable synchronisation with the master selection
     * before items currently not visible with the filter get removed from the selection.
     */
    void filterAboutToBeInvalidated();

    /**
     * Is called after the filter has been invalidated and recomputed.
     * See filterAboutToBeInvalidated.
     */
    void filterInvalidated();

  private:
    QgsFeatureIds mFilteredFeatures;
    QgsMapCanvas* mCanvas;
    FilterMode mFilterMode;
    bool mSelectedOnTop;
    QItemSelectionModel* mMasterSelection;
    QgsAttributeTableModel* mTableModel;
    bool mSyncSelection;
};

#endif
