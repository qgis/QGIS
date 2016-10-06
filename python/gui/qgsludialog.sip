class QgsLUDialog: QDialog
{
%TypeHeaderCode
#include <qgsludialog.h>
%End

  public:
    QgsLUDialog( QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsLUDialog();
    QString lowerValue() const;
    void setLowerValue( const QString& val );
    QString upperValue() const;
    void setUpperValue( const QString& val );
};
