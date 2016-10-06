
/** \ingroup core
 * Base class for feedback objects to be used for cancellation of something running in a worker thread.
 * The class may be used as is or it may be subclassed for extended functionality
 * for a particular operation (e.g. report progress or pass some data for preview).
 *
 * When cancel() is called, the internal code has two options to check for cancellation state:
 * - if the worker thread uses an event loop (e.g. for network communication), the code can
 *   make a queued connection to cancelled() signal and handle the cancellation in its slot.
 * - if the worker thread does not use an event loop, it can poll isCancelled() method regularly
 *   to see if the operation should be cancelled.
 *
 * The class is meant to be created and destroyed in the main thread.
 *
 * For map rendering, the object may be created in constructor of a QgsMapLayerRenderer
 * subclass and available with QgsMapLayerRenderer::feedback() method. When a map rendering job
 * gets cancelled, the cancel() method is called on the feedback object of all layers.
 *
 * @note added in QGIS 3.0
 */
class QgsFeedback : QObject
{
%TypeHeaderCode
#include <qgsfeedback.h>
%End

  public:
    //! Construct a feedback object
    QgsFeedback( QObject* parent /TransferThis/ = nullptr );

    virtual ~QgsFeedback();

    //! Tells the internal routines that the current operation should be cancelled. This should be run by the main thread
    void cancel();

    //! Tells whether the operation has been cancelled already
    bool isCancelled() const;

  signals:
    //! Internal routines can connect to this signal if they use event loop
    void cancelled();

};
