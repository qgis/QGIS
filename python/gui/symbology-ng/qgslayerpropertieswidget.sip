class QgsLayerPropertiesWidget : QWidget
{
%TypeHeaderCode
#include <qgslayerpropertieswidget.h>
%End

  public:
    QgsLayerPropertiesWidget( QgsSymbolLayerV2* layer, const QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:
    void changed();
    void changeLayer( QgsSymbolLayerV2* );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayerV2* layer );
};
