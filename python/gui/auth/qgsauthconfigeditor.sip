class QgsAuthConfigEditor : QWidget
{
%TypeHeaderCode
#include <qgsauthconfigeditor.h>
%End

  public:
    explicit QgsAuthConfigEditor( QWidget *parent /TransferThis/ = 0 );
    ~QgsAuthConfigEditor();

    void toggleTitleVisibility( bool visible );

};
