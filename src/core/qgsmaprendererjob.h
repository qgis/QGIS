#ifndef QGSMAPRENDERERJOB_H
#define QGSMAPRENDERERJOB_H

#include <QObject>
#include <QImage>

#include "qgsrendercontext.h"

#include "qgsmaprendererv2.h"

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

  // TODO: isActive() ?

signals:

  //! emitted when asynchronous rendering is finished (or canceled).
  void finished();

protected:

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

  virtual void start();
  virtual void cancel();

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

  virtual void start();
  virtual void cancel();
  //virtual QImage renderedImage();

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
};


#endif // QGSMAPRENDERERJOB_H
