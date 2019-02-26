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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <list>
#include <QVector>

class QgsExpression;
class QgsGeometry;
class QgsPointXY;
class QgsRectangle;
class QgsVectorLayer;

#include "qgsgeometry.h"
#include "qgsexpression.h"
#include "qgsexpressionnode.h"
#include "qgsexpressionnodeimpl.h"
#include "qgssqlstatement.h"

/**
 * \ingroup core
 * \brief The QgsOgcUtils class provides various utility functions for conversion between
 *   OGC (Open Geospatial Consortium) standards and QGIS internal representations.
 *
 * Currently supported standards:
 * - GML2 - Geography Markup Language (import, export)
 */
class CORE_EXPORT QgsOgcUtils
{
  public:

    /**
     *GML version
     */
    enum GMLVersion
    {
      GML_2_1_2,
      GML_3_1_0,
      GML_3_2_1,
    };

    /**
     * Static method that creates geometry from GML
     \param xmlString xml representation of the geometry. GML elements are expected to be
       in default namespace (\verbatim {<Point>...</Point> \endverbatim) or in
       "gml" namespace (\verbatim <gml:Point>...</gml:Point> \endverbatim)
     */
    static QgsGeometry geometryFromGML( const QString &xmlString );

    /**
     * Static method that creates geometry from GML
      */
    static QgsGeometry geometryFromGML( const QDomNode &geometryNode );

    //! Read rectangle from GML2 Box
    static QgsRectangle rectangleFromGMLBox( const QDomNode &boxNode );

    //! Read rectangle from GML3 Envelope
    static QgsRectangle rectangleFromGMLEnvelope( const QDomNode &envelopeNode );

    /**
     * Exports the geometry to GML
        \returns QDomElement
        \since QGIS 2.16
     */
    static QDomElement geometryToGML( const QgsGeometry &geometry, QDomDocument &doc,
                                      QgsOgcUtils::GMLVersion gmlVersion,
                                      const QString &srsName,
                                      bool invertAxisOrientation,
                                      const QString &gmlIdBase,
                                      int precision = 17 );

    /**
     * Exports the geometry to GML2 or GML3
        \returns QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry &geometry, QDomDocument &doc, const QString &format, int precision = 17 );

    /**
     * Exports the geometry to GML2
        \returns QDomElement
     */
    static QDomElement geometryToGML( const QgsGeometry &geometry, QDomDocument &doc, int precision = 17 );

    /**
     * Exports the rectangle to GML2 Box
        \returns QDomElement
     */
    static QDomElement rectangleToGMLBox( QgsRectangle *box, QDomDocument &doc, int precision = 17 );

    /**
     * Exports the rectangle to GML2 Box
        \returns QDomElement
        \since QGIS 2.16
     */
    static QDomElement rectangleToGMLBox( QgsRectangle *box, QDomDocument &doc,
                                          const QString &srsName,
                                          bool invertAxisOrientation,
                                          int precision = 17 );

    /**
     * Exports the rectangle to GML3 Envelope
        \returns QDomElement
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle *env, QDomDocument &doc, int precision = 17 );

    /**
     * Exports the rectangle to GML3 Envelope
        \returns QDomElement
        \since QGIS 2.16
     */
    static QDomElement rectangleToGMLEnvelope( QgsRectangle *env, QDomDocument &doc,
        const QString &srsName,
        bool invertAxisOrientation,
        int precision = 17 );


    //! Parse XML with OGC fill into QColor
    static QColor colorFromOgcFill( const QDomElement &fillElement );

    //! Parse XML with OGC filter into QGIS expression
    static QgsExpression *expressionFromOgcFilter( const QDomElement &element, QgsVectorLayer *layer = nullptr ) SIP_FACTORY;

    /**
     * Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * \returns valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcFilter( const QgsExpression &exp, QDomDocument &doc, QString *errorMessage = nullptr );

    /**
     * OGC filter version
     */
    enum FilterVersion
    {
      FILTER_OGC_1_0,
      FILTER_OGC_1_1,
      FILTER_FES_2_0
    };

