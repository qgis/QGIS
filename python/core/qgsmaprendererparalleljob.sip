
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
    virtual void waitForFinished();
    virtual bool isActive() const;

    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage();

  protected slots:
    //! layers are rendered, labeling is still pending
    void renderLayersFinished();
    //! all rendering is finished, including labeling
    void renderingFinished();

  protected:

    static void renderLayerStatic( LayerRenderJob& job );
    static void renderLabelsStatic( QgsMapRendererParallelJob* self );
};
