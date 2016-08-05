class QgsVectorFieldSymbolLayerWidget: QgsSymbolLayerWidget
{
%TypeHeaderCode
#include <qgsvectorfieldsymbollayerwidget.h>
%End
  public:
    QgsVectorFieldSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0 );
    ~QgsVectorFieldSymbolLayerWidget();

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer );
    virtual QgsSymbolLayer* symbolLayer();
};
