/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsscalebarrenderer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsScaleBarRenderer
{
%Docstring
 Abstract base class for scale bar renderers.

 Scalebar renderer subclasses implement custom drawing logic, with the possibility to implement
 custom labeling.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsscalebarrenderer.h"
%End
  public:

    struct ScaleBarContext
    {
      double segmentWidth;
%Docstring
Width of each individual segment (in millimeters)
%End

      QSizeF size;
%Docstring
 Destination size for scalebar. This is used for scalebars which
 alter their appearance or alignment based on the desired scalebar
 size (e.g. correctly aligning text in a numeric scale bar).
%End

      double scale;
%Docstring
Scale denominator
%End

    };

    QgsScaleBarRenderer();
%Docstring
 Constructor for QgsScaleBarRenderer.
%End
    virtual ~QgsScaleBarRenderer();

    virtual QString name() const = 0;
%Docstring
 Returns the unique name for this style.
 :rtype: str
%End

    virtual void draw( QgsRenderContext &context,
                       const QgsScaleBarSettings &settings,
                       const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const = 0;
%Docstring
 Draws the scalebar using the specified ``settings`` and ``scaleContext`` to a destination render ``context``.
%End

    virtual QSizeF calculateBoxSize( const QgsScaleBarSettings &settings,
                                     const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;
%Docstring
 Calculates the required box size (in millimeters) for a scalebar using the specified ``settings`` and ``scaleContext``.
 :rtype: QSizeF
%End

  protected:

    void drawDefaultLabels( QgsRenderContext &context,
                            const QgsScaleBarSettings &settings,
                            const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;
%Docstring
 Draws default scalebar labels using the specified ``settings`` and ``scaleContext`` to a destination render ``context``.
%End

    QString firstLabelString( const QgsScaleBarSettings &settings ) const;
%Docstring
 Returns the text used for the first label in the scalebar.
 :rtype: str
%End

    double firstLabelXOffset( const QgsScaleBarSettings &settings ) const;
%Docstring
 Returns the x-offset (in millimeters) used for the first label in the scalebar.
 :rtype: float
%End

    QList<double> segmentPositions( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;
%Docstring
 Returns a list of positions for each segment within the scalebar.
 :rtype: list of float
%End

    QList<double> segmentWidths( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;
%Docstring
 Returns a list of widths of each segment of the scalebar.
 :rtype: list of float
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsscalebarrenderer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
