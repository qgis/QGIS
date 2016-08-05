class QgsPointDisplacementRendererWidget: QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgspointdisplacementrendererwidget.h>
%End
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRendererV2* renderer ) /Factory/;
    QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRendererV2* renderer );
    ~QgsPointDisplacementRendererWidget();

    QgsFeatureRendererV2* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );
};
