/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscharacterselectordialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCharacterSelectorDialog : QDialog
{
%Docstring
A dialog for selecting a single character from a single font
%End

%TypeHeaderCode
#include "qgscharacterselectordialog.h"
%End
  public:
    QgsCharacterSelectorDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  public slots:
    const QChar &selectCharacter( bool *gotChar, const QFont &font, const QString &style );

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscharacterselectordialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
