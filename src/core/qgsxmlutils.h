#ifndef QGSXMLUTILS_H
#define QGSXMLUTILS_H

class QDomDocument;
class QDomElement;

class QgsRectangle;

#include "qgis.h"

/**
 * Assorted helper methods for reading and writing chunks of XML
 */
class CORE_EXPORT QgsXmlUtils
{
  public:

    /* reading */

    static QGis::UnitType readMapUnits( const QDomElement& element );
    static QgsRectangle readRectangle( const QDomElement& element );

    /* writing */

    static QDomElement writeMapUnits( QGis::UnitType units, QDomDocument& doc );
    static QDomElement writeRectangle( const QgsRectangle& rect, QDomDocument& doc );
};


#endif // QGSXMLUTILS_H
