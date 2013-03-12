

class QgsOgcUtils
{
%TypeHeaderCode
#include <qgsogcutils.h>
%End

public:

  /** static method that creates geometry from GML2
   @param XML representation of the geometry. GML elements are expected to be
     in default namespace (<Point>...</Point>) or in "gml" namespace (<gml:Point>...</gml:Point>)
   @note added in 1.9
   */
  static QgsGeometry* geometryFromGML2( const QString& xmlString ) /Factory/;

  /** static method that creates geometry from GML2
    @note added in 1.9
    */
  static QgsGeometry* geometryFromGML2( const QDomNode& geometryNode ) /Factory/;

  /** Exports the geometry to mGML2
      @return true in case of success and false else
   */
  static QDomElement geometryToGML2( QgsGeometry* geometry, QDomDocument& doc );

  /** read rectangle from GML2 Box */
  static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );


  /** static method that creates geometry from GML3
   @param XML representation of the geometry. GML elements are expected to be
     in default namespace (<Point>...</Point>) or in "gml" namespace (<gml:Point>...</gml:Point>)
   */
  static QgsGeometry* geometryFromGML3( const QString& xmlString );

  /** static method that creates geometry from GML2
    */
  static QgsGeometry* geometryFromGML3( const QDomNode& geometryNode );
  /** Exports the geometry to mGML3
      @return true in case of success and false else
   */
  static QDomElement geometryToGML3( QgsGeometry* geometry, QDomDocument& doc );

  /** read rectangle from GML3 Envelope */
  static QgsRectangle rectangleFromGMLEnvelope( const QDomNode& envelopeNode );
};

