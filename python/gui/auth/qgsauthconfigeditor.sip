class QgsAuthConfigEditor : QWidget
{
%TypeHeaderCode
#include <qgsauthconfigeditor.h>
%End

  public:
    explicit QgsAuthConfigEditor( QWidget *parent /TransferThis/ = 0, bool showUtilities = true, bool relayMessages = true );
    ~QgsAuthConfigEditor();

    void toggleTitleVisibility( bool visible );

  public slots:

    void setShowUtilitiesButton( bool show = true );

    void setRelayMessages( bool relay = true );
};
