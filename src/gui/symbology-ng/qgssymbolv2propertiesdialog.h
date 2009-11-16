
#ifndef QGSSYMBOLV2PROPERTIESDIALOG_H
#define QGSSYMBOLV2PROPERTIESDIALOG_H

#include "ui_qgssymbolv2propertiesdialogbase.h"

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsSymbolLayerV2Widget;

class SymbolLayerItem;

#include <QMap>


class GUI_EXPORT QgsSymbolV2PropertiesDialog : public QDialog, private Ui::DlgSymbolV2Properties
{
    Q_OBJECT

  public:
    QgsSymbolV2PropertiesDialog( QgsSymbolV2* symbol, QWidget* parent = NULL );


  public slots:
    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void lockLayer();

    void layerTypeChanged();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

  protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol();

    void populateLayerTypes();

    void updateUi();

    void loadPropertyWidgets();

    void updateSymbolLayerWidget( QgsSymbolLayerV2* layer );
    void updateLockButton();

    int currentRowIndex();
    int currentLayerIndex();
    SymbolLayerItem* currentLayerItem();
    QgsSymbolLayerV2* currentLayer();

    void moveLayerByOffset( int offset );

  protected: // data
    QgsSymbolV2* mSymbol;

    QMap<QString, QgsSymbolLayerV2Widget*> mWidgets;
};

#endif
