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

  private:
    /**Adds classification items of vector layers
     @return 0 in case of success*/
    int addVectorLayerItems( QStandardItem* layerItem, QgsMapLayer* vlayer );

    /**Adds item of raster layer
     @return 0 in case of success*/
    int addRasterLayerItem( QStandardItem* layerItem, QgsMapLayer* rlayer );

    /**Insert a symbol into QgsLegendModel symbol storage*/
    void insertSymbol( QgsSymbol* s );
    /**Removes and deletes a symbol*/
    void removeSymbol( QgsSymbol* s );
    /**Removes and deletes all stored symbols*/
    void removeAllSymbols();

    /**Creates a model item for a vector symbol. The calling function takes ownership*/
    QStandardItem* itemFromSymbol( QgsSymbol* s );
};