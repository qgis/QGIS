/***************************************************************************
  QgsAttributeTableModel.h - Models for attribute table
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

#ifndef QGSATTRIBUTETABLEMODEL_H
#define QGSATTRIBUTETABLEMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QHash>
#include <QQueue>
#include <QMap>
#include <QThread>

#include "qgsvectorlayer.h" // QgsAttributeList
#include "qgsvectorlayercache.h"
#include "qgsconditionalstyle.h"
#include "qgsattributeeditorcontext.h"
#include "qgsattributetableloadworker.h"


class QgsMapCanvas;
class QgsMapLayerAction;
class QgsEditorWidgetFactory;


/**
 * A model backed by a {@link QgsVectorLayerCache} which is able to provide
 * feature/attribute information to a QAbstractItemView.
 *
 * @brief
 * Is able to generate editor widgets for its QModelIndexes as well.
 * Is mostly referred to as "master model" within this doc and the source.
 *
 * @see <a href="http://doc.qt.digia.com/qt/model-view-programming.html">Qt Model View Programming</a>
 */
class GUI_EXPORT QgsAttributeTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum Role
    {
      SortRole = Qt::UserRole + 1,
      FeatureIdRole = Qt::UserRole + 2,
      FieldIndexRole = Qt::UserRole + 3
    };

  public:
    /**
     * Constructor
     * @param layerCache  A layer cache to use as backend
     * @param parent      The parent QObject (owner)
     */
    QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent = 0 );

    /**
     * Destructor
     */
    ~QgsAttributeTableModel( );

    /**
     * Returns the number of rows
     * @param parent parent index
     */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Returns the number of columns
     * @param parent parent index
     */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Returns header data
     * @param section required section
     * @param orientation horizontal or vertical orientation
     * @param role data role
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /**
     * Returns data on the given index
     * @param index model index
     * @param role data role
     */
    virtual QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Updates data on given index
     * @param index model index
     * @param value new data value
     * @param role data role
     */
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Returns item flags for the index
     * @param index model index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Reloads the model data between indices
     * @param index1 start index
     * @param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );

    /**
     * Removes count rows starting with the given row under parent parent from the model.
     */
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    /**
     * Resets the model
     *
     * Alias to loadLayer()
     */
    inline void resetModel() { loadLayer(); }

    /**
     * Maps feature id to table row
     * @param id feature id
     */
    int idToRow( QgsFeatureId id ) const;

    QModelIndex idToIndex( QgsFeatureId id ) const;

    QModelIndexList idToIndexList( QgsFeatureId id ) const;

    /**
     * get field index from column
     */
    int fieldIdx( int col ) const;

    /**
     * get column from field index
     */
    int fieldCol( int idx ) const;

    /**
     * Maps row to feature id
     * @param row row number
     */
    QgsFeatureId rowToId( int row ) const;

    /**
     * Swaps two rows
     * @param a first row
     * @param b second row
     */
    void swapRows( QgsFeatureId a, QgsFeatureId b );

    /**
     * Returns the layer this model uses as backend. Retrieved from the layer cache.
     */
    inline QgsVectorLayer* layer() const { return mLayerCache ? mLayerCache->layer() : NULL; }

    /**
     * Returns the layer cache this model uses as backend.
     */
    inline QgsVectorLayerCache* layerCache() const { return mLayerCache; }

    /**
     * Execute an action
     */
    void executeAction( int action, const QModelIndex &idx ) const;

    /**
     * Execute a QgsMapLayerAction
     */
    void executeMapLayerAction( QgsMapLayerAction* action, const QModelIndex &idx ) const;

    /**
     * Return the feature attributes at given model index
     * @return feature attributes at given model index
     */
    QgsFeature feature( const QModelIndex &idx ) const;

    /**
     * Caches the entire data for one column. This should be called prior to sorting,
     * so the data does not have to be fetched for every single comparison.
     * Specify -1 as column to invalidate the cache
     *
     * @param column The column index of the field to catch
     */
    void prefetchColumnData( int column );

    /**
     * Set a request that will be used to fill this attribute table model.
     * In contrast to a filter, the request will constrain the data shown without the possibility
     * to dynamically adjust it.
     *
     * @param request The request to use to fill this table model.
     */
    void setRequest( const QgsFeatureRequest& request );

    /**
     * Get the the feature request
     */
    const QgsFeatureRequest &request() const;

    /**
     * Sets the context in which this table is shown.
     * Will be forwarded to any editor widget created when editing data on this model.
     *
     * @param context The context
     */
    void setEditorContext( const QgsAttributeEditorContext& context ) { mEditorContext = context; }

    /**
     * Returns the context in which this table is shown.
     * Will be forwarded to any editor widget created when editing data on this model.
     *
     * @return The context
     */
    const QgsAttributeEditorContext& editorContext() const { return mEditorContext; }

  public slots:
    /**
     * Loads the layer into the model
     * Preferably to be called, before using this model as source for any other proxy model
     */
    virtual void loadLayer();

    /** Handles updating the model when the conditional style for a field changes.
     * @param fieldName name of field whose conditional style has changed
     * @note added in QGIS 2.12
     */
    void fieldConditionalStyleChanged( const QString& fieldName );

  signals:
    /**
     * Model has been changed
     */
    void modelChanged();

    /**
     * Called while the features are loaded
     * @note not available in python bindings
     */
    void loadProgress( int featuresLoaded, bool &cancel );

    /**
     * @brief Called when the loading of features has been completed
     */
    void loadFinished();

    /**
     * @brief Called when the loading of features starts
     * @param numFeatures number of features to be loaded
     */
    void loadStarted( long numFeatures );

    /**
     * @brief Called when the loading of features is stopped
     */
    void loadStopped( );

  private slots:
    /**
     * Launched whenever the number of fields has changed
     */
    virtual void updatedFields();

    /**
     * Gets called when an edit command ends
     * This will synchronize all fields which have been changed since the last
     * edit command in one single go
     */
    virtual void editCommandEnded();

    /**
     * Called whenever a column is removed;
     */
    virtual void attributeDeleted( int idx );

  protected slots:
    /**
     * Launched when attribute value has been changed
     * @param fid feature id
     * @param idx attribute index
     * @param value new value
     */
    virtual void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
    /**
     * Launched when eatures have been deleted
     * @param fids feature ids
     */
    virtual void featuresDeleted( const QgsFeatureIds& fids );

    /**
     * Launched when a feature has been added
     * @param fid feature id
     */
    virtual void featureAdded( QgsFeatureId fid );


    /**
     * Launched from the load worker when a feature is ready to be added
     * adds a batch of features to the model
     * @param features feature list
     * @param loadedCount number of loaded features
     */
    virtual void featuresReady( QgsFeatureList features , int loadedCount );

    /**
     * Called when the load worker has finished its job
     */
    virtual void loadLayerFinished();

    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

    /*
    private slots:
      //! set mLoadWorker pointer to 0
      virtual void resetLoadWorkerThread();
      //! set mLoadWorkerThread pointer to 0
      virtual void resetLoadWorker();
    */

  protected:
    QgsVectorLayerCache *mLayerCache;
    int mFieldCount;

    mutable QgsFeature mFeat;

    QgsAttributeList mAttributes;
    QVector<QgsEditorWidgetFactory*> mWidgetFactories;
    QVector<QVariant> mAttributeWidgetCaches;
    QVector<QgsEditorWidgetConfig> mWidgetConfigs;

    mutable QgsExpressionContext mExpressionContext;

    /**
      * Gets mFieldCount, mAttributes and mValueMaps
      */
    virtual void loadAttributes();

  private:
    /**
     * Load feature fid into local cache (mFeat)
     *
     * @param  fid     feature id
     *
     * @return feature exists
     */
    virtual bool loadFeatureAtId( QgsFeatureId fid ) const;

    /**
     * @brief loadWorkerStop: stop the load worker and the thread
     */
    void loadWorkerStop();

    QgsFeatureRequest mFeatureRequest;

    /** The currently cached column */
    int mCachedField;
    /** Allows caching of one specific column (used for sorting) */
    QHash<QgsFeatureId, QVariant> mFieldCache;

    /** Indexes */
    QHash<QgsFeatureId, int> mIdRowMap;
    QHash<int, QgsFeatureId> mRowIdMap;
    mutable QHash<int, QList<QgsConditionalStyle> > mRowStylesMap;

    /**
     * Holds the bounds of changed cells while an update operation is running
     * top    = min row
     * left   = min column
     * bottom = max row
     * right  = max column
     */
    QRect mChangedCellBounds;

    QgsAttributeEditorContext mEditorContext;
    QgsAttributeTableLoadWorker* mLoadWorker;
    QThread mLoadWorkerThread;
};


#endif
