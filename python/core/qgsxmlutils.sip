/**
 * Assorted helper methods for reading and writing chunks of XML
 */
class QgsXmlUtils
{
%TypeHeaderCode
#include <qgsxmlutils.h>
%End
  public:

    /** Decodes a distance unit from a DOM element.
     * @param element DOM element to decode
     * @returns distance units
     * @see writeMapUnits()
     */
    static QgsUnitTypes::DistanceUnit readMapUnits( const QDomElement& element );

    static QgsRectangle readRectangle( const QDomElement& element );

    /* writing */

    /** Encodes a distance unit to a DOM element.
     * @param units units to encode
     * @param doc DOM document
     * @returns element containing encoded units
     * @see readMapUnits()
     */
    static QDomElement writeMapUnits( QgsUnitTypes::DistanceUnit units, QDomDocument& doc );

    static QDomElement writeRectangle( const QgsRectangle& rect, QDomDocument& doc );
};
