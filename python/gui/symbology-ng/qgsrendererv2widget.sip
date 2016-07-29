class QgsRendererV2Widget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style );

    virtual ~QgsRendererV2Widget();

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRendererV2* renderer() = 0;

    //! show a dialog with renderer's symbol level settings
    void showSymbolLevelsDialog( QgsFeatureRendererV2* r );

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const;

    /**
     * This method should be called whenever the renderer is actually set on the layer.
     */
    void applyChanges();

  signals:
    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

  protected:
    /** Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
      and by connecting the slot contextMenuViewCategories(const QPoint&)*/
    virtual QList<QgsSymbolV2*> selectedSymbols();
    virtual void refreshSymbolView();

  protected slots:
    void  contextMenuViewCategories( QPoint p );
    /** Change color of selected symbols*/
    void changeSymbolColor();
    /** Change opacity of selected symbols*/
    void changeSymbolTransparency();
    /** Change units mm/map units of selected symbols*/
    void changeSymbolUnit();
    /** Change line widths of selected symbols*/
    void changeSymbolWidth();
    /** Change marker sizes of selected symbols*/
    void changeSymbolSize();
    /** Change marker angles of selected symbols*/
    void changeSymbolAngle();

    virtual void copy();
    virtual void paste();

  private:
    /**
     * This will be called whenever the renderer is set on a layer.
     * This can be overwritten in subclasses.
     */
    virtual void apply();

};


/**
Utility classes for "en masse" size definition
*/
class QgsDataDefinedValueDialog : QDialog
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    /** Constructor
     * @param symbolList must not be empty
     * @param layer must not be null
     * @param label value label
     */
    QgsDataDefinedValueDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer, const QString & label );
    virtual ~QgsDataDefinedValueDialog();

    /** Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const;

  public slots:
    void dataDefinedChanged();

  protected:
    QgsDataDefined symbolDataDefined() const;
    void init( const QString & description ); // needed in children ctor to call virtual

    virtual QgsDataDefined symbolDataDefined( const QgsSymbolV2 * ) const = 0;
    virtual double value( const QgsSymbolV2 * ) const = 0;
    virtual void setDataDefined( QgsSymbolV2* symbol, const QgsDataDefined& dd ) = 0;
};

class QgsDataDefinedSizeDialog : QgsDataDefinedValueDialog
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsDataDefinedSizeDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer );

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbolV2 * symbol ) const;

    double value( const QgsSymbolV2 * symbol ) const;

    void setDataDefined( QgsSymbolV2* symbol, const QgsDataDefined& dd );
};

class QgsDataDefinedRotationDialog : QgsDataDefinedValueDialog
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsDataDefinedRotationDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer );

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbolV2 * symbol ) const;

    double value( const QgsSymbolV2 * symbol ) const;

    void setDataDefined( QgsSymbolV2* symbol, const QgsDataDefined& dd );
};


class QgsDataDefinedWidthDialog : QgsDataDefinedValueDialog
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsDataDefinedWidthDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer );

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbolV2 * symbol ) const;

    double value( const QgsSymbolV2 * symbol ) const;

    void setDataDefined( QgsSymbolV2* symbol, const QgsDataDefined& dd );
};
