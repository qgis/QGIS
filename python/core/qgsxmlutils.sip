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

    static Qgis::UnitType readMapUnits( const QDomElement& element );
    static QgsRectangle readRectangle( const QDomElement& element );

    /* writing */

    static QDomElement writeMapUnits( Qgis::UnitType units, QDomDocument& doc );
    static QDomElement writeRectangle( const QgsRectangle& rect, QDomDocument& doc );
};
