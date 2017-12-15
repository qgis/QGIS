/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsnumericscalebarrenderer.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsNumericScaleBarRenderer: QgsScaleBarRenderer
{
%Docstring
A scale bar style that draws text in the form of '1:XXXXX'.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsnumericscalebarrenderer.h"
%End
  public:

    QgsNumericScaleBarRenderer();
%Docstring
Constructor for QgsNumericScaleBarRenderer.
%End

    virtual QString name() const;

     virtual void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

     virtual QSizeF calculateBoxSize( const QgsScaleBarSettings &settings,
                             const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsnumericscalebarrenderer.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
