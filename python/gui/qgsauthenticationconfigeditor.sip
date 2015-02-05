class QgsAuthConfigEditor : QWidget
{
%TypeHeaderCode
#include <qgsauthenticationconfigeditor.h>
%End

  public:
    explicit QgsAuthConfigEditor( QWidget *parent = 0 );
    ~QgsAuthConfigEditor();

    void toggleTitleVisibility( bool visible );

};
