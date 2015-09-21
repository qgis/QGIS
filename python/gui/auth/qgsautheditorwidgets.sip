class QgsAuthEditorWidgets : QWidget
{
%TypeHeaderCode
#include <qgsautheditorwidgets.h>
%End

  public:
    explicit QgsAuthEditorWidgets( QWidget *parent /TransferThis/ = 0 );

    ~QgsAuthEditorWidgets();

    QTabWidget * tabbedWidget();
};
