#ifndef QGSOGCUTILS_H
#define QGSOGCUTILS_H

class QDomNode;
class QDomElement;
class QDomDocument;
class QString;

#include <list>
#include <QVector>

class QgsExpression;
class QgsGeometry;
class QgsPoint;
class QgsRectangle;

#include "qgsexpression.h"

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

    /** static method that creates geometry from GML
     @param xmlString xml representation of the geometry. GML elements are expected to be
       in default namespace (\verbatim {<Point>...</Point> \endverbatim) or in
       "gml" namespace (\verbatim <gml:Point>...</gml:Point> \endverbatim)
     */
    static QgsGeometry* geometryFromGML( const QString& xmlString );

    /** static method that creates geometry from GML
      */
    static QgsGeometry* geometryFromGML( const QDomNode& geometryNode );

    /** read rectangle from GML2 Box */
    static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );

    /** read rectangle from GML3 Envelope */
    static QgsRectangle rectangleFromGMLEnvelope( const QDomNode& envelopeNode );

    /** Exports the geometry to GML2 or GML3
        @return QDomELement
     */
    static QDomElement geometryToGML( QgsGeometry* geometry, QDomDocument& doc, QString format );

    /** Exports the geometry to GML2
        @return QDomElement
     */
    static QDomElement geometryToGML( QgsGeometry* geometry, QDomDocument& doc );

    /** Exports the rectangle to GML2 Box
        @return QDomElement
     */
    static QDomElement rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc );

    /** Exports the rectangle to GML2 Envelope
        @return QDomElement
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc );


    /** Parse XML with OGC filter into QGIS expression */
    static QgsExpression* expressionFromOgcFilter( const QDomElement& element );

    /** Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * @return valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage = 0 );

  private:
    /** static method that creates geometry from GML Point */
    static QgsGeometry* geometryFromGMLPoint( const QDomElement& geometryElement );
    /** static method that creates geometry from GML LineString */
    static QgsGeometry* geometryFromGMLLineString( const QDomElement& geometryElement );
    /** static method that creates geometry from GML Polygon */
    static QgsGeometry* geometryFromGMLPolygon( const QDomElement& geometryElement );
    /** static method that creates geometry from GML MultiPoint */
    static QgsGeometry* geometryFromGMLMultiPoint( const QDomElement& geometryElement );
    /** static method that creates geometry from GML MultiLineString */
    static QgsGeometry* geometryFromGMLMultiLineString( const QDomElement& geometryElement );
    /** static method that creates geometry from GML MultiPolygon */
    static QgsGeometry* geometryFromGMLMultiPolygon( const QDomElement& geometryElement );
    /** Reads the \verbatim <gml:coordinates> \endverbatim element and extracts the coordinates as points
       @param coords list where the found coordinates are appended
       @param elem the \verbatim <gml:coordinates> \endverbatim element
       @return boolean for success*/
    static bool readGMLCoordinates( std::list<QgsPoint>& coords, const QDomElement elem );
    /** Reads the \verbatim <gml:pos> \endverbatim or \verbatim <gml:posList> \endverbatim
       and extracts the coordinates as points
       @param coords list where the found coordinates are appended
       @param elem the \verbatim <gml:pos> \endverbatim or
                    \verbatim <gml:posList> \endverbatim element
       @return boolean for success*/
    static bool readGMLPositions( std::list<QgsPoint>& coords, const QDomElement elem );


    /**Create a GML coordinates element from a point list.
      @param points list of data points
      @param doc the GML document
      @return QDomElement */
    static QDomElement createGMLCoordinates( const QVector<QgsPoint> points, QDomDocument& doc );

    /**Create a GML pos or posList element from a point list.
      @param points list of data points
      @param doc the GML document
      @return QDomElement */
    static QDomElement createGMLPositions( const QVector<QgsPoint> points, QDomDocument& doc );

    //! handle a generic sub-expression
    static QgsExpression::Node* nodeFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handle a generic binary operator
    static QgsExpression::NodeBinaryOperator* nodeBinaryOperatorFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles various spatial operation tags (\verbatim <Intersects> \endverbatim, \verbatim <Touches> \endverbatim etc.)
    static QgsExpression::NodeFunction* nodeSpatialOperatorFromOgcFilter( QDomElement& element, QString& errorMessage );
    //! handle \verbatim <Not> \endverbatim tag
    static QgsExpression::NodeUnaryOperator* nodeNotFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <Function> \endverbatim tag
    static QgsExpression::NodeFunction* nodeFunctionFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <Literal> \endverbatim tag
    static QgsExpression::Node* nodeLiteralFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <PropertyName> \endverbatim tag
    static QgsExpression::NodeColumnRef* nodeColumnRefFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <PropertyIsBetween> \endverbatim tag
    static QgsExpression::Node* nodeIsBetweenFromOgcFilter( QDomElement& element, QString& errorMessage );
    //! handles \verbatim <PropertyIsNull> \endverbatim tag
    static QgsExpression::NodeBinaryOperator* nodePropertyIsNullFromOgcFilter( QDomElement& element, QString& errorMessage );

    static QDomElement expressionNodeToOgcFilter( const QgsExpression::Node* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionUnaryOperatorToOgcFilter( const QgsExpression::NodeUnaryOperator* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionBinaryOperatorToOgcFilter( const QgsExpression::NodeBinaryOperator* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionLiteralToOgcFilter( const QgsExpression::NodeLiteral* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionColumnRefToOgcFilter( const QgsExpression::NodeColumnRef* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionInOperatorToOgcFilter( const QgsExpression::NodeInOperator* node, QDomDocument& doc, QString& errorMessage );
    static QDomElement expressionFunctionToOgcFilter( const QgsExpression::NodeFunction* node, QDomDocument& doc, QString& errorMessage );

};

#endif // QGSOGCUTILS_H
