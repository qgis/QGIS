class QgsPointDisplacementRendererWidget: QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgspointdisplacementrendererwidget.h>
%End
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;
    QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsPointDisplacementRendererWidget();

    QgsFeatureRendererV2* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );
};
