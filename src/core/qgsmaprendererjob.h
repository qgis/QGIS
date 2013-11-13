#ifndef QGSMAPRENDERERJOB_H
#define QGSMAPRENDERERJOB_H

#include <QObject>
#include <QImage>

#include "qgsrendercontext.h"

#include "qgsmapsettings.h"

class QgsPalLabeling;


/** abstract base class renderer jobs that asynchronously start map rendering */
class QgsMapRendererJob : public QObject
{
  Q_OBJECT
public:

  enum Type
  {
    SequentialJob,
    CustomPainterJob
    //ParallelJob
  };


  QgsMapRendererJob(Type type, const QgsMapSettings& settings) : mType(type), mSettings(settings) { }

  virtual ~QgsMapRendererJob() {}

  Type type() const { return mType; }

  //! Start the rendering job and immediately return.
  virtual void start() = 0;

  //! Stop the rendering job - does not return until the job has terminated.
  virtual void cancel() = 0;

  //! Block until the job has finished.
  virtual void waitForFinished() = 0;

  // TODO: isActive() ?

  //! Assign an existing labeling engine with the rendering job
  //! TODO: handle concurrency - one labeling instance cannot be shared by multiple active rendering jobs!
  virtual void setLabelingEngine( QgsPalLabeling* labeling ) = 0;

signals:

  //! emitted when asynchronous rendering is finished (or canceled).
  void finished();

protected:

  /** Convenience function to project an extent into the layer source
   * CRS, but also split it into two extents if it crosses
   * the +/- 180 degree line. Modifies the given extent to be in the
   * source CRS coordinates, and if it was split, returns true, and
   * also sets the contents of the r2 parameter
   */
  static bool reprojectToLayerExtent(const QgsCoordinateTransform* ct, bool layerCrsGeographic, QgsRectangle& extent, QgsRectangle& r2 );


  Type mType;

  QgsMapSettings mSettings;
};


class QgsMapRendererQImageJob : public QgsMapRendererJob
{
public:
  QgsMapRendererQImageJob(Type type, const QgsMapSettings& settings);

  //! Get a preview/resulting image - in case QPainter has not been provided.
  //! With QPainter specified, it will return invalid QImage (there's no way to provide it).
  virtual QImage renderedImage() = 0;
};


class QgsMapRendererCustomPainterJob;


/** job implementation that renders everything sequentially in one thread */
class QgsMapRendererSequentialJob : public QgsMapRendererQImageJob
{
  Q_OBJECT
public:
  QgsMapRendererSequentialJob(const QgsMapSettings& settings);
  ~QgsMapRendererSequentialJob();

  virtual void start();
  virtual void cancel();
  virtual void waitForFinished();

  virtual void setLabelingEngine( QgsPalLabeling* labeling );

  // from QgsMapRendererJobWithPreview
  virtual QImage renderedImage();

public slots:

  void internalFinished();

protected:

  QgsMapRendererCustomPainterJob* mInternalJob;
  QImage mImage;
  QPainter* mPainter;
};



/** job implementation that renders all layers in parallel - the implication is that rendering is done to QImage */
//class QgsMapRendererParallelJob : public QgsMapRendererJobWithPreview
//{
//};


#include <QtConcurrentRun>
#include <QFutureWatcher>

/** job implementation that renders everything sequentially using a custom painter.
 *  The returned image is always invalid (because there is none available).
 */
class QgsMapRendererCustomPainterJob : public QgsMapRendererJob
{
  Q_OBJECT
public:
  QgsMapRendererCustomPainterJob(const QgsMapSettings& settings, QPainter* painter);
  ~QgsMapRendererCustomPainterJob();

  virtual void start();
  virtual void cancel();
  virtual void waitForFinished();

  virtual void setLabelingEngine( QgsPalLabeling* labeling );

protected slots:
  void futureFinished();

protected:
  static void staticRender(QgsMapRendererCustomPainterJob* self); // function to be used within the thread

  void startRender();

private:
  QPainter* mPainter;
  QFuture<void> mFuture;
  QFutureWatcher<void> mFutureWatcher;
  QgsRenderContext mRenderContext;
  QgsPalLabeling* mLabelingEngine;
};


#endif // QGSMAPRENDERERJOB_H
