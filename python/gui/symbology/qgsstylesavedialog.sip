/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsstylesavedialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsStyleSaveDialog: QDialog
{
%Docstring
 a dialog for setting properties of a newly saved style.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsstylesavedialog.h"
%End
  public:

    QgsStyleSaveDialog( QWidget *parent /TransferThis/ = 0, QgsStyle::StyleEntity type = QgsStyle::SymbolEntity );
%Docstring
 Constructor for QgsSymbolSaveDialog
 \param parent parent widget
 \param type the QgsStyle entity type being saved
%End

    QString name() const;
%Docstring
returns the text value of the name element
 :rtype: str
%End

    QString tags() const;
%Docstring
returns the text value of the tags element
 :rtype: str
%End

    bool isFavorite() const;
%Docstring
returns whether the favorite element is checked
 :rtype: bool
%End



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsstylesavedialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
