/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutcontext.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutContext
{
%Docstring
 Stores information relating to the current context and rendering settings for a layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutcontext.h"
%End
  public:

    enum Flag
    {
      FlagDebug,
      FlagOutlineOnly,
      FlagAntialiasing,
      FlagUseAdvancedEffects,
    };
    typedef QFlags<QgsLayoutContext::Flag> Flags;


    enum GridStyle
    {
      GridLines,
      GridDots,
      GridCrosses
    };

    QgsLayoutContext();

    void setFlags( const QgsLayoutContext::Flags flags );
%Docstring
 Sets the combination of ``flags`` that will be used for rendering the layout.
.. seealso:: setFlag()
.. seealso:: flags()
.. seealso:: testFlag()
%End

    void setFlag( const QgsLayoutContext::Flag flag, const bool on = true );
%Docstring
 Enables or disables a particular rendering ``flag`` for the layout. Other existing
 flags are not affected.
.. seealso:: setFlags()
.. seealso:: flags()
.. seealso:: testFlag()
%End

    QgsLayoutContext::Flags flags() const;
%Docstring
 Returns the current combination of flags used for rendering the layout.
.. seealso:: setFlags()
.. seealso:: setFlag()
.. seealso:: testFlag()
 :rtype: QgsLayoutContext.Flags
%End

    bool testFlag( const Flag flag ) const;
%Docstring
 Check whether a particular rendering ``flag`` is enabled for the layout.
.. seealso:: setFlags()
.. seealso:: setFlag()
.. seealso:: flags()
 :rtype: bool
%End

    QgsRenderContext::Flags renderContextFlags() const;
%Docstring
 Returns the combination of render context flags matched to the layout context's settings.
 :rtype: QgsRenderContext.Flags
%End

    void setFeature( const QgsFeature &feature );
%Docstring
 Sets the current ``feature`` for evaluating the layout. This feature may
 be used for altering an item's content and appearance for a report
 or atlas layout.
.. seealso:: feature()
%End

    QgsFeature feature() const;
%Docstring
 Returns the current feature for evaluating the layout. This feature may
 be used for altering an item's content and appearance for a report
 or atlas layout.
.. seealso:: setFeature()
 :rtype: QgsFeature
%End

    QgsVectorLayer *layer() const;
%Docstring
 Returns the vector layer associated with the layout's context.
.. seealso:: setLayer()
 :rtype: QgsVectorLayer
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
 Sets the vector ``layer`` associated with the layout's context.
.. seealso:: layer()
%End

    void setDpi( double dpi );
%Docstring
 Sets the ``dpi`` for outputting the layout. This also sets the
 corresponding DPI for the context's measurementConverter().
.. seealso:: dpi()
%End

    double dpi() const;
%Docstring
 Returns the ``dpi`` for outputting the layout.
.. seealso:: setDpi()
 :rtype: float
%End


    QgsLayoutMeasurementConverter &measurementConverter();
%Docstring
 Returns the layout measurement converter to be used in the layout. This converter is used
 for translating between other measurement units and the layout's native unit.
 :rtype: QgsLayoutMeasurementConverter
%End

    bool gridVisible() const;
%Docstring
 Returns true if the page grid should be drawn.
 :rtype: bool
%End

    void setGridResolution( const QgsLayoutMeasurement &resolution );
%Docstring
 Sets the page/snap grid ``resolution``.
.. seealso:: gridResolution()
.. seealso:: setGridOffset()
%End

    QgsLayoutMeasurement gridResolution() const;
%Docstring
 Returns the page/snap grid resolution.
.. seealso:: setGridResolution()
.. seealso:: gridOffset()
 :rtype: QgsLayoutMeasurement
%End

    void setGridOffset( const QgsLayoutPoint offset );
%Docstring
 Sets the ``offset`` of the page/snap grid.
.. seealso:: gridOffset()
.. seealso:: setGridResolution()
%End

    QgsLayoutPoint gridOffset() const;
%Docstring
 Returns the offset of the page/snap grid.
.. seealso:: setGridOffset()
.. seealso:: gridResolution()
 :rtype: QgsLayoutPoint
%End

    void setGridPen( const QPen &pen );
%Docstring
 Sets the ``pen`` used for drawing page/snap grids.
.. seealso:: gridPen()
.. seealso:: setGridStyle()
%End

    QPen gridPen() const;
%Docstring
 Returns the pen used for drawing page/snap grids.
.. seealso:: setGridPen()
.. seealso:: gridStyle()
 :rtype: QPen
%End

    void setGridStyle( const GridStyle style );
%Docstring
 Sets the ``style`` used for drawing the page/snap grids.
.. seealso:: gridStyle()
.. seealso:: setGridPen()
%End

    GridStyle gridStyle() const;
%Docstring
 Returns the style used for drawing the page/snap grids.
.. seealso:: setGridStyle()
.. seealso:: gridPen()
 :rtype: GridStyle
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutcontext.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
