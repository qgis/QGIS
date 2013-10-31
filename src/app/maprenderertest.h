#ifndef MAPRENDERERTEST_H
#define MAPRENDERERTEST_H

#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "qgsmapsettings.h"
#include "qgsmaplayer.h"

#include "qgsmaprendererjob.h"

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

    job = 0;

    // init renderer
    ms.setLayers(QStringList(layer->id()));
    ms.setExtent(layer->extent());
    ms.setOutputSize(i.size());
    ms.setOutputDpi(120);
    ms.updateDerived();

    if (ms.hasValidSettings())
      qDebug("map renderer settings valid");

    setPixmap(QPixmap::fromImage(i));

    connect(&timer, SIGNAL(timeout()), SLOT(onMapUpdateTimeout()));
    timer.setInterval(100);
  }

  void mousePressEvent(QMouseEvent * event)
  {
    if (event->button() == Qt::RightButton)
    {
      qDebug("cancelling!");

      if (job)
      {
        job->cancel();
        delete job;
        job = 0;
      }
    }
    else
    {
      qDebug("starting!");

      if (job)
      {
        qDebug("need to cancel first!");
        job->cancel();
        delete job;
        job = 0;
      }

      i.fill(Qt::gray);

      painter = new QPainter(&i);

      job = new QgsMapRendererCustomPainterJob(ms, painter);
      connect(job, SIGNAL(finished()), SLOT(f()));

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
  QgsMapSettings ms;
  QgsMapRendererJob* job;
  QTimer timer;
};



#endif // MAPRENDERERTEST_H
