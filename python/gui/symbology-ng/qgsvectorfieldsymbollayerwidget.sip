class QgsVectorFieldSymbolLayerWidget: QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgsvectorfieldsymbollayerwidget.h>
%End
  public:
    QgsVectorFieldSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0 );
    ~QgsVectorFieldSymbolLayerWidget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();
};
