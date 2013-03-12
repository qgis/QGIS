#ifndef QGSOGCUTILS_H
#define QGSOGCUTILS_H

class QDomNode;
class QDomElement;
class QDomDocument;
class QString;

#include <list>

class QgsGeometry;
class QgsPoint;
class QgsRectangle;

/**
 * @brief The QgsOgcUtils class provides various utility functions for conversion between
 *   OGC (Open Geospatial Consortium) standards and QGIS internal representations.
 *
 * Currently supported standards:
 * - GML2 - Geography Markup Language (import, export)
 *
 * @note added in 2.0
 */
class CORE_EXPORT QgsOgcUtils
{
public:


  /** static method that creates geometry from GML2
   @param XML representation of the geometry. GML elements are expected to be
     in default namespace (<Point>...</Point>) or in "gml" namespace (<gml:Point>...</gml:Point>)
   */
  static QgsGeometry* geometryFromGML2( const QString& xmlString );

  /** static method that creates geometry from GML2
    */
  static QgsGeometry* geometryFromGML2( const QDomNode& geometryNode );

  /** Exports the geometry to mGML2
      @return true in case of success and false else
   */
  static QDomElement geometryToGML2( QgsGeometry* geometry, QDomDocument& doc );

  /** read rectangle from GML2 Box */
  static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );

private:
  /** static method that creates geometry from GML2 Point */
  static QgsGeometry* geometryFromGML2Point( const QDomElement& geometryElement );
  /** static method that creates geometry from GML2 LineString */
  static QgsGeometry* geometryFromGML2LineString( const QDomElement& geometryElement );
  /** static method that creates geometry from GML2 Polygon */
  static QgsGeometry* geometryFromGML2Polygon( const QDomElement& geometryElement );
  /** static method that creates geometry from GML2 MultiPoint */
  static QgsGeometry* geometryFromGML2MultiPoint( const QDomElement& geometryElement );
  /** static method that creates geometry from GML2 MultiLineString */
  static QgsGeometry* geometryFromGML2MultiLineString( const QDomElement& geometryElement );
  /** static method that creates geometry from GML2 MultiPolygon */
  static QgsGeometry* geometryFromGML2MultiPolygon( const QDomElement& geometryElement );
  /** Reads the <gml:coordinates> element and extracts the coordinates as points
     @param coords list where the found coordinates are appended
     @param elem the <gml:coordinates> element
     @return boolean for success*/
  static bool readGML2Coordinates( std::list<QgsPoint>& coords, const QDomElement elem );
};

#endif // QGSOGCUTILS_H
