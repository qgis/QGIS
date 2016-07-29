class QgsEllipseSymbolLayerV2Widget: QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgsellipsesymbollayerv2widget.h>
%End
  public:
    QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0 );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();
};
