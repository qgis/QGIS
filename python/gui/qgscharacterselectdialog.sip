class QgsCharacterSelectorDialog : QDialog
{
%TypeHeaderCode
#include <qgscharacterselectdialog.h>
%End
  public:
    QgsCharacterSelectorDialog( QWidget* parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsCharacterSelectorDialog();

  public slots:
    const QChar& selectCharacter( bool* gotChar, const QFont& font, const QString& style );
};
