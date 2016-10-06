class QgsLayerPropertiesWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgslayerpropertieswidget.h>
%End

  public:
    QgsLayerPropertiesWidget( QgsSymbolLayer* layer, const QgsSymbol* symbol, const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

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


  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:
    void changed();
    void changeLayer( QgsSymbolLayer* );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayer* layer );
};
