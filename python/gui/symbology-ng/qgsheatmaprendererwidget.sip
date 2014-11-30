class QgsHeatmapRendererWidget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgsheatmaprendererwidget.h>
%End
  public:
    /** static creation method
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    /** Constructor
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    /** @returns the current feature renderer */
    virtual QgsFeatureRendererV2* renderer();
};
