#ifndef QGSVECTORLAYER3DRENDERERWIDGET_H
#define QGSVECTORLAYER3DRENDERERWIDGET_H

#include <memory>

#include "qgsmaplayerconfigwidget.h"

class QCheckBox;
class QgsPolygon3DSymbolWidget;
class QgsVectorLayer;
class QgsMapCanvas;
class VectorLayer3DRenderer;


//! Widget for configuration of 3D renderer of a vector layer
class QgsVectorLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );
    ~QgsVectorLayer3DRendererWidget();

    void setLayer( QgsVectorLayer *layer );

    //! no transfer of ownership
    void setRenderer( const VectorLayer3DRenderer *renderer );
    //! no transfer of ownership
    VectorLayer3DRenderer *renderer();

  public slots:
    virtual void apply() override;

  private slots:
    void onEnabledClicked();

  private:
    QCheckBox *chkEnabled;
    QgsPolygon3DSymbolWidget *widgetPolygon;

    std::unique_ptr<VectorLayer3DRenderer> mRenderer;
};

#endif // QGSVECTORLAYER3DRENDERERWIDGET_H
