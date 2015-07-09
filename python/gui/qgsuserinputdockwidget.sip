
class QgsUserInputDockWidget : QDockWidget
{
%TypeHeaderCode
#include "qgsuserinputdockwidget.h"
%End

  public:
    QgsUserInputDockWidget( QWidget* parent /TransferThis/ = 0 );
    ~QgsUserInputDockWidget();

    //! add a widget to be displayed in the dock
    void addUserInputWidget( QWidget* widget );

  protected:
    //! will not display the dock if it contains no widget
    void paintEvent( QPaintEvent *event );

};
