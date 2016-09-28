/** \class QgsPointClusterRendererWidget
 * \ingroup gui
 * A widget which allows configuration of the properties for a QgsPointClusterRenderer.
 * \note added in QGIS 3.0
 */

class QgsPointClusterRendererWidget: QgsRendererWidget
{
%TypeHeaderCode
#include <qgspointclusterrendererwidget.h>
%End
  public:

    /** Returns a new QgsPointClusterRendererWidget.
     * @param layer associated vector layer
     * @param style style collection
     * @param renderer source QgsPointClusterRenderer renderer
     * @returns new QgsRendererWidget
     */
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    /** Constructor for QgsPointClusterRendererWidget.
     * @param layer associated vector layer
     * @param style style collection
     * @param renderer source QgsPointClusterRenderer renderer
     */
    QgsPointClusterRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );

    ~QgsPointClusterRendererWidget();

    QgsFeatureRenderer* renderer();
    void setContext( const QgsSymbolWidgetContext& context );
};
