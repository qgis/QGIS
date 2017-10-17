/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutSnapper: QgsLayoutSerializableObject
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

    QgsLayoutSnapper( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutSnapper, attached to the specified ``layout``.
%End

    virtual QString stringType() const;
    virtual QgsLayout *layout();


    void setSnapTolerance( const int snapTolerance );
%Docstring
 Sets the snap ``tolerance`` (in pixels) to use when snapping.
.. seealso:: snapTolerance()
%End

    int snapTolerance() const;
%Docstring
 Returns the snap tolerance (in pixels) to use when snapping.
.. seealso:: setSnapTolerance()
 :rtype: int
%End

    bool snapToGrid() const;
%Docstring
 Returns true if snapping to grid is enabled.
.. seealso:: setSnapToGrid()
 :rtype: bool
%End

    void setSnapToGrid( bool enabled );
%Docstring
 Sets whether snapping to grid is ``enabled``.
.. seealso:: snapToGrid()
%End

    bool snapToGuides() const;
%Docstring
 Returns true if snapping to guides is enabled.
.. seealso:: setSnapToGuides()
 :rtype: bool
%End

    void setSnapToGuides( bool enabled );
%Docstring
 Sets whether snapping to guides is ``enabled``.
.. seealso:: snapToGuides()
%End

    QPointF snapPoint( QPointF point, double scaleFactor, bool &snapped /Out/ ) const;
%Docstring
 Snaps a layout coordinate ``point``. If ``point`` was snapped, ``snapped`` will be set to true.

 The ``scaleFactor`` argument should be set to the transformation from
 scalar transform from layout coordinates to pixels, i.e. the
 graphics view transform().m11() value.

 This method considers snapping to the grid, snap lines, etc.
 :rtype: QPointF
%End

    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX /Out/, bool &snappedY /Out/ ) const;
%Docstring
 Snaps a layout coordinate ``point`` to the grid. If ``point``
 was snapped horizontally, ``snappedX`` will be set to true. If ``point``
 was snapped vertically, ``snappedY`` will be set to true.

 The ``scaleFactor`` argument should be set to the transformation from
 scalar transform from layout coordinates to pixels, i.e. the
 graphics view transform().m11() value.

 If snapToGrid() is disabled, this method will return the point
 unchanged.
 :rtype: QPointF
%End

    double snapPointToGuides( double original, QgsLayoutGuide::Orientation orientation, double scaleFactor, bool &snapped /Out/ ) const;
%Docstring
 Snaps a layout coordinate ``point`` to the grid. If ``point``
 was snapped, ``snapped`` will be set to true.

 The ``scaleFactor`` argument should be set to the transformation from
 scalar transform from layout coordinates to pixels, i.e. the
 graphics view transform().m11() value.

 If snapToGrid() is disabled, this method will return the point
 unchanged.
 :rtype: float
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
 Stores the snapper's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.
.. seealso:: readXml()
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &gridElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
 Sets the snapper's state from a DOM element. snapperElement is the DOM node corresponding to the snapper.
.. seealso:: writeXml()
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
