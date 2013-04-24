/** \ingroup MapComposer
 * A model that provides layers as root items. The classification items are
 * children of the layer items.
 */

class QgsLegendModel: QStandardItemModel
{
%TypeHeaderCode
#include <qgslegendmodel.h>
%End

  public:

    enum ItemType
    {
      GroupItem = 0,
      LayerItem,
      ClassificationItem
    };

    QgsLegendModel();
    ~QgsLegendModel();

    /**Sets layer set and groups*/
    void setLayerSetAndGroups( const QStringList& layerIds, const QList<  QPair< QString, QList<QString> > >& groupInfo );
    void setLayerSet( const QStringList& layerIds );
    /**Adds a group
      @param text name of group (defaults to translation of "Group")
      @param position insertion position (toplevel position (or -1 if it should be placed at the end of the legend).
      @returns a pointer to the added group
      */
    QStandardItem *addGroup( QString text = QString::null, int position = -1 );

    /**Tries to automatically update a model entry (e.g. a whole layer or only a single item)*/
    void updateItem( QStandardItem* item );
    /**Updates the whole symbology of a layer*/
    void updateLayer( QStandardItem* layerItem );
    /**Tries to update a single classification item*/
    void updateVectorV2ClassificationItem( QStandardItem* classificationItem, QgsSymbolV2* symbol, QString itemText );
    void updateRasterClassificationItem( QStandardItem* classificationItem );

    bool writeXML( QDomElement& composerLegendElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& legendModelElem, const QDomDocument& doc );

    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    /**Implemented to support drag operations*/
    virtual bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );

    /**For the drag operation*/
    QMimeData* mimeData( const QModelIndexList &indexes ) const;
    QStringList mimeTypes() const;

    /**Implements the drop operation*/
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    void setAutoUpdate( bool autoUpdate );
    bool autoUpdate();

  public slots:
    void removeLayer( const QString& layerId );
    void addLayer( QgsMapLayer* theMapLayer );

  signals:
    void layersChanged();
};
