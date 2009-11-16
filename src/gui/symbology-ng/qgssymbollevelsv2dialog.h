#ifndef QGSSYMBOLLEVELSV2DIALOG_H
#define QGSSYMBOLLEVELSV2DIALOG_H

#include <QDialog>
#include <QList>

#include "qgsrendererv2.h"

#include "ui_qgssymbollevelsv2dialogbase.h"


class GUI_EXPORT QgsSymbolLevelsV2Dialog : public QDialog, private Ui::QgsSymbolLevelsV2DialogBase
{
    Q_OBJECT
  public:
    QgsSymbolLevelsV2Dialog( QgsSymbolV2List symbols, bool usingSymbolLevels, QWidget* parent = NULL );

    bool usingLevels() const;

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    void populateTable();
    void setDefaultLevels();

  protected:
    //! maximal number of layers from all symbols
    int mMaxLayers;
    QgsSymbolV2List mSymbols;
};

#endif // QGSSYMBOLLEVELSV2DIALOG_H
