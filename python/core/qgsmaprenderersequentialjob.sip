

/** job implementation that renders everything sequentially in one thread */
class QgsMapRendererSequentialJob : QgsMapRendererQImageJob
{
%TypeHeaderCode
#include <qgsmaprenderersequentialjob.h>
%End

  public:
    QgsMapRendererSequentialJob( const QgsMapSettings& settings );
    ~QgsMapRendererSequentialJob();

    virtual void start();
    virtual void cancel();
    virtual void waitForFinished();
    virtual bool isActive() const;

    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage();

  public slots:

    void internalFinished();
};
