class QgsHeatmapRendererWidget : QgsRendererWidget
{
%TypeHeaderCode
#include <qgsheatmaprendererwidget.h>
%End
  public:
    /** Static creation method
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    /** Constructor
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );

    /** @returns the current feature renderer */
    virtual QgsFeatureRenderer* renderer();

    virtual void setContext( const QgsSymbolWidgetContext& context );
};
