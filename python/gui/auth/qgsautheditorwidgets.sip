class QgsAuthMethodPlugins : QDialog
{
%TypeHeaderCode
#include <qgsautheditorwidgets.h>
%End

  public:
    explicit QgsAuthMethodPlugins( QWidget *parent /TransferThis/ = 0 );

    ~QgsAuthMethodPlugins();
};


class QgsAuthEditorWidgets : QWidget
{
%TypeHeaderCode
#include <qgsautheditorwidgets.h>
%End

  public:
    explicit QgsAuthEditorWidgets( QWidget *parent /TransferThis/ = 0 );

    ~QgsAuthEditorWidgets();
};
