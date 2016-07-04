/***************************************************************************
    qgsogcutils.h
    ---------------------
    begin                : March 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOGCUTILS_H
#define QGSOGCUTILS_H

class QColor;
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

#include "qgsgeometry.h"
#include "qgsexpression.h"
#include "qgssqlstatement.h"

/** \ingroup core
 * @brief The QgsOgcUtils class provides various utility functions for conversion between
 *   OGC (Open Geospatial Consortium) standards and QGIS internal representations.
 *
 * Currently supported standards:
 * - GML2 - Geography Markup Language (import, export)
 */
class CORE_EXPORT QgsOgcUtils
{
  public:

    /** GML version
     *  @note not available in Python bindings
     */
    typedef enum
    {
      GML_2_1_2,
      GML_3_1_0,
      GML_3_2_1,
    } GMLVersion;

    /** Static method that creates geometry from GML
     @param xmlString xml representation of the geometry. GML elements are expected to be
       in default namespace (\verbatim {<Point>...</Point> \endverbatim) or in
       "gml" namespace (\verbatim <gml:Point>...</gml:Point> \endverbatim)
     */
    static QgsGeometry* geometryFromGML( const QString& xmlString );

    /** Static method that creates geometry from GML
      */
    static QgsGeometry* geometryFromGML( const QDomNode& geometryNode );

    /** Read rectangle from GML2 Box */
    static QgsRectangle rectangleFromGMLBox( const QDomNode& boxNode );

    /** Read rectangle from GML3 Envelope */
    static QgsRectangle rectangleFromGMLEnvelope( const QDomNode& envelopeNode );

    /** Exports the geometry to GML
        @return QDomElement
        @note Added in QGIS 2.16
     */
    static QDomElement geometryToGML( const QgsGeometry* geometry, QDomDocument& doc,
                                      GMLVersion gmlVersion,
                                      const QString& srsName,
                                      bool invertAxisOrientation,
                                      const QString& gmlIdBase,
                                      int precision = 17 );

