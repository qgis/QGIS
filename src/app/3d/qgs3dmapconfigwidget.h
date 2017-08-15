#ifndef QGS3DMAPCONFIGWIDGET_H
#define QGS3DMAPCONFIGWIDGET_H

#include <QWidget>

#include <ui_map3dconfigwidget.h>

class Qgs3DMapSettings;

class QgsMapCanvas;


class Qgs3DMapConfigWidget : public QWidget, private Ui::Map3DConfigWidget
{
    Q_OBJECT
  public:
    //! construct widget. does not take ownership of the passed map.
    explicit Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, QWidget *parent = nullptr );
    ~Qgs3DMapConfigWidget();

    void apply();

  signals:

  private slots:
    void onTerrainLayerChanged();
    void updateMaxZoomLevel();

  private:
    Qgs3DMapSettings *mMap;
    QgsMapCanvas *mMainCanvas;
};

#endif // QGS3DMAPCONFIGWIDGET_H
