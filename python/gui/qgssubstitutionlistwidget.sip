/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssubstitutionlistwidget.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSubstitutionListWidget : QgsPanelWidget
{
%Docstring
 A widget which allows users to specify a list of substitutions to apply to a string, with
 options for exporting and importing substitution lists.
.. versionadded:: 3.0
.. seealso:: QgsSubstitutionListDialog
%End

%TypeHeaderCode
#include "qgssubstitutionlistwidget.h"
%End
  public:

    QgsSubstitutionListWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsSubstitutionListWidget.
 \param parent parent widget
%End

    void setSubstitutions( const QgsStringReplacementCollection &substitutions );
%Docstring
 Sets the list of substitutions to show in the widget.
 \param substitutions substitution list
.. seealso:: substitutions()
%End

    QgsStringReplacementCollection substitutions() const;
%Docstring
 Returns the list of substitutions currently defined by the widget.
.. seealso:: setSubstitutions()
 :rtype: QgsStringReplacementCollection
%End

  signals:

    void substitutionsChanged( const QgsStringReplacementCollection &substitutions );
%Docstring
Emitted when the substitution definitions change.
%End

};

class QgsSubstitutionListDialog : QDialog
{
%Docstring
 A dialog which allows users to specify a list of substitutions to apply to a string, with
 options for exporting and importing substitution lists.
.. versionadded:: 3.0
.. seealso:: QgsSubstitutionListWidget
%End

%TypeHeaderCode
#include "qgssubstitutionlistwidget.h"
%End
  public:

    QgsSubstitutionListDialog( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsSubstitutionListDialog.
 \param parent parent widget
%End

    void setSubstitutions( const QgsStringReplacementCollection &substitutions );
%Docstring
 Sets the list of substitutions to show in the dialog.
 \param substitutions substitution list
.. seealso:: substitutions()
%End

    QgsStringReplacementCollection substitutions() const;
%Docstring
 Returns the list of substitutions currently defined by the dialog.
.. seealso:: setSubstitutions()
 :rtype: QgsStringReplacementCollection
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssubstitutionlistwidget.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
