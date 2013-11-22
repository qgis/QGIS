#ifndef QGSMAPLAYERRENDERER_H
#define QGSMAPLAYERRENDERER_H

#include <QList>
#include <QString>

/**
 * Base class for utility classes that encapsulate information necessary
 * for rendering of map layers. The rendering is typically done in a background
 * thread, so it is necessary to keep all structures required for rendering away
 * from the original map layer because it may change any time.
 *
 * Because the data needs to be copied (to avoid the need for locking),
 * is is highly desirable to use copy-on-write where possible. This way,
 * the overhead of copying (both memory and CPU) will be kept low.
 * Qt containers and various Qt classes use implicit sharing.
 *
 * The scenario will be:
 * 1. renderer job (doing preparation in the GUI thread) calls
 *    QgsMapLayer::createMapRenderer() and gets instance of this class.
 *    The instance is initialized at that point and should not need
 *    additional calls to QgsVectorLayer.
 * 2. renderer job (still in GUI thread) stores the renderer for later use.
 * 3. renderer job (in worker thread) calls QgsMapLayerRenderer::render()
 * 4. renderer job (again in GUI thread) will check errors() and report them
 */
class QgsMapLayerRenderer
{
public:
  virtual ~QgsMapLayerRenderer() {}

  //! Do the rendering (based on data stored in the class)
  virtual bool render() = 0;

  //! Container for errors (@todo instead of simple message could have error codes + custom data)
  struct Error
  {
    QString message;
  };

  typedef QList<Error> ErrorList;

  //! Return list of errors (problems) that happened during the rendering
  ErrorList errors() const { return mErrors; }

protected:
  ErrorList mErrors;
};

#endif // QGSMAPLAYERRENDERER_H
