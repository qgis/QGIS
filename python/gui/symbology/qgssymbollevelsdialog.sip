/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollevelsdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSymbolLevelsDialog : QDialog
{

%TypeHeaderCode
#include "qgssymbollevelsdialog.h"
%End
  public:

    ~QgsSymbolLevelsDialog();

    bool usingLevels() const;
%Docstring
 :rtype: bool
%End

    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:


  private:
    QgsSymbolLevelsDialog();
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbollevelsdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
