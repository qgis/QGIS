#ifndef MAPRENDERERTEST_H
#define MAPRENDERERTEST_H

#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "qgsmaprendererv2.h"
#include "qgsmaplayer.h"

class TestWidget : public QLabel
{
  Q_OBJECT
public:
  TestWidget(QgsMapLayer* layer)
  {
    //p = QPixmap(200,200);
    //p.fill(Qt::red);

    i = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    i.fill(Qt::gray);

    // init renderer
    rend.setLayers(QStringList(layer->id()));
    rend.setExtent(layer->extent());
    rend.setOutputSize(i.size());
    rend.setOutputDpi(120);
    rend.updateDerived();

    if (rend.hasValidSettings())
      qDebug("map renderer settings valid");

    connect(&rend, SIGNAL(finished()), SLOT(f()));

    setPixmap(QPixmap::fromImage(i));

    connect(&timer, SIGNAL(timeout()), SLOT(onMapUpdateTimeout()));
    timer.setInterval(100);
  }

  void mousePressEvent(QMouseEvent * event)
  {
    if (event->button() == Qt::RightButton)
    {
      qDebug("cancelling!");

      rend.cancel();
    }
    else
    {
      qDebug("starting!");

      if (rend.isRendering())
      {
        qDebug("need to cancel first!");
        rend.cancel();

        // TODO: need to ensure that finished slot has been called
      }

      i.fill(Qt::gray);

      painter = new QPainter(&i);
      rend.startWithCustomPainter(painter);
      timer.start();
    }
  }

protected slots:
  void f()
  {
    qDebug("finished!");

    painter->end();
    delete painter;

    timer.stop();

    //update();

    setPixmap(QPixmap::fromImage(i));
  }

  void onMapUpdateTimeout()
  {
    qDebug("update timer!");

    setPixmap(QPixmap::fromImage(i));
  }

protected:
  //QPixmap p;
  QImage i;
  QPainter* painter;
  QgsMapRendererV2 rend;
  QTimer timer;
};



#endif // MAPRENDERERTEST_H
