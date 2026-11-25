/***************************************************************************
  QgsAttributeTableFilterModel.h - Filter Model for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : (C) 2009 by Vita Cizek
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

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsattributetablemodel.h"
#include "qgsfeaturemodel.h"

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QTimer>

class QgsVectorLayerCache;
class QgsMapCanvas;
class QItemSelectionModel;

/**
 * \ingroup gui
 * \class QgsAttributeTableFilterModel
 * \brief A proxy model for filtering an attribute table model.
 */
class GUI_EXPORT QgsAttributeTableFilterModel : public QSortFilterProxyModel, public QgsFeatureModel
{
    Q_OBJECT

  public:
    /**
     * Make sure, the master model is already loaded, so the selection will get synchronized.
     *
     * \param parent parent object (owner)
     * \param sourceModel The QgsAttributeTableModel to use as source (mostly referred to as master model)
     * \param canvas  The mapCanvas. Used to identify the currently visible features.
     */
    QgsAttributeTableFilterModel( QgsMapCanvas *canvas, QgsAttributeTableModel *sourceModel, QObject *parent SIP_TRANSFERTHIS = nullptr );

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /**
     * Set the attribute table model that backs this model
     *
     * \param sourceModel The model
     *
     */
    void setSourceModel( QgsAttributeTableModel *sourceModel );

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /**
     * Changes the sort order of the features. If set to TRUE, selected features
     * will be sorted on top, regardless of the current sort column
     *
     * \param selectedOnTop Specify, if selected features should be sorted on top
     */
    void setSelectedOnTop( bool selectedOnTop );

    /**
     * Returns if selected features are currently shown on top
     *
     * \returns TRUE if selected are shown on top
     */
    bool selectedOnTop();

    /**
     * Specify a list of features, which the filter will accept.
     * The filter mode will automatically be adjusted to show only these features (ShowFilteredList).
     *
     * \param ids  The list of feature ids which will be accepted by the filter
     */
    virtual void setFilteredFeatures( const QgsFeatureIds &ids );

    /**
     * Gets a list of currently filtered feature ids
     *
     * \returns A list of feature ids
     */
    QgsFeatureIds filteredFeatures() const;

    /**
     * Set the filter mode the filter will use.
     *
     * \param filterMode Sets the current mode of the filter
     */
    void setFilterMode( Qgis::AttributeTableFilterMode filterMode );

    /**
     * Disconnect the connections set for the current filterMode
     */
    void disconnectFilterModeConnections();

    /**
     * Disconnect the connections set for the new \a filterMode
     */
    void connectFilterModeConnections( Qgis::AttributeTableFilterMode filterMode );

    /**
     * The current filterModel
     */
    Qgis::AttributeTableFilterMode filterMode() { return mFilterMode; }

    /**
     * Returns the layer this filter acts on.
     *
     * \returns Abovementioned layer
     */
    inline QgsVectorLayer *layer() const { return masterModel()->layer(); }

    /**
     * Returns the layerCache this filter acts on.
     *
     * \returns The layer cache
     */
    inline QgsVectorLayerCache *layerCache() const { return masterModel()->layerCache(); }

    /**
     * Returns the table model this filter is using
     *
     * \returns the table model in quesion
     */
    inline QgsAttributeTableModel *masterModel() const { return mTableModel; }

    /**
     * Returns the feature id for a given model index.
     *
     * \param row A model index of the row in question
     *
     * \returns The feature id of the feature visible in the provided row
     */
    QgsFeatureId rowToId( const QModelIndex &row ) const;

    QModelIndex fidToIndex( QgsFeatureId fid ) override;

    QModelIndexList fidToIndexList( QgsFeatureId fid );

    QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;

    inline QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const { return mapFromSource( sourceIndex ); }

    /**
     * Sort by the given column using the given order.
     * Prefetches all the data from the layer to speed up sorting.
     *
     * \param column The column which should be sorted
     * \param order  The order ( Qt::AscendingOrder or Qt::DescendingOrder )
     */
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;

    /**
     * Sort by the given expression using the given order.
     * Prefetches all the data from the layer to speed up sorting.
     *
     * \param expression The expression which should be used for sorting
     * \param order      The order ( Qt::AscendingOrder or Qt::DescendingOrder )
     */
    void sort( const QString &expression, Qt::SortOrder order = Qt::AscendingOrder );

    /**
     * The expression which is used to sort the attribute table.
     */
    QString sortExpression() const;

    //! Returns the map canvas
    QgsMapCanvas *mapCanvas() const { return mCanvas; }

    /**
     * Gets the index of the first column that contains an action widget.
     * Returns -1 if none is defined.
     *
     * \deprecated QGIS 4.4
     */
    Q_DECL_DEPRECATED int actionColumnIndex() const SIP_DEPRECATED;

    /**
     * Set the attribute table configuration to control which fields are shown,
     * in which order they are shown as well as if and where an action column
     * is shown.
     * \param config attribute table config
     * \param force default FALSE, if TRUE the attribute table configuration will be reset even if it is not changed.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig &config, bool force SIP_PYARGREMOVE = false );

    /**
     * Set the \a expression and the \a context to be stored in case of the features
     * need to be filtered again (like on filter or on main model data change).
     *
     * \since QGIS 3.10.3
     */
    void setFilterExpression( const QgsExpression &expression, const QgsExpressionContext &context );

    /**
     * Returns the stored filter expression string.
     *
     * \since QGIS 3.28.0
     */
    QString filterExpression() const { return mFilterExpression; };

  signals:

    /**
     * Emitted whenever the sort column is changed
     * \param column The sort column
     * \param order The sort order
     */
    void sortColumnChanged( int column, Qt::SortOrder order );

    /**
     * Emitted when the filtering of the features has been done
     */
    void featuresFiltered();

    /**
     * Emitted when the the visible features on extend are reloaded (the list is created)
     */
    void visibleReloaded();

    /**
     * Emitted when an error occurred while filtering features
     * \since QGIS 3.18
     */
    void filterError( const QString &errorMessage );

  protected:
    /**
     * Returns TRUE if the source row will be accepted
     *
     * \param sourceRow row from the source model
     * \param sourceParent parent index in the source model
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
     * \deprecated QGIS 3.10.3. Made private as reloadVisible().
     */
    Q_DECL_DEPRECATED void extentsChanged();

    /**
     * Updates the filtered features in the filter model. It is called when the data of the
     * main table or the filter expression changed.
     *
     * \since QGIS 3.10.3
     */
    void filterFeatures();

  private slots:
    void selectionChanged();
    void onColumnsChanged();
    void reloadVisible();
    void onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
    void onGeometryChanged();

  private:
    QgsFeatureIds mFilteredFeatures;
    QgsMapCanvas *mCanvas = nullptr;
    Qgis::AttributeTableFilterMode mFilterMode = Qgis::AttributeTableFilterMode::ShowAll;
    bool mSelectedOnTop = false;
    QgsAttributeTableModel *mTableModel = nullptr;

    QgsAttributeTableConfig mConfig;

    QgsExpression mFilterExpression;
    QgsExpressionContext mFilterExpressionContext;

    QTimer mReloadVisibleTimer;
    QTimer mFilterFeaturesTimer;
    void startTimedReloadVisible();
    void startTimedFilterFeatures();
};

#endif