    /**
     * Returns an expression from a WFS filter embedded in a document.
     * \param element The WFS Filter
     * \param version The WFS version
     * \param layer Layer to use to retrieve field values from literal filters
     * \since QGIS 3.4
     */
    static QgsExpression *expressionFromOgcFilter( const QDomElement &element, FilterVersion version, QgsVectorLayer *layer = nullptr ) SIP_FACTORY;

    /**
     * Creates OGC filter XML element. Supports minimum standard filter
     * according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
     * \returns valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    static QDomElement expressionToOgcFilter( const QgsExpression &exp,
        QDomDocument &doc,
        QgsOgcUtils::GMLVersion gmlVersion,
        FilterVersion filterVersion,
        const QString &geometryName,
        const QString &srsName,
        bool honourAxisOrientation,
        bool invertAxisOrientation,
        QString *errorMessage = nullptr ) SIP_SKIP;

    /**
     * Creates an OGC expression XML element.
     * \returns valid OGC expression QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcExpression( const QgsExpression &exp, QDomDocument &doc, QString *errorMessage = nullptr );

    /**
     * Creates an OGC expression XML element.
     * \returns valid OGC expression QDomElement on success,
     * otherwise null QDomElement
     */
    static QDomElement expressionToOgcExpression( const QgsExpression &exp,
        QDomDocument &doc,
        QgsOgcUtils::GMLVersion gmlVersion,
        FilterVersion filterVersion,
        const QString &geometryName,
        const QString &srsName,
        bool honourAxisOrientation,
        bool invertAxisOrientation,
        QString *errorMessage = nullptr );

#ifndef SIP_RUN

    /**
     * \ingroup core
     * Layer properties. Used by SQLStatementToOgcFilter().
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    class LayerProperties
    {
      public:
        //! Constructor
        LayerProperties() = default;

        //! Layer name
        QString mName;
        //! Geometry attribute name
        QString mGeometryAttribute;
        //! SRS name
        QString mSRSName;
    };
#endif

    /**
     * Creates OGC filter XML element from the WHERE and JOIN clauses of a SQL
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
     * \returns valid \verbatim <Filter> \endverbatim QDomElement on success,
     * otherwise null QDomElement
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    static QDomElement SQLStatementToOgcFilter( const QgsSQLStatement &statement,
        QDomDocument &doc,
        QgsOgcUtils::GMLVersion gmlVersion,
        FilterVersion filterVersion,
        const QList<LayerProperties> &layerProperties,
        bool honourAxisOrientation,
        bool invertAxisOrientation,
        const QMap< QString, QString> &mapUnprefixedTypenameToPrefixedTypename,
        QString *errorMessage = nullptr ) SIP_SKIP;

  private:

    //! Static method that creates geometry from GML Point
    static QgsGeometry geometryFromGMLPoint( const QDomElement &geometryElement );
    //! Static method that creates geometry from GML LineString
    static QgsGeometry geometryFromGMLLineString( const QDomElement &geometryElement );
    //! Static method that creates geometry from GML Polygon
    static QgsGeometry geometryFromGMLPolygon( const QDomElement &geometryElement );
    //! Static method that creates geometry from GML MultiPoint
    static QgsGeometry geometryFromGMLMultiPoint( const QDomElement &geometryElement );
    //! Static method that creates geometry from GML MultiLineString
    static QgsGeometry geometryFromGMLMultiLineString( const QDomElement &geometryElement );
    //! Static method that creates geometry from GML MultiPolygon
    static QgsGeometry geometryFromGMLMultiPolygon( const QDomElement &geometryElement );

    /**
     * Reads the \verbatim <gml:coordinates> \endverbatim element and extracts the coordinates as points
       \param coords list where the found coordinates are appended
       \param elem the \verbatim <gml:coordinates> \endverbatim element
       \returns boolean for success*/
    static bool readGMLCoordinates( QgsPolylineXY &coords, const QDomElement &elem );

    /**
     * Reads the \verbatim <gml:pos> \endverbatim or \verbatim <gml:posList> \endverbatim
       and extracts the coordinates as points
       \param coords list where the found coordinates are appended
       \param elem the \verbatim <gml:pos> \endverbatim or
                    \verbatim <gml:posList> \endverbatim element
       \returns boolean for success*/
    static bool readGMLPositions( QgsPolylineXY &coords, const QDomElement &elem );


