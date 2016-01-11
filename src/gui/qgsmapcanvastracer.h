#ifndef QGSMAPCANVASTRACER_H
#define QGSMAPCANVASTRACER_H

#include "qgsmessagebar.h"
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

    virtual bool init();

    //! Retrieve instance of this class associated with given canvas (if any).
    //! The class keeps a simple registry of tracers associated with map canvas
    //! instances for easier access to the common tracer by various map tools
    static QgsMapCanvasTracer* tracerForCanvas( QgsMapCanvas* canvas );

  signals:
    //! emit a message
    void messageEmitted( const QString& message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );

    //! emit signal to clear previous message
    //void messageDiscarded();

  private slots:
    void updateSettings();
    void updateLayerSettings();
    void onCurrentLayerChanged();

  private:
    QgsMapCanvas* mCanvas;

    QAction* mActionEnableTracing;

    static QHash<QgsMapCanvas*, QgsMapCanvasTracer*> sTracers;
};

#endif // QGSMAPCANVASTRACER_H