    /** Exports the geometry to GML2 or GML3
        @return QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, const QString& format, int precision = 17 );

    /** Exports the geometry to GML2
        @return QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, int precision = 17 );

    /** Exports the rectangle to GML2 Box
        @return QDomElement
     */
    static QDomElement rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc, int precision = 17 );

    /** Exports the rectangle to GML2 Box
        @return QDomElement
        @note Added in QGIS 2.16
     */
    static QDomElement rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc,
                                          const QString& srsName,
                                          bool invertAxisOrientation,
                                          int precision = 17 );

    /** Exports the rectangle to GML3 Envelope
        @return QDomElement
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc, int precision = 17 );

    /** Exports the rectangle to GML3 Envelope
        @return QDomElement
        @note Added in QGIS 2.16
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc,
        const QString& srsName,
        bool invertAxisOrientation,
        int precision = 17 );


    /** Parse XML with OGC fill into QColor */
    static QColor colorFromOgcFill( const QDomElement& fillElement );

    /** Parse XML with OGC filter into QGIS expression */
    static QgsExpression* expressionFromOgcFilter( const QDomElement& element );

    /** Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * @return valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage = nullptr );

    /** OGC filter version
     * @note not available in Python bindings
     */
    typedef enum
    {
      FILTER_OGC_1_0,
      FILTER_OGC_1_1,
      FILTER_FES_2_0
    } FilterVersion;

    /** Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * @return valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     * @note Added in QGIS 2.16
     * @note not available in Python bindings
     */
    static QDomElement expressionToOgcFilter( const QgsExpression& exp,
        QDomDocument& doc,
        GMLVersion gmlVersion,
        FilterVersion filterVersion,
        const QString& geometryName,
        const QString& srsName,
        bool honourAxisOrientation,
        bool invertAxisOrientation,
        QString* errorMessage = nullptr );

    /** \ingroup core
     * Layer properties. Used by SQLStatementToOgcFilter().
     * @note Added in QGIS 2.16
     * @note not available in Python bindings
     */
    class LayerProperties
    {
      public:
        /** Constructor */
        LayerProperties() {}

        /** Layer name */
        QString mName;
        /** Geometry attribute name */
        QString mGeometryAttribute;
        /** SRS name */
        QString mSRSName;
    };

    /** Creates OGC filter XML element from the WHERE and JOIN clauses of a SQL
     * statement. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT,LIKE,BETWEEN,IN)
     * Supports layer joins.
     * Supports ST_GeometryFromText(wkt[, srid/srsname]),
     *          ST_MakeEnvelope(xmin,ymin,xmax,ymax[, srid/srsname])
     *          ST_GeomFromGML(serialized_gml_string)
     *          BBOX()
     *          ST_Intersects(), ST_Contains(), ST_Crosses(), ST_Equals(),
     *          ST_Disjoint(), ST_Overlaps(), ST_Touches(), ST_Within()
     *          ST_DWithin(), ST_Beyond()
     *          custom functions
     * @return valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     * @note Added in QGIS 2.16
     * @note not available in Python bindings
     */
    static QDomElement SQLStatementToOgcFilter( const QgsSQLStatement& statement,
        QDomDocument& doc,
        GMLVersion gmlVersion,
        FilterVersion filterVersion,
        const QList<LayerProperties>& layerProperties,
        bool honourAxisOrientation,
        bool invertAxisOrientation,
        const QMap< QString, QString>& mapUnprefixedTypenameToPrefixedTypename,
        QString* errorMessage = nullptr );

  private:

    /** Static method that creates geometry from GML Point */
    static QgsGeometry* geometryFromGMLPoint( const QDomElement& geometryElement );
    /** Static method that creates geometry from GML LineString */
    static QgsGeometry* geometryFromGMLLineString( const QDomElement& geometryElement );
    /** Static method that creates geometry from GML Polygon */
    static QgsGeometry* geometryFromGMLPolygon( const QDomElement& geometryElement );
    /** Static method that creates geometry from GML MultiPoint */
    static QgsGeometry* geometryFromGMLMultiPoint( const QDomElement& geometryElement );
    /** Static method that creates geometry from GML MultiLineString */
    static QgsGeometry* geometryFromGMLMultiLineString( const QDomElement& geometryElement );
    /** Static method that creates geometry from GML MultiPolygon */
    static QgsGeometry* geometryFromGMLMultiPolygon( const QDomElement& geometryElement );
    /** Reads the \verbatim <gml:coordinates> \endverbatim element and extracts the coordinates as points
       @param coords list where the found coordinates are appended
       @param elem the \verbatim <gml:coordinates> \endverbatim element
       @return boolean for success*/
    static bool readGMLCoordinates( QgsPolyline &coords, const QDomElement &elem );
    /** Reads the \verbatim <gml:pos> \endverbatim or \verbatim <gml:posList> \endverbatim
       and extracts the coordinates as points
       @param coords list where the found coordinates are appended
       @param elem the \verbatim <gml:pos> \endverbatim or
                    \verbatim <gml:posList> \endverbatim element
       @return boolean for success*/
    static bool readGMLPositions( QgsPolyline &coords, const QDomElement &elem );


    /** Create a GML coordinates element from a point list.
      @param points list of data points
      @param doc the GML document
      @return QDomElement */
    static QDomElement createGMLCoordinates( const QgsPolyline &points, QDomDocument& doc );

    /** Create a GML pos or posList element from a point list.
      @param points list of data points
      @param doc the GML document
      @return QDomElement */
    static QDomElement createGMLPositions( const QgsPolyline &points, QDomDocument& doc );

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
};

/** \ingroup core
 * Internal use by QgsOgcUtils
 * @note not available in Python bindings
 */
class QgsOgcUtilsExprToFilter
{
  public:
    /** Constructor */
    QgsOgcUtilsExprToFilter( QDomDocument& doc,
                             QgsOgcUtils::GMLVersion gmlVersion,
                             QgsOgcUtils::FilterVersion filterVersion,
                             const QString& geometryName,
                             const QString& srsName,
                             bool honourAxisOrientation,
                             bool invertAxisOrientation );

