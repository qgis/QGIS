

class QgsOgcUtils
{
%TypeHeaderCode
#include <qgsogcutils.h>
%End

public:

  /** static method that creates geometry from GML
   @param XML representation of the geometry. GML elements are expected to be
     in default namespace (<Point>...</Point>) or in "gml" namespace (<gml:Point>...</gml:Point>)
   @note added in 1.9
   */
  static QgsGeometry* geometryFromGML( const QString& xmlString ) /Factory/;

  /** static method that creates geometry from GML
    @note added in 1.9
    */
  static QgsGeometry* geometryFromGML( const QDomNode& geometryNode ) /Factory/;

  /** read rectangle from GML2 Box */
  static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );

  /** read rectangle from GML3 Envelope */
  static QgsRectangle rectangleFromGMLEnvelope( const QDomNode& envelopeNode );

  /** Exports the geometry to GML2
      @return QDomElement
   */
  static QDomElement geometryToGML( QgsGeometry* geometry, QDomDocument& doc );

  /** Exports the geometry to GML2 or GML3
      @return QDomElement
   */
  static QDomElement geometryToGML( QgsGeometry* geometry, QDomDocument& doc, QString Format );
};

