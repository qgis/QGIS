class QgsSymbolSelectorWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgssymbolselectordialog.h>
%End

  public:
    QgsSymbolSelectorWidget( QgsSymbol* symbol, QgsStyle* style, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0);

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @param context symbol widget context
     * @see context()
     * @note added in QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext& context );

    /** Returns the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @see setContext()
     * @note added in QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * @brief Return the symbol that is currently active in the widget. Can be null.
     * @return The active symbol.
     */
    QgsSymbol* symbol();

  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol();
    //! @note not available in python bindings
    // void loadSymbol( QgsSymbol* symbol, SymbolLayerItem* parent );

    void updateUi();

    void updateLockButton();

    //! @note not available in python bindings
    // SymbolLayerItem* currentLayerItem();
    QgsSymbolLayer* currentLayer();

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
    void changeLayer( QgsSymbolLayer* layer );
};

class QgsSymbolSelectorDialog : QDialog
{
%TypeHeaderCode
#include <qgssymbolselectordialog.h>
%End

  public:
    QgsSymbolSelectorDialog( QgsSymbol* symbol, QgsStyle* style, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = 0, bool embedded = false );

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @param context symbol widget context
     * @see context()
     * @note added in QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext& context );

    /** Returns the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @see setContext()
     * @note added in QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    QgsSymbol* symbol();
  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol();
    //! @note not available in python bindings
    // void loadSymbol( QgsSymbol* symbol, SymbolLayerItem* parent );

    void updateUi();

    void updateLockButton();

    //! @note not available in python bindings
    // SymbolLayerItem* currentLayerItem();
    QgsSymbolLayer* currentLayer();

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
    void changeLayer( QgsSymbolLayer* layer );


};
