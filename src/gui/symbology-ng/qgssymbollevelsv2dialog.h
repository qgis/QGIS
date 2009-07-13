#ifndef QGSSYMBOLLEVELSV2DIALOG_H
#define QGSSYMBOLLEVELSV2DIALOG_H

#include <QDialog>
#include <QList>

#include "qgsrendererv2.h"

#include "ui_qgssymbollevelsv2dialogbase.h"

typedef QList<QgsSymbolV2*> QgsSymbolV2List;


class QgsSymbolLevelsV2Dialog : public QDialog, private Ui::QgsSymbolLevelsV2DialogBase
{
  Q_OBJECT
public:
    QgsSymbolLevelsV2Dialog(QgsSymbolV2List symbols, QgsSymbolV2LevelOrder levels, QWidget* parent = NULL);

    QgsSymbolV2LevelOrder& levels() { return mLevels; }

public slots:
    void updateUi();
    void updateLevels(QTableWidgetItem* item);

protected:
    void populateTable();
    void setDefaultLevels();
    int levelForSymbolLayer(QgsSymbolV2* sym, int layer);

protected:
    //! maximal number of layers from all symbols
    int mMaxLayers;
    QgsSymbolV2List mSymbols;
    QgsSymbolV2LevelOrder mLevels;
};

#endif // QGSSYMBOLLEVELSV2DIALOG_H
