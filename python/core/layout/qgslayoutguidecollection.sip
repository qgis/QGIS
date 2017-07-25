/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutguidecollection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutGuide
{
%Docstring
 Contains the configuration for a single snap guide used by a layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutguidecollection.h"
%End
  public:

    enum Orientation
    {
      Horizontal,
      Vertical,
    };

    QgsLayoutGuide( QgsLayout *layout, Orientation orientation, const QgsLayoutMeasurement &position );

    Orientation orientation() const;
%Docstring
 Returns the guide's orientation.
 :rtype: Orientation
%End

    QgsLayoutMeasurement position() const;
%Docstring
 Returns the guide's position within the page.

 The position indicates either the horizontal or vertical position
 of the guide, depending on the guide's orientation().

.. seealso:: setPosition()
 :rtype: QgsLayoutMeasurement
%End

    void setPosition( const QgsLayoutMeasurement &position );
%Docstring
 Sets the guide's ``position`` within the page.

 The ``position`` argument indicates either the horizontal or vertical position
 of the guide, depending on the guide's orientation().

.. seealso:: position()
%End

    int page() const;
%Docstring
 Returns the page number of the guide.

 Page numbering begins at 0.

.. seealso:: setPage()
 :rtype: int
%End

    void setPage( int page );
%Docstring
 Sets the ``page`` number of the guide.

 Page numbering begins at 0.

.. seealso:: page()
%End

    void update();
%Docstring
 Updates the position of the guide's line item.
%End

    QGraphicsLineItem *item();
%Docstring
 Returns the guide's line item.
 :rtype: QGraphicsLineItem
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutguidecollection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
