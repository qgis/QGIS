class QgsArrowSymbolLayerWidget: QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgsarrowsymbollayerwidget.h>
%End
  public:
    /** Constructor
     * @param layer the layer where this symbol layer is applied
     * @param parent the parent widget
     */
    QgsArrowSymbolLayerWidget( const QgsVectorLayer* layer, QWidget* parent /TransferThis/ = 0 );

    /** Static creation method
     * @param layer the layer where this symbol layer is applied
     */
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* layer ) /Factory/;

    /** Set the symbol layer */
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    /** Get the current symbol layer */
    virtual QgsSymbolLayerV2* symbolLayer();
};
