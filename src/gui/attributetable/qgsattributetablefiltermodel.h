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

#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QTimer>

#include "qgsattributetablemodel.h"
#include "qgsfeaturemodel.h"
#include "qgis_gui.h"

class QgsVectorLayerCache;
class QgsMapCanvas;
class QItemSelectionModel;

/**
 * \ingroup gui
 * \class QgsAttributeTableFilterModel
 */
class GUI_EXPORT QgsAttributeTableFilterModel: public QSortFilterProxyModel, public QgsFeatureModel
{
    Q_OBJECT

  public:

    /**
     * The filter mode defines how the rows should be filtered.
     */
    enum FilterMode
    {
      ShowAll,          //!< Show all features
      ShowSelected,     //!< Show only selected features
      ShowVisible,      //!< Show only visible features (depends on the map canvas)
      ShowFilteredList, //!< Show only features whose ids are on the filter list. {\see setFilteredFeatures}
      ShowEdited,       //!< Show only features which have unsaved changes
      ShowInvalid,      //!< Show only features not respecting constraints (since QGIS 3.30)
    };
    Q_ENUM( FilterMode )

    /**
     * The type of a column.
     */
    enum ColumnType
    {
      ColumnTypeField,       //!< This column shows a field
      ColumnTypeActionButton //!< This column shows action buttons
    };
    Q_ENUM( ColumnType )

    // *INDENT-OFF*

    /**
     * The additional roles defined by this filter model.
     * The values of these roles start just after the roles defined by
     * QgsAttributeTableModel so they do not conflict.
     * \note Prior to QGIS 3.36 this was available as QgsAttributeTableFilterModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAttributeTableFilterModel, Role ) : int
    {
      Type SIP_MONKEYPATCH_COMPAT_NAME(TypeRole) = static_cast< int >( QgsAttributeTableModel::CustomRole::User ) //!< The type of a given column
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

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
    QgsFeatureIds filteredFeatures();

    /**
     * Set the filter mode the filter will use.
     *
     * \param filterMode Sets the current mode of the filter
     */
    void setFilterMode( FilterMode filterMode );

    /**
     * Disconnect the connections set for the current filterMode
     */
    void disconnectFilterModeConnections();

    /**
     * Disconnect the connections set for the new \a filterMode
     */
    void connectFilterModeConnections( FilterMode filterMode );

    /**
     * The current filterModel
     */
    FilterMode filterMode() { return mFilterMode; }

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
    QgsFeatureId rowToId( const QModelIndex &row );

    QModelIndex fidToIndex( QgsFeatureId fid ) override;

    QModelIndexList fidToIndexList( QgsFeatureId fid );

    QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;

    inline QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const { return mapFromSource( sourceIndex ); }

    QModelIndex mapToSource( const QModelIndex &proxyIndex ) const override;

    QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;

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

    QVariant data( const QModelIndex &index, int role ) const override;

    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

    /**
     * Gets the index of the first column that contains an action widget.
     * Returns -1 if none is defined.
     */
    int actionColumnIndex() const;

    int columnCount( const QModelIndex &parent ) const override;

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
     * \deprecated since QGIS 3.10.3 - made private as reloadVisible()
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
    FilterMode mFilterMode = FilterMode::ShowAll;
    bool mSelectedOnTop = false;
    QgsAttributeTableModel *mTableModel = nullptr;

    QgsAttributeTableConfig mConfig;
    QVector<int> mColumnMapping;
    QgsExpression mFilterExpression;
    QgsExpressionContext mFilterExpressionContext;

    int mapColumnToSource( int column ) const;
    int mapColumnFromSource( int column ) const;

    QTimer mReloadVisibleTimer;
    QTimer mFilterFeaturesTimer;
    void startTimedReloadVisible();
    void startTimedFilterFeatures();
};

#endif
