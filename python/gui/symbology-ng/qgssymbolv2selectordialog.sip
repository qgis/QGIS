class QgsSymbolV2SelectorDialog : QDialog
{
%TypeHeaderCode
#include <qgssymbolv2selectordialog.h>
%End

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = 0, bool embedded = false );

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol();
    //! @note not available in python bindings
    // void loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent );

    void updateUi();

    void updateLockButton();

    //! @note not available in python bindings
    // SymbolLayerItem* currentLayerItem();
    QgsSymbolLayerV2* currentLayer();

    void moveLayerByOffset( int offset );

    void setWidget( QWidget* widget );

  signals:
    void symbolModified();

  public slots:
    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void saveSymbol();
    void lockLayer();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();
    //! alters tree and sets proper widget when Layer Type is changed
    //! @note: The layer is received from the LayerPropertiesWidget
    void changeLayer( QgsSymbolLayerV2* layer );


};
