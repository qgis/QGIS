class QgsEllipseSymbolLayerWidget: QgsSymbolLayerWidget
{
%TypeHeaderCode
#include <qgsellipsesymbollayerwidget.h>
%End
  public:
    QgsEllipseSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0 );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer );
    virtual QgsSymbolLayer* symbolLayer();
};
