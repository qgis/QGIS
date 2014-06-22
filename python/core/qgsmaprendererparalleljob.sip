
/** job implementation that renders all layers in parallel */
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
