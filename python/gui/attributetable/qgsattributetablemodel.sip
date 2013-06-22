class QgsAttributeTableModel : QAbstractTableModel
{
%TypeHeaderCode
#include <qgsattributetablemodel.h>
%End
  public:
    enum Role
    {
      SortRole,
      FeatureIdRole,
      FieldIndexRole,
    };

  public:
    /**
     * Constructor
     * @param layerCache  A layer cache to use as backend
     * @param parent      The parent QObject (owner)
     */
    QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent = 0 );

    virtual ~QgsAttributeTableModel();

    /**
     * Loads the layer into the model
     * Preferably to be called, before basing any other models on this model
     */
    virtual void loadLayer();

    /**
     * Returns the number of rows
     * @param parent parent index
     */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Returns the number of columns
     * @param parent parent index
     */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Returns header data
     * @param section required section
     * @param orientation horizontal or vertical orientation
     * @param role data role
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    /**
     * Returns data on the given index
     * @param index model index
     * @param role data role
     */
    virtual QVariant data( const QModelIndex &index, int role ) const;

    /**
     * Updates data on given index
     * @param index model index
     * @param value new data value
     * @param role data role
     */
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    /**
     * Returns item flags for the index
     * @param index model index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    /**
     * Reloads the model data between indices
     * @param index1 start index
     * @param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );

    /**
     * Remove rows
     */
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    /**
     * Resets the model
     */
    void resetModel();

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
    QgsVectorLayer* layer() const;

    /**
     * Returns the layer cache this model uses as backend.
     */
    QgsVectorLayerCache* layerCache() const;

    /**
     * Execute an action
     */
    void executeAction( int action, const QModelIndex &idx ) const;

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

  signals:
    /**
     * Model has been changed
     */
    void modelChanged();

    //! @note not available in python bindings
    // void progress( int i, bool &cancel );
    void finished();

  protected slots:
    /**
     * Launched when attribute value has been changed
     * @param fid feature id
     * @param idx attribute index
     * @param value new value
     */
    virtual void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
    /**
     * Launched when a feature has been deleted
     * @param fid feature id
     */
    virtual void featureDeleted( QgsFeatureId fid );
    /**
     * Launched when a feature has been added
     * @param fid feature id
     * @param inOperation guard insertion with beginInsertRows() / endInsertRows()
     */
    virtual void featureAdded( QgsFeatureId fid, bool inOperation = true );

    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

  protected:
    /**
      * Gets mFieldCount, mAttributes and mValueMaps
      */
    virtual void loadAttributes();
};
