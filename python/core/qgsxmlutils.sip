/**
 * Assorted helper methods for reading and writing chunks of XML
 */
class QgsXmlUtils
{
%TypeHeaderCode
#include <qgsxmlutils.h>
%End
  public:

    /* reading */

    static QGis::UnitType readMapUnits( const QDomElement& element );
    static QgsRectangle readRectangle( const QDomElement& element );

    /* writing */

    static QDomElement writeMapUnits( QGis::UnitType units, QDomDocument& doc );
    static QDomElement writeRectangle( const QgsRectangle& rect, QDomDocument& doc );
};
