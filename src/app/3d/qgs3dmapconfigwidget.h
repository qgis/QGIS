#ifndef QGS3DMAPCONFIGWIDGET_H
#define QGS3DMAPCONFIGWIDGET_H

#include <QWidget>

#include <ui_map3dconfigwidget.h>

class Map3D;

class Qgs3DMapConfigWidget : public QWidget, private Ui::Map3DConfigWidget
{
    Q_OBJECT
  public:
    //! construct widget. does not take ownership of the passed map.
    explicit Qgs3DMapConfigWidget( const Map3D *map, QWidget *parent = nullptr );
    ~Qgs3DMapConfigWidget();

    Map3D *map();

  signals:

  public slots:

  private:
    Map3D *mMap;
};

#endif // QGS3DMAPCONFIGWIDGET_H
