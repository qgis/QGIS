/***************************************************************************
  QgsAttributeTableModel.h - Models for attribute table
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

#ifndef QGSATTRIBUTETABLEMODEL_H
#define QGSATTRIBUTETABLEMODEL_H

#include <QAbstractTableModel>
#include "qgis_sip.h"
#include <QModelIndex>
#include <QObject>
#include <QHash>
#include <QQueue>
#include <QMap>

#include "qgsconditionalstyle.h"
#include "qgsattributeeditorcontext.h"
#include "qgsmaplayeractionregistry.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsEditorWidgetFactory;
class QgsFieldFormatter;
class QgsVectorLayerCache;

/**
 * \ingroup gui
 * \brief A model backed by a QgsVectorLayerCache which is able to provide
 * feature/attribute information to a QAbstractItemView.
 *
 * \brief
 * Is able to generate editor widgets for its QModelIndexes as well.
 * Is mostly referred to as "master model" within this doc and the source.
 *
 * \see <a href="http://doc.qt.digia.com/qt/model-view-programming.html">Qt Model View Programming</a>
 */
class GUI_EXPORT QgsAttributeTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsAttributeTableModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAttributeTableModel, Role ) : int
    {
      FeatureId SIP_MONKEYPATCH_COMPAT_NAME( FeatureIdRole ) = Qt::UserRole, //!< Get the feature id of the feature in this row
      FieldIndex SIP_MONKEYPATCH_COMPAT_NAME( FieldIndexRole ),              //!< Get the field index of this column
      User SIP_MONKEYPATCH_COMPAT_NAME( UserRole ),                          //!< Start further roles starting from this role
      // Insert new values here, SortRole needs to be the last one
      Sort SIP_MONKEYPATCH_COMPAT_NAME( SortRole ), //!< Role used for sorting start here
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

  public:
    /**
     * Constructor
     * \param layerCache  A layer cache to use as backend
     * \param parent      The parent QObject (owner)
     */
    QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent = nullptr );

    /**
     * Returns the number of rows
     * \param parent parent index
     */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Returns the number of columns
     * \param parent parent index
     */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Returns header data
     * \param section required section
     * \param orientation horizontal or vertical orientation
     * \param role data role
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /**
     * Returns data on the given index
     * \param index model index
     * \param role data role
     */
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Updates data on given index
     * \param index model index
     * \param value new data value
     * \param role data role
     */
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Returns item flags for the index
     * \param index model index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Reloads the model data between indices
     * \param index1 start index
     * \param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );

    /**
     * Remove rows
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
     * \param id feature id
     */
    int idToRow( QgsFeatureId id ) const;

    QModelIndex idToIndex( QgsFeatureId id ) const;

    QModelIndexList idToIndexList( QgsFeatureId id ) const;

    /**
     * Gets field index from column
     */
    int fieldIdx( int col ) const;

    /**
     * Gets column from field index
     */
    int fieldCol( int idx ) const;

    /**
     * Maps row to feature id
     * \param row row number
     */
    QgsFeatureId rowToId( int row ) const;

    /**
     * Swaps two rows
     * \param a first row
     * \param b second row
     */
    void swapRows( QgsFeatureId a, QgsFeatureId b );

    /**
     * Returns the layer this model uses as backend. Retrieved from the layer cache.
     */
    inline QgsVectorLayer *layer() const { return mLayer; }

    /**
     * Returns the layer cache this model uses as backend.
     */
    inline QgsVectorLayerCache *layerCache() const { return mLayerCache; }

    /**
     * Execute an action
     */
    void executeAction( QUuid action, const QModelIndex &idx ) const;

    /**
     * Execute a QgsMapLayerAction
     */
    void executeMapLayerAction( QgsMapLayerAction *action, const QModelIndex &idx, const QgsMapLayerActionContext &context = QgsMapLayerActionContext() ) const;

    /**
     * Returns the feature attributes at given model index
     * \returns feature attributes at given model index
     */
    QgsFeature feature( const QModelIndex &idx ) const;

    /**
     * Caches the entire data for one column. This should be called prior to sorting,
     * so the data does not have to be fetched for every single comparison.
     * Specify -1 as column to invalidate the cache
     *
     * \param column The column index of the field to catch
     */
    void prefetchColumnData( int column );

    /**
     * Prefetches the entire data for an \a expression. Based on this cached information
     * the sorting can later be done in a performant way. A \a cacheIndex can be specified
     * if multiple caches should be filled. In this case, the caches will be available
     * as ``QgsAttributeTableModel::SortRole + cacheIndex``.
     */
    void prefetchSortData( const QString &expression, unsigned long cacheIndex = 0 );

    /**
     * The expression which was used to fill the sorting cache at index \a cacheIndex.
     *
     *  \see prefetchSortData
     */
    QString sortCacheExpression( unsigned long cacheIndex = 0 ) const;

    /**
     * Set a request that will be used to fill this attribute table model.
     * In contrast to a filter, the request will constrain the data shown without the possibility
     * to dynamically adjust it.
     *
     * \param request The request to use to fill this table model.
     */
    void setRequest( const QgsFeatureRequest &request );

    // TODO QGIS 4: return copy instead of reference

    /**
     * Gets the the feature request
     */
    const QgsFeatureRequest &request() const;

    /**
     * Sets the context in which this table is shown.
     * Will be forwarded to any editor widget created when editing data on this model.
     *
     * \param context The context
     */
    void setEditorContext( const QgsAttributeEditorContext &context ) { mEditorContext = context; }

    /**
     * Returns the context in which this table is shown.
     * Will be forwarded to any editor widget created when editing data on this model.
     *
     * \returns The context
     */
    const QgsAttributeEditorContext &editorContext() const { return mEditorContext; }

    /**
     * Empty extra columns to announce from this model.
     * Any extra columns need to be implemented by proxy models in front of this model.
     */
    int extraColumns() const;

    /**
     * Empty extra columns to announce from this model.
     * Any extra columns need to be implemented by proxy models in front of this model.
     */
    void setExtraColumns( int extraColumns );

    /**
     * Returns whether the attribute table will add a visual feedback to cells when an attribute
     * constraint is not met.
     * \since QGIS 3.30
     */
    bool showValidityState() const { return mShowValidityState; }

    /**
     * Sets whether the attribute table will add a visual feedback to cells when an attribute constraint
     * is not met.
     * \since QGIS 3.30
     */
    void setShowValidityState( bool show ) { mShowValidityState = show; }

  public slots:

    /**
     * Loads the layer into the model
     * Preferably to be called, before using this model as source for any other proxy model
     */
    virtual void loadLayer();

    /**
     * Handles updating the model when the conditional style for a field changes.
     * \param fieldName name of field whose conditional style has changed
     */
    void fieldConditionalStyleChanged( const QString &fieldName );

  signals:

    /**
     * Emitted when the model has been changed.
     */
    void modelChanged();

    //! \note not available in Python bindings
    void progress( int i, bool &cancel ) SIP_SKIP;

    /**
     * Emitted when the model has completely loaded all features.
     */
    void finished();

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

    /**
     * Launched when attribute value has been changed
     * \param fid feature id
     * \param idx attribute index
     * \param value new value
     */
    virtual void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );

    /**
     * Launched when features have been deleted
     * \param fids feature ids
     */
    virtual void featuresDeleted( const QgsFeatureIds &fids );

    /**
     * Launched when a feature has been added
     * \param fid feature id
     */
    virtual void featureAdded( QgsFeatureId fid );

    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

    virtual void fieldFormatterRemoved( QgsFieldFormatter *fieldFormatter );

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsVectorLayerCache *mLayerCache = nullptr;
    int mFieldCount = 0;

    mutable QgsFeature mFeat;

    QgsFields mFields;
    QgsAttributeList mAttributes;

    struct WidgetData
    {
        QgsFieldFormatter *fieldFormatter = nullptr;
        QVariant cache;
        QVariantMap config;
        bool loaded = false;
    };
    mutable QVector<WidgetData> mWidgetDatas;

    QHash<QgsFeatureId, int> mIdRowMap;
    QHash<int, QgsFeatureId> mRowIdMap;
    mutable QHash<QgsFeatureId, QList<QgsConditionalStyle>> mRowStylesMap;
    mutable QHash<QgsFeatureId, QHash<int, QgsConditionalStyle>> mConstraintStylesMap;

    mutable QgsExpressionContext mExpressionContext;

    /**
     * Returns widget information for \a column index
     */
    const WidgetData &getWidgetData( int column ) const;

    /**
      * Gets mFieldCount, mAttributes
      */
    void loadAttributes();

    /**
     * Load feature fid into local cache (mFeat)
     *
     * \param  fid     feature id
     *
     * \returns feature exists
     */
    virtual bool loadFeatureAtId( QgsFeatureId fid ) const;

    /**
     * Load feature fid into local cache (mFeat) ensuring that the field with
     * index \a fieldIdx is also fetched even if the cached attributes did not
     * contain the field (e.g. because it was hidden in the attribute table).
     *
     * \param  fid      feature id
     * \param  fieldIdx field index
     *
     * \returns feature exists
     */
    virtual bool loadFeatureAtId( QgsFeatureId fid, int fieldIdx ) const;

    QgsFeatureRequest mFeatureRequest;

    struct SortCache
    {
        //! If it is set, a simple field is used for sorting, if it's -1 it's the mSortCacheExpression
        int sortFieldIndex;
        //! The currently cached column
        QgsExpression sortCacheExpression;
        QgsAttributeList sortCacheAttributes;
        //! Allows caching of one value per column (used for sorting)
        QHash<QgsFeatureId, QVariant> sortCache;
    };

    std::vector<SortCache> mSortCaches;

    QgsAttributeEditorContext mEditorContext;

    int mExtraColumns = 0;

    //! Flag for massive changes operations, set by edit command or rollback
    bool mBulkEditCommandRunning = false;

    //! TRUE if model is in the midst of a reset operation
    bool mResettingModel = false;

    //! Sets the flag for massive changes operations
    void bulkEditCommandStarted();

    //! Clears the flag for massive changes operations, updates/rebuilds the layer cache and tells the view to update
    void bulkEditCommandEnded();

    //! Changed attribute values within a bulk edit command
    QMap<QPair<QgsFeatureId, int>, QVariant> mAttributeValueChanges;

    //! Inserted feature IDs within a bulk edit command
    QList<QgsFeatureId> mInsertedRowsChanges;

    //! TRUE if triggered by afterRollback()
    bool mIsCleaningUpAfterRollback = false;

    bool mShowValidityState = false;

    friend class TestQgsAttributeTable;
};


#endif
