

class QgsOgcUtils
{
%TypeHeaderCode
#include <qgsogcutils.h>
%End

  public:

    /** Static method that creates geometry from GML
     @param xmlString xml representation of the geometry. GML elements are expected to be
       in default namespace (\verbatim {<Point>...</Point> \endverbatim) or in
       "gml" namespace (\verbatim <gml:Point>...</gml:Point> \endverbatim)
     */
    static QgsGeometry* geometryFromGML( const QString& xmlString ) /Factory/;

    /** Static method that creates geometry from GML
      */
    static QgsGeometry* geometryFromGML( const QDomNode& geometryNode ) /Factory/;

    /** Read rectangle from GML2 Box */
    static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );

    /** Read rectangle from GML3 Envelope */
    static QgsRectangle rectangleFromGMLEnvelope( const QDomNode& envelopeNode );

    /** Exports the geometry to GML2 or GML3
        @return QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, QString format, const int &precision = 17 );

    /** Exports the geometry to GML2
        @return QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, const int &precision = 17 );

    /** Exports the rectangle to GML2 Box
        @return QDomElement
     */
    static QDomElement rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc, const int &precision = 17 );

    /** Exports the rectangle to GML2 Envelope
        @return QDomElement
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc, const int &precision = 17 );


    /** Parse XML with OGC fill into QColor */
    static QColor colorFromOgcFill( const QDomElement& fillElement );

    /** Parse XML with OGC filter into QGIS expression */
    static QgsExpression* expressionFromOgcFilter( const QDomElement& element ) /Factory/;

    /** Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * @return valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage /Out/ );

};

