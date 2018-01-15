/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsxmlutils.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsXmlUtils
{
%Docstring
Assorted helper methods for reading and writing chunks of XML
%End

%TypeHeaderCode
#include "qgsxmlutils.h"
%End
  public:


    static QgsUnitTypes::DistanceUnit readMapUnits( const QDomElement &element );
%Docstring
Decodes a distance unit from a DOM element.

:param element: DOM element to decode

:return: distance units

.. seealso:: :py:func:`writeMapUnits`
%End

    static QgsRectangle readRectangle( const QDomElement &element );


    static QDomElement writeMapUnits( QgsUnitTypes::DistanceUnit units, QDomDocument &doc );
%Docstring
Encodes a distance unit to a DOM element.

:param units: units to encode
:param doc: DOM document

:return: element containing encoded units

.. seealso:: :py:func:`readMapUnits`
%End

    static QDomElement writeRectangle( const QgsRectangle &rect, QDomDocument &doc );

    static QDomElement writeVariant( const QVariant &value, QDomDocument &doc );
%Docstring
Write a QVariant to a QDomElement.

Supported types are

- QVariant.Map
- QVariant.Int
- QVariant.Double
- QVariant.String
%End

    static QVariant readVariant( const QDomElement &element );
%Docstring
Read a QVariant from a QDomElement.
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsxmlutils.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
