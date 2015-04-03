#ifndef MAPRENDERERTEST_H
#define MAPRENDERERTEST_H

#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "qgsmapsettings.h"
#include "qgsmaplayer.h"

#include "qgsmaprenderersequentialjob.h"

class TestWidget : public QLabel
{
    Q_OBJECT
  public:
    TestWidget( QgsMapLayer* layer )
    {
      job = 0;

      // init renderer
      ms.setLayers( QStringList( layer->id() ) );
      ms.setExtent( layer->extent() );
      ms.setOutputSize( size() );

      if ( ms.hasValidSettings() )
        qDebug( "map renderer settings valid" );

      connect( &timer, SIGNAL( timeout() ), SLOT( onMapUpdateTimeout() ) );
      timer.setInterval( 100 );
    }

    void mousePressEvent( QMouseEvent * event ) override
    {
      if ( event->button() == Qt::RightButton )
      {
        qDebug( "cancelling!" );

        if ( job )
        {
          job->cancel();
          delete job;
          job = 0;
        }
      }
      else
      {
        qDebug( "starting!" );

        if ( job )
        {
          qDebug( "need to cancel first!" );
          job->cancel();
          delete job;
          job = 0;
        }

        job = new QgsMapRendererSequentialJob( ms );
        connect( job, SIGNAL( finished() ), SLOT( f() ) );

        job->start();

        timer.start();
      }
    }

  protected slots:
    void f()
    {
      qDebug( "finished!" );

      timer.stop();

      if ( job )
        setPixmap( QPixmap::fromImage( job->renderedImage() ) );
    }

    void onMapUpdateTimeout()
    {
      qDebug( "update timer!" );

      if ( job )
        setPixmap( QPixmap::fromImage( job->renderedImage() ) );
    }

  protected:
    QgsMapSettings ms;
    QgsMapRendererQImageJob* job;
    QTimer timer;
};



#endif // MAPRENDERERTEST_H
