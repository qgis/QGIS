/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbusyindicatordialog.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsBusyIndicatorDialog : QDialog
{
%Docstring
 A simple dialog to show an indeterminate busy progress indicator.
%End

%TypeHeaderCode
#include "qgsbusyindicatordialog.h"
%End
  public:

    QgsBusyIndicatorDialog( const QString &message = QString(), QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
 Constructor
 Modal busy indicator dialog with no buttons.
 \param message Text to show above busy progress indicator.
 \param parent parent object (owner)
 \param fl widget flags
%End

    QString message() const;
%Docstring
 :rtype: str
%End
    void setMessage( const QString &message );

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbusyindicatordialog.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
