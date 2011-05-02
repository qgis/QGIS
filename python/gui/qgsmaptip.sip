
/** \ingroup gui
 * A maptip is a class to display a tip on a map canvas
 * when a mouse is hovered over a feature.
 */
class QgsMapTip
{
%TypeHeaderCode
#include <qgsmaptip.h>
%End

  public:
    /** Default constructor
     */
    QgsMapTip();
    /** Destructor
     */
    virtual ~QgsMapTip();
    /** Show a maptip at a given point on the map canvas
     * @param QgsMapLayer thepLayer - a qgis vector map layer pointer that will
     *        be used to provide the attribute data for the map tip.
     * @param QgsPoint theMapPosition - a reference to the position of the cursor
     *        in map coordinatess.
     * @param QgsPoint thePixelPosition - a reference to the position of the cursor
     *        in pixel coordinates.
     * @param QgsMapCanvas thepMapCanvas - a map canvas on which the tip is drawn
     */
    void showMapTip( QgsMapLayer * thepLayer,
                     QgsPoint & theMapPosition,
                     QPoint & thePixelPosition,
                     QgsMapCanvas *mpMapCanvas );
    /** Clear the current maptip if it exists
     * @param QgsMapCanvas mpMapCanvas - the canvas from which the tip should
     *        be cleared.
     */
    void clear( QgsMapCanvas *mpMapCanvas );

};
