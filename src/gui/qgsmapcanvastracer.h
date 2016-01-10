#ifndef QGSMAPCANVASTRACER_H
#define QGSMAPCANVASTRACER_H

#include "qgstracer.h"

class QAction;
class QgsMapCanvas;

class GUI_EXPORT QgsMapCanvasTracer : public QgsTracer
{
    Q_OBJECT

  public:
    explicit QgsMapCanvasTracer( QgsMapCanvas* canvas );
    ~QgsMapCanvasTracer();

    QAction* actionEnableTracing() { return mActionEnableTracing; }

    //! Retrieve instance of this class associated with given canvas (if any).
    //! The class keeps a simple registry of tracers associated with map canvas
    //! instances for easier access to the common tracer by various map tools
    static QgsMapCanvasTracer* tracerForCanvas( QgsMapCanvas* canvas );

  private slots:
    void updateSettings();

  private:
    QgsMapCanvas* mCanvas;

    QAction* mActionEnableTracing;

    static QHash<QgsMapCanvas*, QgsMapCanvasTracer*> sTracers;
};

#endif // QGSMAPCANVASTRACER_H
