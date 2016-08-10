class QgsPointDisplacementRendererWidget: QgsRendererWidget
{
%TypeHeaderCode
#include <qgspointdisplacementrendererwidget.h>
%End
  public:
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;
    QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsPointDisplacementRendererWidget();

    QgsFeatureRenderer* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );
};
