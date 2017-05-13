/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdialog.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDialog : QDialog
{
%Docstring
 A generic dialog with layout and button box
%End

%TypeHeaderCode
#include "qgsdialog.h"
%End
  public:

    QgsDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags,
               QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close,
               Qt::Orientation orientation = Qt::Horizontal );
%Docstring
 Constructor for QgsDialog.
%End

    QVBoxLayout *layout();
%Docstring
Returns the central layout. Widgets added to it must have this dialog as parent.
 :rtype: QVBoxLayout
%End
    QDialogButtonBox *buttonBox();
%Docstring
Returns the button box.
 :rtype: QDialogButtonBox
%End

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdialog.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
