/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsvertexmarker.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




%ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgsvertexmarker.h>
%End

class QgsVertexMarker : QgsMapCanvasItem
{
%Docstring
A class for marking vertices of features using e.g. circles or 'x'.
%End

%TypeHeaderCode
#include "qgsvertexmarker.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsVertexMarker *>( sipCpp ) )
      sipType = sipType_QgsVertexMarker;
    else
      sipType = nullptr;
%End
  public:

    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE,
      ICON_DOUBLE_TRIANGLE,
    };

    QgsVertexMarker( QgsMapCanvas *mapCanvas /TransferThis/ );

    void setCenter( const QgsPointXY &point );

    void setIconType( int iconType );

    void setIconSize( int iconSize );

    void setColor( const QColor &color );
%Docstring
Sets the stroke ``color`` for the marker.

.. seealso:: :py:func:`color`

.. seealso:: :py:func:`setFillColor`
%End

    QColor color() const;
%Docstring
Returns the stroke color for the marker.

.. seealso:: :py:func:`setColor`

.. seealso:: :py:func:`fillColor`

.. versionadded:: 3.0
%End

    void setFillColor( const QColor &color );
%Docstring
Sets the fill ``color`` for the marker. This setting only
applies to some icon types.

.. versionadded:: 3.0

.. seealso:: :py:func:`fillColor`

.. seealso:: :py:func:`setColor`
%End

    QColor fillColor() const;
%Docstring
Returns the fill ``color`` for the marker. This setting only
applies to some icon types.

.. versionadded:: 3.0

.. seealso:: :py:func:`setFillColor`

.. seealso:: :py:func:`color`
%End

    void setPenWidth( int width );

    virtual void paint( QPainter *p );


    virtual QRectF boundingRect() const;


    virtual void updatePosition();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsvertexmarker.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
