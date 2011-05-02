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
    QgsLegendModel();
    ~QgsLegendModel();

    void setLayerSet( const QStringList& layerIds );

    /**Tries to automatically update a model entry (e.g. a whole layer or only a single item)*/
    void updateItem( QStandardItem* item );
    /**Updates the whole symbology of a layer*/
    void updateLayer( QStandardItem* layerItem );
    /**Tries to update a single classification item*/
    void updateVectorClassificationItem( QStandardItem* classificationItem, QgsSymbol* symbol, QString itemText );
    void updateRasterClassificationItem( QStandardItem* classificationItem );

    bool writeXML( QDomElement& composerLegendElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& legendModelElem, const QDomDocument& doc );

  public slots:
    void removeLayer( const QString& layerId );
    void addLayer( QgsMapLayer* theMapLayer );

  signals:
    void layersChanged();
};
