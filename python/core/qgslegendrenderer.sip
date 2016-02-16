
/**
 * @brief The QgsLegendRenderer class handles automatic layout and rendering of legend.
 * The content is given by QgsLayerTreeModel instance. Various layout properties can be configured
 * within QgsLegendRenderer.
 *
 * All spacing and sizes are in millimeters.
 *
 * @note added in 2.6
 */
class QgsLegendRenderer
{
%TypeHeaderCode
#include <qgslegendrenderer.h>
%End

  public:
    /** Construct legend renderer. The ownership of legend model does not change */
    QgsLegendRenderer( QgsLayerTreeModel* legendModel, const QgsLegendSettings& settings );

    /** Run the layout algorithm and determine the size required for legend */
    QSizeF minimumSize();

    /** Set the preferred resulting legend size. */
    void setLegendSize( QSizeF s );

    /** Find out preferred legend size set by the client. If null, the legend will be drawn with the minimum size */
    QSizeF legendSize() const;

    /** Draw the legend with given painter. It will occupy the area reported in legendSize().
     *  Painter should be scaled beforehand so that units correspond to millimeters.
     */
    void drawLegend( QPainter* painter );


    static void setNodeLegendStyle( QgsLayerTreeNode* node, QgsComposerLegendStyle::Style style );
    static QgsComposerLegendStyle::Style nodeLegendStyle( QgsLayerTreeNode* node, QgsLayerTreeModel* model );

};
