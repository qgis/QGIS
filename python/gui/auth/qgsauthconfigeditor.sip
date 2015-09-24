class QgsAuthConfigEditor : QWidget
{
%TypeHeaderCode
#include <qgsauthconfigeditor.h>
%End

  public:
    explicit QgsAuthConfigEditor( QWidget *parent /TransferThis/ = 0, bool showUtilities = true, QgsMessageBar *msgbar /TransferThis/ = 0 );
    ~QgsAuthConfigEditor();

    void toggleTitleVisibility( bool visible );

  public slots:

    void showUtilitiesButton( bool show = true );

    void setMessageBar( QgsMessageBar *msgbar /TransferThis/ = 0 );
};
