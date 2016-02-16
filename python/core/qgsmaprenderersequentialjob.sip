

/** Job implementation that renders everything sequentially in one thread.
 *
 * The resulting map image can be retrieved with renderedImage() function.
 * It is safe to call that function while rendering is active to see preview of the map.
 *
 * @note added in 2.4
 */
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
