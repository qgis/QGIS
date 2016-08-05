class QgsArrowSymbolLayerWidget: QgsSymbolLayerWidget
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
    static QgsSymbolLayerWidget* create( const QgsVectorLayer* layer ) /Factory/;

    /** Set the symbol layer */
    virtual void setSymbolLayer( QgsSymbolLayer* layer );
    /** Get the current symbol layer */
    virtual QgsSymbolLayer* symbolLayer();
};
