class QgsSymbolV2SelectorWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgssymbolv2selectordialog.h>
%End

  public:
    QgsSymbolV2SelectorWidget( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0);

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is transferred to the dialog.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context /Transfer/ );

    /** Returns the expression context used for the dialog, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the dialog.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const;

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * @brief Return the symbol that is currently active in the widget. Can be null.
     * @return The active symbol.
     */
    QgsSymbolV2* symbol();

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

    void lockLayer();

    void saveSymbol() /Deprecated/;

    //! Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
    //! @note added in QGIS 2.14
    void duplicateLayer();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();
    //! alters tree and sets proper widget when Layer Type is changed
    //! @note: The layer is received from the LayerPropertiesWidget
    void changeLayer( QgsSymbolLayerV2* layer );
};

class QgsSymbolV2SelectorDialog : QDialog
{
%TypeHeaderCode
#include <qgssymbolv2selectordialog.h>
%End

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0, bool embedded = false );

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is transferred to the dialog.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context /Transfer/ );

    /** Returns the expression context used for the dialog, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the dialog.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const;

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    QgsSymbolV2* symbol();
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

    void lockLayer();

    void saveSymbol() /Deprecated/;

    //! Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
    //! @note added in QGIS 2.14
    void duplicateLayer();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();
    //! alters tree and sets proper widget when Layer Type is changed
    //! @note: The layer is received from the LayerPropertiesWidget
    void changeLayer( QgsSymbolLayerV2* layer );


};
