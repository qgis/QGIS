/** \ingroup gui
 * @brief The QgsUserInputDockWidget class is a dock widget that shall be used to display widgets for user inputs.
 * It can be used by map tools, plugins, etc.
 * Several widgets can be displayed at once, they will be separated by a separator. Widgets will be either layout horizontally or vertically.
 * The dock is automatically hidden if it contains no widget.
 */
class QgsUserInputDockWidget : QDockWidget
{
%TypeHeaderCode
#include "qgsuserinputdockwidget.h"
%End

  public:
    QgsUserInputDockWidget( QWidget* parent /TransferThis/ = 0 );
    ~QgsUserInputDockWidget();

    /** Add a widget to be displayed in the dock.
     * @param widget widget to add. Ownership is not transferred.
     */
    void addUserInputWidget( QWidget* widget );

  protected:
    //! will not display the dock if it contains no widget
    void paintEvent( QPaintEvent *event );

};
