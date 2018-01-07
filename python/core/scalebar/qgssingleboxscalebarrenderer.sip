/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgssingleboxscalebarrenderer.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSingleBoxScaleBarRenderer: QgsScaleBarRenderer
{
%Docstring
Scalebar style that draws a single box with alternating
color for the segments.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssingleboxscalebarrenderer.h"
%End
  public:

    QgsSingleBoxScaleBarRenderer();
%Docstring
Constructor for QgsSingleBoxScaleBarRenderer.
%End

    virtual QString name() const;

     virtual void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgssingleboxscalebarrenderer.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
