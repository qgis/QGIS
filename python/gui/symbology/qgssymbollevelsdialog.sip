/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollevelsdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSymbolLevelsWidget : QgsPanelWidget
{
%Docstring
 A widget which allows the user to modify the rendering order of symbol layers.
.. seealso:: QgsSymbolLevelsDialog
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssymbollevelsdialog.h"
%End
  public:
    QgsSymbolLevelsWidget( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSymbolLevelsWidget
%End

    bool usingLevels() const;
%Docstring
Returns whether the level ordering is enabled
 :rtype: bool
%End

    void setForceOrderingEnabled( bool enabled );
%Docstring
 Sets whether the level ordering is always forced on and hide the checkbox (used by rule-based renderer)
 \param enabled toggle level ordering
%End

  public slots:
    void apply();
%Docstring
Apply button
%End

  protected:




  private:
    QgsSymbolLevelsWidget();

};

class QgsSymbolLevelsDialog : QDialog
{
%Docstring
 A dialog which allows the user to modify the rendering order of symbol layers.
.. seealso:: QgsSymbolLevelsWidget
%End

%TypeHeaderCode
#include "qgssymbollevelsdialog.h"
%End
  public:

    QgsSymbolLevelsDialog( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSymbolLevelsDialog.
%End

    void setForceOrderingEnabled( bool enabled );

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollevelsdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
