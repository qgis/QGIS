class QgsEllipseSymbolLayerV2Widget: QgsSymbolLayerWidget
{
%TypeHeaderCode
#include <qgsellipsesymbollayerv2widget.h>
%End
  public:
    QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer );
    virtual QgsSymbolLayer* symbolLayer();
};