    /**
     * Create a GML coordinates element from a point list.
      \param points list of data points
      \param doc the GML document
      \returns QDomElement */
    static QDomElement createGMLCoordinates( const QgsPolylineXY &points, QDomDocument &doc );

    /**
     * Create a GML pos or posList element from a point list.
      \param points list of data points
      \param doc the GML document
      \returns QDomElement */
    static QDomElement createGMLPositions( const QgsPolylineXY &points, QDomDocument &doc );

    //! handle a generic sub-expression
    static QgsExpressionNode *nodeFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer = nullptr );
    //! handle a generic binary operator
    static QgsExpressionNodeBinaryOperator *nodeBinaryOperatorFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer = nullptr );
    //! handles various spatial operation tags (\verbatim <Intersects> \endverbatim, \verbatim <Touches> \endverbatim etc.)
    static QgsExpressionNodeFunction *nodeSpatialOperatorFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handle \verbatim <Not> \endverbatim tag
    static QgsExpressionNodeUnaryOperator *nodeNotFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <Function> \endverbatim tag
    static QgsExpressionNodeFunction *nodeFunctionFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <Literal> \endverbatim tag
    static QgsExpressionNode *nodeLiteralFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer = nullptr );
    //! handles \verbatim <PropertyName> \endverbatim tag
    static QgsExpressionNodeColumnRef *nodeColumnRefFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <PropertyIsBetween> \endverbatim tag
    static QgsExpressionNode *nodeIsBetweenFromOgcFilter( QDomElement &element, QString &errorMessage );
    //! handles \verbatim <PropertyIsNull> \endverbatim tag
    static QgsExpressionNodeBinaryOperator *nodePropertyIsNullFromOgcFilter( QDomElement &element, QString &errorMessage );
};

#ifndef SIP_RUN

/**
 * \ingroup core
 * Internal use by QgsOgcUtils
 * \note not available in Python bindings
 */
class QgsOgcUtilsExprToFilter
{
  public:
    //! Constructor
    QgsOgcUtilsExprToFilter( QDomDocument &doc,
                             QgsOgcUtils::GMLVersion gmlVersion,
                             QgsOgcUtils::FilterVersion filterVersion,
                             const QString &geometryName,
                             const QString &srsName,
                             bool honourAxisOrientation,
                             bool invertAxisOrientation );

    //! Convert an expression to a OGC filter
    QDomElement expressionNodeToOgcFilter( const QgsExpressionNode *node, QgsExpression *expression, const QgsExpressionContext *context );

    //! Returns whether the gml: namespace is used
    bool GMLNamespaceUsed() const { return mGMLUsed; }

    //! Returns the error message.
    QString errorMessage() const { return mErrorMessage; }

  private:
    QDomDocument &mDoc;
    bool mGMLUsed;
    QgsOgcUtils::GMLVersion mGMLVersion;
    QgsOgcUtils::FilterVersion mFilterVersion;
    const QString &mGeometryName;
    const QString &mSrsName;
    bool mInvertAxisOrientation;
    QString mErrorMessage;
    QString mFilterPrefix;
    QString mPropertyName;
    int mGeomId;

    QDomElement expressionUnaryOperatorToOgcFilter( const QgsExpressionNodeUnaryOperator *node, QgsExpression *expression, const QgsExpressionContext *context );
    QDomElement expressionBinaryOperatorToOgcFilter( const QgsExpressionNodeBinaryOperator *node, QgsExpression *expression, const QgsExpressionContext *context );
    QDomElement expressionLiteralToOgcFilter( const QgsExpressionNodeLiteral *node, QgsExpression *expression, const QgsExpressionContext *context );
    QDomElement expressionColumnRefToOgcFilter( const QgsExpressionNodeColumnRef *node, QgsExpression *expression, const QgsExpressionContext *context );
    QDomElement expressionInOperatorToOgcFilter( const QgsExpressionNodeInOperator *node, QgsExpression *expression, const QgsExpressionContext *context );
    QDomElement expressionFunctionToOgcFilter( const QgsExpressionNodeFunction *node, QgsExpression *expression, const QgsExpressionContext *context );
};

