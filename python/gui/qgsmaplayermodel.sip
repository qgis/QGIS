
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
    static const int LayerIdRole;

    /**
     * @brief QgsMapLayerModel creates a model to display layers in widgets.
     */
    explicit QgsMapLayerModel( QObject *parent /TransferThis/ = 0 );
    /**
     * @brief QgsMapLayerModel creates a model to display a specific list of layers in a widget.
     */
    explicit QgsMapLayerModel( QList<QgsMapLayer*> layers, QObject *parent /TransferThis/ = 0 );

    /**
     * @brief setItemsCheckable defines if layers should be selectable in the widget
     */
    void setItemsCheckable( bool checkable );
    /**
     * @brief checkAll changes the checkstate for all the layers
     */
    void checkAll( Qt::CheckState checkState );
    /**
     * @brief layersChecked returns the list of layers which are checked (or unchecked)
     */
    QList<QgsMapLayer*> layersChecked( Qt::CheckState checkState = Qt::Checked );
    //! returns if the items can be checked or not
    bool itemsCheckable();

    /**
     * @brief indexFromLayer returns the model index for a given layer
     */
    QModelIndex indexFromLayer( QgsMapLayer* layer );


  protected slots:
    void removeLayers( const QStringList layerIds );
    void addLayers( QList<QgsMapLayer*> layers );

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role );
    Qt::ItemFlags flags( const QModelIndex &index ) const;
};


