/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutgridsettings.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutGridSettings : QgsLayoutSerializableObject
{
%Docstring
 Contains settings relating to the appearance, spacing and offset for layout grids.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutgridsettings.h"
%End
  public:

    enum Style
    {
      StyleLines,
      StyleDots,
      StyleCrosses
    };

    QgsLayoutGridSettings( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutGridSettings.
%End

    virtual QString stringType() const;
    virtual QgsLayout *layout();


    void setResolution( const QgsLayoutMeasurement &resolution );
%Docstring
 Sets the page/snap grid ``resolution``.
.. seealso:: resolution()
.. seealso:: setOffset()
%End

    QgsLayoutMeasurement resolution() const;
%Docstring
 Returns the page/snap grid resolution.
.. seealso:: setResolution()
.. seealso:: offset()
 :rtype: QgsLayoutMeasurement
%End

    void setOffset( const QgsLayoutPoint offset );
%Docstring
 Sets the ``offset`` of the page/snap grid.
.. seealso:: offset()
.. seealso:: setResolution()
%End

    QgsLayoutPoint offset() const;
%Docstring
 Returns the offset of the page/snap grid.
.. seealso:: setOffset()
.. seealso:: resolution()
 :rtype: QgsLayoutPoint
%End

    void setPen( const QPen &pen );
%Docstring
 Sets the ``pen`` used for drawing page/snap grids.
.. seealso:: pen()
.. seealso:: setStyle()
%End

    QPen pen() const;
%Docstring
 Returns the pen used for drawing page/snap grids.
.. seealso:: setPen()
.. seealso:: style()
 :rtype: QPen
%End

    void setStyle( const Style style );
%Docstring
 Sets the ``style`` used for drawing the page/snap grids.
.. seealso:: style()
.. seealso:: setPen()
%End

    Style style() const;
%Docstring
 Returns the style used for drawing the page/snap grids.
.. seealso:: setStyle()
.. seealso:: pen()
 :rtype: Style
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
 Stores the grid's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.
.. seealso:: readXml()
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &gridElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
 Sets the grid's state from a DOM element. gridElement is the DOM node corresponding to the grid.
.. seealso:: writeXml()
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutgridsettings.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