    /** Convert an expression to a OGC filter */
    QDomElement expressionNodeToOgcFilter( const QgsExpression::Node* node );

    /** Return whether the gml: namespace is used */
    bool GMLNamespaceUsed() const { return mGMLUsed; }

    /** Return the error message. */
    const QString& errorMessage() const { return mErrorMessage; }

  private:
    QDomDocument& mDoc;
    bool mGMLUsed;
    QgsOgcUtils::GMLVersion mGMLVersion;
    QgsOgcUtils::FilterVersion mFilterVersion;
    const QString& mGeometryName;
    const QString& mSrsName;
    bool mInvertAxisOrientation;
    QString mErrorMessage;
    QString mFilterPrefix;
    QString mPropertyName;
    int mGeomId;

    QDomElement expressionUnaryOperatorToOgcFilter( const QgsExpression::NodeUnaryOperator* node );
    QDomElement expressionBinaryOperatorToOgcFilter( const QgsExpression::NodeBinaryOperator* node );
    QDomElement expressionLiteralToOgcFilter( const QgsExpression::NodeLiteral* node );
    QDomElement expressionColumnRefToOgcFilter( const QgsExpression::NodeColumnRef* node );
    QDomElement expressionInOperatorToOgcFilter( const QgsExpression::NodeInOperator* node );
    QDomElement expressionFunctionToOgcFilter( const QgsExpression::NodeFunction* node );
};

/** \ingroup core
 * Internal use by QgsOgcUtils
 * @note not available in Python bindings
 */
class QgsOgcUtilsSQLStatementToFilter
{
  public:
    /** Constructor */
    QgsOgcUtilsSQLStatementToFilter( QDomDocument& doc,
                                     QgsOgcUtils::GMLVersion gmlVersion,
                                     QgsOgcUtils::FilterVersion filterVersion,
                                     const QList<QgsOgcUtils::LayerProperties>& layerProperties,
                                     bool honourAxisOrientation,
                                     bool invertAxisOrientation,
                                     const QMap< QString, QString>& mapUnprefixedTypenameToPrefixedTypename );

    /** Convert a SQL statement to a OGC filter */
    QDomElement toOgcFilter( const QgsSQLStatement::Node* node );

    /** Return whether the gml: namespace is used */
    bool GMLNamespaceUsed() const { return mGMLUsed; }

    /** Return the error message. */
    const QString& errorMessage() const { return mErrorMessage; }

  private:
    QDomDocument& mDoc;
    bool mGMLUsed;
    QgsOgcUtils::GMLVersion mGMLVersion;
    QgsOgcUtils::FilterVersion mFilterVersion;
    const QList<QgsOgcUtils::LayerProperties>& mLayerProperties;
    bool mHonourAxisOrientation;
    bool mInvertAxisOrientation;
    QString mErrorMessage;
    QString mFilterPrefix;
    QString mPropertyName;
    int mGeomId;
    QString mCurrentSRSName;
    QMap<QString, QString> mMapTableAliasToNames;
    const QMap< QString, QString>& mMapUnprefixedTypenameToPrefixedTypename;

    QDomElement toOgcFilter( const QgsSQLStatement::NodeUnaryOperator* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeBinaryOperator* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeLiteral* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeColumnRef* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeInOperator* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeBetweenOperator* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeFunction* node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeJoin* node, const QString& leftTable );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeSelect* node );

    void visit( const QgsSQLStatement::NodeTableDef* node );
    QString getGeometryColumnSRSName( const QgsSQLStatement::Node* node );
    bool processSRSName( const QgsSQLStatement::NodeFunction* mainNode,
                         QList<QgsSQLStatement::Node*> args,
                         bool lastArgIsSRSName,
                         QString& srsName,
                         bool& axisInversion );
};

#endif // QGSOGCUTILS_H
