#ifndef QGSVECTORLAYER3DRENDERERWIDGET_H
#define QGSVECTORLAYER3DRENDERERWIDGET_H

#include <memory>

#include "qgsmaplayerconfigwidget.h"

class QCheckBox;
class QLabel;
class QStackedWidget;

class QgsLine3DSymbolWidget;
class QgsPoint3DSymbolWidget;
class QgsPolygon3DSymbolWidget;
class QgsVectorLayer;
class QgsMapCanvas;
class QgsVectorLayer3DRenderer;


//! Widget for configuration of 3D renderer of a vector layer
class QgsVectorLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );
    ~QgsVectorLayer3DRendererWidget();

    void setLayer( QgsVectorLayer *layer );

    //! no transfer of ownership
    void setRenderer( const QgsVectorLayer3DRenderer *renderer );
    //! no transfer of ownership
    QgsVectorLayer3DRenderer *renderer();

  public slots:
    virtual void apply() override;

  private slots:
    void onEnabledClicked();

  private:
    QCheckBox *chkEnabled;
    QStackedWidget *widgetStack;
    QgsLine3DSymbolWidget *widgetLine;
    QgsPoint3DSymbolWidget *widgetPoint;
    QgsPolygon3DSymbolWidget *widgetPolygon;
    QLabel *widgetUnsupported;

    std::unique_ptr<QgsVectorLayer3DRenderer> mRenderer;
};

#endif // QGSVECTORLAYER3DRENDERERWIDGET_H
