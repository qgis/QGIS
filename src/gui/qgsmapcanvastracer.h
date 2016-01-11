#ifndef QGSMAPCANVASTRACER_H
#define QGSMAPCANVASTRACER_H

#include "qgstracer.h"

class QAction;
class QgsMapCanvas;
class QgsMessageBar;
class QgsMessageBarItem;

class GUI_EXPORT QgsMapCanvasTracer : public QgsTracer
{
    Q_OBJECT

  public:
    explicit QgsMapCanvasTracer( QgsMapCanvas* canvas, QgsMessageBar* messageBar = 0 );
    ~QgsMapCanvasTracer();

    QAction* actionEnableTracing() { return mActionEnableTracing; }

    virtual bool init();

    //! Retrieve instance of this class associated with given canvas (if any).
    //! The class keeps a simple registry of tracers associated with map canvas
    //! instances for easier access to the common tracer by various map tools
    static QgsMapCanvasTracer* tracerForCanvas( QgsMapCanvas* canvas );

    //! Report a path finding error to the user
    void reportError( PathError err, bool addingVertex );

  private slots:
    void updateSettings();
    void updateLayerSettings();
    void onCurrentLayerChanged();

  private:
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
    QgsMessageBarItem* mLastMessage;

    QAction* mActionEnableTracing;

    static QHash<QgsMapCanvas*, QgsMapCanvasTracer*> sTracers;
};

#endif // QGSMAPCANVASTRACER_H
