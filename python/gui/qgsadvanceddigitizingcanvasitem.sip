/**
 * @brief The QgsAdvancedDigitizingCanvasItem class draws the graphical elements of the CAD tools (@see QgsAdvancedDigitizingDockWidget) on the map canvas.
 */

class QgsAdvancedDigitizingCanvasItem : QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsadvanceddigitizingcanvasitem.h>
%End
  public:

    explicit QgsAdvancedDigitizingCanvasItem( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget );
    ~QgsAdvancedDigitizingCanvasItem();

    void paint( QPainter *painter );
};
