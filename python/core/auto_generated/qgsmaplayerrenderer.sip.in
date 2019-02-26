/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerrenderer.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapLayerRenderer
{
%Docstring
Base class for utility classes that encapsulate information necessary
for rendering of map layers. The rendering is typically done in a background
thread, so it is necessary to keep all structures required for rendering away
from the original map layer because it may change any time.

Because the data needs to be copied (to avoid the need for locking),
it is highly desirable to use copy-on-write where possible. This way,
the overhead of copying (both memory and CPU) will be kept low.
Qt containers and various Qt classes use implicit sharing.

The scenario will be:
1. renderer job (doing preparation in the GUI thread) calls
QgsMapLayer.createMapRenderer() and gets instance of this class.
The instance is initialized at that point and should not need
additional calls to QgsVectorLayer.
2. renderer job (still in GUI thread) stores the renderer for later use.
3. renderer job (in worker thread) calls :py:func:`QgsMapLayerRenderer.render()`
4. renderer job (again in GUI thread) will check errors() and report them

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmaplayerrenderer.h"
%End
  public:
    QgsMapLayerRenderer( const QString &layerID );
    virtual ~QgsMapLayerRenderer();

    virtual bool render() = 0;
%Docstring
Do the rendering (based on data stored in the class)
%End

    virtual QgsFeedback *feedback() const;
%Docstring
Access to feedback object of the layer renderer (may be ``None``)

.. versionadded:: 3.0
%End

    QStringList errors() const;
%Docstring
Returns list of errors (problems) that happened during the rendering
%End

    QString layerId() const;
%Docstring
Gets access to the ID of the layer rendered by this class
%End

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerrenderer.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
