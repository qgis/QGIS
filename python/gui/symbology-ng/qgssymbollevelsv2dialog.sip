class QgsSymbolLevelsV2Dialog : QDialog
{
%TypeHeaderCode
#include <qgssymbollevelsv2dialog.h>
%End

  public:
    // @note not available in python bindings
    // QgsSymbolLevelsV2Dialog( QList< QPair<QString, QgsSymbolV2*> > list, bool usingSymbolLevels, QWidget* parent = NULL );

    bool usingLevels() const;

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    void populateTable();
    void setDefaultLevels();

  private:
    QgsSymbolLevelsV2Dialog();
};
