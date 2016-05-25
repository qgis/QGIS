#ifndef QGSRENDERERRASTERPROPERTIESDIALOG_H
#define QGSRENDERERRASTERPROPERTIESDIALOG_H

#include <QObject>
#include <QDialog>

#include "ui_qgsrendererrasterpropswidgetbase.h"

class QgsRasterLayer;
class QgsMapCanvas;
class QgsRasterRendererWidget;

class GUI_EXPORT QgsRendererRasterPropertiesWidget : public QWidget, private Ui::QgsRendererRasterPropsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * A widget to hold the renderer properties for a raster layer.
     * @param canvas The canvas object used to calculate the max and min values from the extent.
     * @param parent Parent object
     */
    QgsRendererRasterPropertiesWidget( QgsMapCanvas *canvas, QWidget *parent = 0 );
    ~QgsRendererRasterPropertiesWidget();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * Return the active render widget. Can be null.
     */
    QgsRasterRendererWidget* currentRenderWidget() { return mRendererWidget; }

  signals:

    /**
     * Emmited when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply();

    /**
     * @brief Sync the widget to the given layer.
     * @param layer The layer to use for the widget
     */
    void syncToLayer( QgsRasterLayer *layer );

  private slots:
    /** Slot to reset all color rendering options to default */
    void on_mResetColorRenderingBtn_clicked();

    /** Enable or disable saturation controls depending on choice of grayscale mode */
    void toggleSaturationControls( int grayscaleMode );

    /** Enable or disable colorize controls depending on checkbox */
    void toggleColorizeControls( bool colorizeEnabled );
  private:
    void setRendererWidget( const QString& rendererName );

    QgsRasterLayer* mRasterLayer;
    QgsMapCanvas* mMapCanvas;
    QgsRasterRendererWidget* mRendererWidget;
};

#endif // QGSRENDERERRASTERPROPERTIESDIALOG_H
