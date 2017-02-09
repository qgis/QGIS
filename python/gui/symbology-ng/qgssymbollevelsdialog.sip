class QgsSymbolLevelsDialog : QDialog
{
%TypeHeaderCode
#include <qgssymbollevelsdialog.h>
%End

  public:
    //! @note not available in python bindings
    // QgsSymbolLevelsDialog( const QList< QPair<QString, QgsSymbol*> >& list, bool usingSymbolLevels, QWidget* parent = NULL );

    bool usingLevels() const;

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    //! @note not available in python bindings
    //void populateTable();
    //! @note not available in python bindings
    //void setDefaultLevels();

  private:
    QgsSymbolLevelsDialog();
};
