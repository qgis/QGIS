/** \ingroup core
 * The drawing pipe for raster layers.
 */
class QgsRasterDrawer
{
%TypeHeaderCode
#include <qgsrasterdrawer.h>
%End
  public:
    QgsRasterDrawer( QgsRasterIterator *iterator );

    /** Draws raster data.
     * @param p destination QPainter
     * @param viewPort viewport to render
     * @param theQgsMapToPixel map to pixel convertor
     * @param ctx render context
     */
    void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel, const QgsRenderContext *ctx = nullptr );

  protected:
    /** Draws raster part
     * @param p the painter to draw to
     * @param viewPort view port to draw to
     * @param img image to draw
     * @param topLeftCol Left position relative to left border of viewport
     * @param topLeftRow Top position relative to top border of viewport
     * @param mapToPixel map to device coordinate transformation info
     * @note not available in python bindings
     */
    //void drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow ) const;
};
