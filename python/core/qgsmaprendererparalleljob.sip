
/** Job implementation that renders all layers in parallel.
 *
 * The resulting map image can be retrieved with renderedImage() function.
 * It is safe to call that function while rendering is active to see preview of the map.
 *
 * @note added in 2.4
 */
class QgsMapRendererParallelJob : QgsMapRendererQImageJob
{
%TypeHeaderCode
#include <qgsmaprendererparalleljob.h>
%End

  public:
    QgsMapRendererParallelJob( const QgsMapSettings& settings );
    ~QgsMapRendererParallelJob();

    virtual void start();
    virtual void cancel();
    virtual void cancelWithoutBlocking();
    virtual void waitForFinished();
    virtual bool isActive() const;
    virtual bool usedCachedLabels() const;

    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage();

};
