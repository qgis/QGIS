class QgsLUDialog: QDialog
{
%TypeHeaderCode
#include <qgsludialog.h>
%End

  public:
    QgsLUDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsLUDialog();
    QString lowerValue() const;
    void setLowerValue( QString val );
    QString upperValue() const;
    void setUpperValue( QString val );
};
