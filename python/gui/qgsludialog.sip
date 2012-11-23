class QgsLUDialog: QDialog
{
%TypeHeaderCode
#include <qgsludialog.h>
%End

  public:
    QgsLUDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsLUDialog();
    QString lowerValue() const;
    void setLowerValue( QString val );
    QString upperValue() const;
    void setUpperValue( QString val );
};
