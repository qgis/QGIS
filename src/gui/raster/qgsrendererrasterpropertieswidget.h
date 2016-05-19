#ifndef QGSRENDERERRASTERPROPERTIESDIALOG_H
#define QGSRENDERERRASTERPROPERTIESDIALOG_H

#include <QObject>
#include <QDialog>

#include "ui_qgsrendererrasterpropswidgetbase.h"

class QgsRasterLayer;
class QgsMapCanvas;
class QgsRasterRendererWidget;

class GUI_EXPORT QgsRendererRasterPropertiesWidget : public QDialog, private Ui::QgsRendererRasterPropsWidgetBase
{
    Q_OBJECT

  public:
    QgsRendererRasterPropertiesWidget( QgsRasterLayer* layer, QgsMapCanvas *canvas, QObject *parent = 0 );
    ~QgsRendererRasterPropertiesWidget();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

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

  private:
    void setRendererWidget( const QString& rendererName );

    QgsRasterLayer* mRasterLayer;
    QgsMapCanvas* mMapCanvas;
    QgsRasterRendererWidget* mRendererWidget;
};

#endif // QGSRENDERERRASTERPROPERTIESDIALOG_H
