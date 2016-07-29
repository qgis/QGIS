
/** \ingroup gui
 * \class QgsGraduatedHistogramWidget
 * \brief Graphical histogram for displaying distribution of field values and
 * editing range breaks for a QgsGraduatedSymbolRendererV2 renderer.
 *
 * \note Added in version 2.9
 */

class QgsGraduatedHistogramWidget : QWidget
{
%TypeHeaderCode
#include <qgsgraduatedhistogramwidget.h>
%End

  public:

    /** QgsGraduatedHistogramWidget constructor
     * @param parent parent widget
     */
    QgsGraduatedHistogramWidget( QWidget *parent /TransferThis/ = 0 );
    ~QgsGraduatedHistogramWidget();

    /** Sets the QgsGraduatedSymbolRendererV2 renderer associated with the histogram.
     * The histogram will fetch the ranges from the renderer before every refresh.
     * @param renderer associated QgsGraduatedSymbolRendererV2
     */
    void setRenderer( QgsGraduatedSymbolRendererV2* renderer );

  signals:

    /** Emitted when the user modifies the graduated ranges using the histogram widget.
     * @param rangesAdded true if the user has added ranges, false if the user has just
     * modified existing range breaks
     */
    void rangesModified( bool rangesAdded );

  protected:

    virtual void drawHistogram();

};
