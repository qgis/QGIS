/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsludialog.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLUDialog: QDialog
{

%TypeHeaderCode
#include "qgsludialog.h"
%End
  public:
    QgsLUDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    QString lowerValue() const;
%Docstring
 :rtype: str
%End
    void setLowerValue( const QString &val );
    QString upperValue() const;
%Docstring
 :rtype: str
%End
    void setUpperValue( const QString &val );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsludialog.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
