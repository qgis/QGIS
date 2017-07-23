/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsLayoutSnapper
{
%Docstring
 Manages snapping grids and preset snap lines in a layout, and handles
 snapping points to the nearest grid coordinate/snap line when possible.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutsnapper.h"
%End
  public:

    enum GridStyle
    {
      GridLines,
      GridDots,
      GridCrosses
    };

    QgsLayoutSnapper();

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
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
