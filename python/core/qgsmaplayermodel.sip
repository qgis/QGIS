
/**
 * @brief The QgsMapLayerModel class is a model to display layers in widgets.
 * @see QgsMapLayerProxyModel to sort and/filter the layers
 * @see QgsFieldModel to combine in with a field selector.
 * @note added in 2.3
 */
class QgsMapLayerModel : QAbstractItemModel
{

%TypeHeaderCode
#include "qgsmaplayermodel.h"
%End

  public:

    //! Item data roles
    enum ItemDataRole
    {
      LayerIdRole, /*!< Stores the map layer ID */
      LayerRole, /*!< Stores pointer to the map layer itself */
      IsEmptyRole, //!< True if index corresponds to the empty (not set) value
      IsAdditionalRole, //!< True if index corresponds to an additional (non map layer) item
    };

    /**
     * @brief QgsMapLayerModel creates a model to display layers in widgets.
     */
    explicit QgsMapLayerModel( QObject *parent /TransferThis/ = 0 );
    /**
     * @brief QgsMapLayerModel creates a model to display a specific list of layers in a widget.
     */
    explicit QgsMapLayerModel( const QList<QgsMapLayer*>& layers, QObject *parent /TransferThis/ = 0 );

    /**
     * @brief setItemsCheckable defines if layers should be selectable in the widget
     */
    void setItemsCheckable( bool checkable );
    /**
     * @brief checkAll changes the checkstate for all the layers
     */
    void checkAll( Qt::CheckState checkState );

    /**
     * Sets whether an optional empty layer ("not set") option is present in the model.
     * @see allowEmptyLayer()
     * @note added in QGIS 3.0
     */
    void setAllowEmptyLayer( bool allowEmpty );

    /**
     * Returns true if the model allows the empty layer ("not set") choice.
     * @see setAllowEmptyLayer()
     * @note added in QGIS 3.0
     */
    bool allowEmptyLayer() const;

    /**
     * Sets whether the CRS of layers is also included in the model's display role.
     * @see showCrs()
     * @note added in QGIS 3.0
     */
    void setShowCrs( bool showCrs );

    /**
     * Returns true if the model includes layer's CRS in the display role.
     * @see setShowCrs()
     * @note added in QGIS 3.0
     */
    bool showCrs() const;

    /**
     * Sets a list of additional (non map layer) items to include at the end of the model.
     * These may represent additional layers such as layers which are not included in the map
     * layer registry, or paths to layers which have not yet been loaded into QGIS.
     * @see additionalItems()
     * @note added in QGIS 3.0
     */
    void setAdditionalItems( const QStringList& items );

    /**
     * Return the list of additional (non map layer) items included at the end of the model.
     * @see setAdditionalItems()
     * @note added in QGIS 3.0
     */
    QStringList additionalItems() const;

    /**
     * @brief layersChecked returns the list of layers which are checked (or unchecked)
     */
    QList<QgsMapLayer*> layersChecked( Qt::CheckState checkState = Qt::Checked );
    //! returns if the items can be checked or not
    bool itemsCheckable() const;

    /**
     * @brief indexFromLayer returns the model index for a given layer
     */
    QModelIndex indexFromLayer( QgsMapLayer* layer ) const;


  protected slots:
    void removeLayers( const QStringList& layerIds );
    void addLayers( const QList<QgsMapLayer*>& layers );

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /**
     * Returns strings for all roles supported by this model.
     */
    QHash<int, QByteArray> roleNames() const;

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole );
    Qt::ItemFlags flags( const QModelIndex &index ) const;
};