/**
 * \ingroup core
 * \brief Internal use by QgsOgcUtils
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class QgsOgcUtilsExpressionFromFilter
{
  public:

    /**
     * Constructor for QgsOgcUtilsExpressionFromFilter.
     * \param version WFS Version
     * \param layer Layer to use to retrieve field values from literal filters
     */
    QgsOgcUtilsExpressionFromFilter( QgsOgcUtils::FilterVersion version = QgsOgcUtils::FILTER_OGC_1_0,
                                     const QgsVectorLayer *layer = nullptr );

    /**
     * Returns an expression node from a WFS filter embedded in a document
     * element. NULLPTR is returned when an error happened.
     * \param element The WFS filter
     */
    QgsExpressionNode *nodeFromOgcFilter( const QDomElement &element );

    /**
     * Returns the underlying error message, or an empty string in case of no
     * error.
     */
    QString errorMessage() const;

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * binary operators.
     *
     */
    QgsExpressionNodeBinaryOperator *nodeBinaryOperatorFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * spatial operators.
     */
    QgsExpressionNodeFunction *nodeSpatialOperatorFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * column references.
     */
    QgsExpressionNodeColumnRef *nodeColumnRefFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * literal tag.
     */
    QgsExpressionNode *nodeLiteralFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * Not operator.
     */
    QgsExpressionNodeUnaryOperator *nodeNotFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * IsNull operator.
     */
    QgsExpressionNodeBinaryOperator *nodePropertyIsNullFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * functions.
     */
    QgsExpressionNodeFunction *nodeFunctionFromOgcFilter( const QDomElement &element );

    /**
     * Returns an expression node from a WFS filter embedded in a document with
     * boudnaries operator.
     */
    QgsExpressionNode *nodeIsBetweenFromOgcFilter( const QDomElement &element );

  private:
    const QgsVectorLayer *mLayer = nullptr;
    QString mErrorMessage;
    QString mPropertyName;
    QString mPrefix;
};

/**
 * \ingroup core
 * Internal use by QgsOgcUtils
 * \note not available in Python bindings
 */
class QgsOgcUtilsSQLStatementToFilter
{
  public:
    //! Constructor
    QgsOgcUtilsSQLStatementToFilter( QDomDocument &doc,
                                     QgsOgcUtils::GMLVersion gmlVersion,
                                     QgsOgcUtils::FilterVersion filterVersion,
                                     const QList<QgsOgcUtils::LayerProperties> &layerProperties,
                                     bool honourAxisOrientation,
                                     bool invertAxisOrientation,
                                     const QMap< QString, QString> &mapUnprefixedTypenameToPrefixedTypename );

    //! Convert a SQL statement to a OGC filter
    QDomElement toOgcFilter( const QgsSQLStatement::Node *node );

    //! Returns whether the gml: namespace is used
    bool GMLNamespaceUsed() const { return mGMLUsed; }

    //! Returns the error message.
    QString errorMessage() const { return mErrorMessage; }

  private:
    QDomDocument &mDoc;
    bool mGMLUsed;
    QgsOgcUtils::GMLVersion mGMLVersion;
    QgsOgcUtils::FilterVersion mFilterVersion;
    const QList<QgsOgcUtils::LayerProperties> &mLayerProperties;
    bool mHonourAxisOrientation;
    bool mInvertAxisOrientation;
    QString mErrorMessage;
    QString mFilterPrefix;
    QString mPropertyName;
    int mGeomId;
    QString mCurrentSRSName;
    QMap<QString, QString> mMapTableAliasToNames;
    const QMap< QString, QString> &mMapUnprefixedTypenameToPrefixedTypename;

    QDomElement toOgcFilter( const QgsSQLStatement::NodeUnaryOperator *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeBinaryOperator *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeLiteral *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeColumnRef *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeInOperator *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeBetweenOperator *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeFunction *node );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeJoin *node, const QString &leftTable );
    QDomElement toOgcFilter( const QgsSQLStatement::NodeSelect *node );

    void visit( const QgsSQLStatement::NodeTableDef *node );
    QString getGeometryColumnSRSName( const QgsSQLStatement::Node *node );
    bool processSRSName( const QgsSQLStatement::NodeFunction *mainNode,
                         QList<QgsSQLStatement::Node *> args,
                         bool lastArgIsSRSName,
                         QString &srsName,
                         bool &axisInversion );
};
#endif // #ifndef SIP_RUN

#endif // QGSOGCUTILS_H
