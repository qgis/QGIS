

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

  /** Exports the rectangle to GML2 Box
      @return QDomElement
   */
  static QDomElement rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc );

  /** Exports the rectangle to GML2 Envelope
      @return QDomElement
   */
  static QDomElement rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc );


  /** Parse XML with OGC filter into QGIS expression */
  static QgsExpression* expressionFromOgcFilter( const QDomElement& element ) /Factory/;

  /** Creates OGC filter XML element. Supports minimum standard filter according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
      @return valid <Filter> QDomElement on success, otherwise null QDomElement
   */
  static QDomElement expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage /Out/ );

};

