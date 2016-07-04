/***************************************************************************
    qgsdiagramrendererv2.h
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDIAGRAMRENDERERV2_H
#define QGSDIAGRAMRENDERERV2_H

#include <QColor>
#include <QFont>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QDomDocument>

#include "qgsfeature.h"
#include "qgsexpressioncontext.h"
#include "qgssymbollayerv2.h"

class QgsDiagram;
class QgsDiagramRendererV2;
class QgsFeature;
class QgsRenderContext;
class QDomElement;
class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsVectorLayer;
class QgsLayerTreeModelLegendNode;
class QgsLayerTreeLayer;

namespace pal { class Layer; }

/** \ingroup core
 * \class QgsDiagramLayerSettings
 * \brief Stores the settings for rendering of all diagrams for a layer.
 *
 * QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
 * QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
 */

class CORE_EXPORT QgsDiagramLayerSettings
{
  public:

    //avoid inclusion of QgsPalLabeling
    enum Placement
    {
      AroundPoint = 0, // Point / Polygon
      OverPoint, // Point / Polygon
      Line, // Line / Polygon
      Curved, // Line
      Horizontal, // Polygon
      Free // Polygon
    };

    //! Line placement flags for controlling line based placements
    enum LinePlacementFlags
    {
      OnLine    = 1,
      AboveLine = 2,
      BelowLine = 4,
      MapOrientation = 8
    };

    QgsDiagramLayerSettings();

    //! Copy constructor
    QgsDiagramLayerSettings( const QgsDiagramLayerSettings& rh );

    QgsDiagramLayerSettings& operator=( const QgsDiagramLayerSettings& rh );

    ~QgsDiagramLayerSettings();

    /** Returns the diagram placement.
     * @see setPlacement()
     * @note added in QGIS 2.16
     */
    //TODO QGIS 3.0 - rename getter to placement()
    Placement getPlacement() const { return placement; }

    /** Sets the diagram placement.
     * @param value placement value
     * @see getPlacement()
     * @note added in QGIS 2.16
     */
    void setPlacement( Placement value ) { placement = value; }

    //! Diagram placement
    //TODO QGIS 3.0 - make private, rename to mPlacement
    Placement placement;

    /** Returns the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * @see setLinePlacementFlags()
     * @note added in QGIS 2.16
     */
    unsigned int linePlacementFlags() const { return placementFlags; }

    /** Sets the the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * @param flags placement value
     * @see getPlacement()
     * @note added in QGIS 2.16
     */
    void setLinePlacementFlags( unsigned int flags ) { placementFlags = flags; }

    //! Diagram placement flags
    // TODO QGIS 3.0 - make private, rename to mPlacementFlags, use QFlags
    unsigned int placementFlags;

    /** Returns the diagram priority.
     * @returns diagram priority, where 0 = low and 10 = high
     * @note placement priority is shared with labeling, so diagrams with a high priority may displace labels
     * and vice-versa
     * @see setPriority()
     * @note added in QGIS 2.16
     */
    //TODO QGIS 3.0 - rename getter to priority()
    int getPriority() const { return priority; }

    /** Sets the diagram priority.
     * @param value priority, where 0 = low and 10 = high
     * @see getPriority()
     * @note added in QGIS 2.16
     */
    void setPriority( int value ) { priority = value; }

    //! Placement priority, where 0 = low and 10 = high
    //! @note placement priority is shared with labeling, so diagrams with a high priority may displace labels
    //! and vice-versa
    // TODO QGIS 3.0 - make private, rename to mPriority
    int priority;

    /** Returns the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * @note z-index ordering is shared with labeling, so diagrams with a high z-index may be drawn over labels
     * with a low z-index and vice-versa
     * @see setZIndex()
     * @note added in QGIS 2.16
     */
    //TODO QGIS 3.0 - rename getter to zIndex()
    double getZIndex() const { return zIndex; }

    /** Sets the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * @param index diagram z-index
     * @see getZIndex()
     * @note added in QGIS 2.16
     */
    void setZIndex( double index ) { zIndex = index; }

    //! Z-index of diagrams, where diagrams with a higher z-index are drawn on top of diagrams with a lower z-index
    // TODO QGIS 3.0 - rename to mZIndex, make private
    double zIndex;


    /** Returns whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * @see setIsObstacle()
     * @note added in QGIS 2.16
     */
    bool isObstacle() const { return obstacle; }

    /** Sets whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * @param isObstacle set to true for feature to act as obstacle
     * @see isObstacle()
     * @note added in QGIS 2.16
     */
    void setIsObstacle( bool isObstacle ) { obstacle = isObstacle; }

    //! Whether associated feature acts as an obstacle for other labels or diagrams
    // TODO QGIS 3.0 - rename to mObstacle, make private
    bool obstacle;

    /** Returns the distance between the diagram and the feature (in mm).
     * @see setDistance()
     * @note added in QGIS 2.16
     */
    double distance() const { return dist; }

    /** Sets the distance between the diagram and the feature.
     * @param distance distance in mm
     * @see distance()
     * @note added in QGIS 2.16
     */
    void setDistance( double distance ) { dist = distance; }

    //! Distance between diagram and the feature (in mm)
    // TODO QGIS 3.0 - make private, rename to mDistance
    double dist;

    /** Returns the diagram renderer associated with the layer.
     * @see setRenderer()
     * @note added in QGIS 2.16
     */
    // TODO QGIS 3.0 - rename to renderer()
    QgsDiagramRendererV2* getRenderer() { return renderer; }

    /** Returns the diagram renderer associated with the layer.
     * @see setRenderer()
     * @note added in QGIS 2.16
     */
    // TODO QGIS 3.0 - rename to renderer()
    const QgsDiagramRendererV2* getRenderer() const { return renderer; }

    /** Sets the diagram renderer associated with the layer.
     * @param diagramRenderer diagram renderer. Ownership is transferred to the object.
     * @see getRenderer()
     * @note added in QGIS 2.16
     */
    void setRenderer( QgsDiagramRendererV2* diagramRenderer );

    //! Associated diagram renderer. Owned by this object.
    // TODO QGIS 3.0 - make private, rename to mRenderer
    QgsDiagramRendererV2* renderer;

    /** Returns the coordinate transform associated with the layer.
     * @see setCoordinateTransform()
     * @note added in QGIS 2.16
     */
    QgsCoordinateTransform* coordinateTransform() { return ct; }

    /** Returns the coordinate transform associated with the layer.
     * @see setCoordinateTransform()
     * @note added in QGIS 2.16
     */
    const QgsCoordinateTransform* coordinateTransform() const { return ct; }

    /** Sets the coordinate transform associated with the layer.
     * @param transform coordinate transform. Ownership is transferred to the object.
     * @see coordinateTransform()
     * @note added in QGIS 2.16
     */
    void setCoordinateTransform( QgsCoordinateTransform* transform );

    //! Associated coordinate transform. Owned by this object.
    // TODO QGIS 3.0 - make private, rename to mCt
    QgsCoordinateTransform* ct;

    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED const QgsMapToPixel* xform;

    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED QgsFields fields;

    //! Attribute index for x coordinate (or -1 if position not data defined)
    int xPosColumn;

    //! Attribute index for y coordinate (or -1 if position not data defined)
    int yPosColumn;

    //! Attribute index for visibility (or -1 if visibility not data defined)
    int showColumn;

    /** Returns whether the layer should show all diagrams, including overlapping diagrams
     * @see setShowAllDiagrams()
     * @note added in QGIS 2.16
     */
    bool showAllDiagrams() const { return showAll; }

    /** Sets whether the layer should show all diagrams, including overlapping diagrams
     * @param showAllDiagrams set to true to show all diagrams
     * @see showAllDiagrams()
     * @note added in QGIS 2.16
     */
    void setShowAllDiagrams( bool showAllDiagrams ) { showAll = showAllDiagrams; }

    //! Whether to show all diagrams, including overlapping diagrams
    // TODO QGIS 3.0 - make private, rename to mShowAll
    bool showAll;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    /** Returns the set of any fields referenced by the layer's diagrams.
     * @param context expression context the diagrams will be drawn using
     * @param fields layer fields
     * @note added in QGIS 2.16
     */
    //TODO QGIS 3.0 - remove need for fields parameter
    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext(), const QgsFields& fields = QgsFields() ) const;

};

/** \ingroup core
 * \class QgsDiagramSettings
 * \brief Stores the settings for rendering a single diagram.
 *
 * QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
 * QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
 */

class CORE_EXPORT QgsDiagramSettings
{
  public:

    //! @deprecated use QgsSymbolV2::OutputUnit instead
    enum SizeType
    {
      MM,
      MapUnits
    };

    enum LabelPlacementMethod
    {
      Height,
      XHeight
    };

    //! Orientation of histogram
    enum DiagramOrientation
    {
      Up,
      Down,
      Left,
      Right
    };

    QgsDiagramSettings()
        : enabled( true )
        , sizeType( QgsSymbolV2::MM )
        , lineSizeUnit( QgsSymbolV2::MM )
        , penWidth( 0.0 )
        , labelPlacementMethod( QgsDiagramSettings::Height )
        , diagramOrientation( QgsDiagramSettings::Up )
        , barWidth( 5.0 )
        , transparency( 0 )
        , scaleByArea( true )
        , angleOffset( 90 * 16 ) //top
        , scaleBasedVisibility( false )
        , minScaleDenominator( -1 )
        , maxScaleDenominator( -1 )
        , minimumSize( 0.0 )
    {}
    bool enabled;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    //! @note added in 2.10
    QList< QString > categoryLabels;
    QSizeF size; //size

    /** Diagram size unit
     */
    QgsSymbolV2::OutputUnit sizeType;

    /** Diagram size unit scale
     * @note added in 2.16
     */
    QgsMapUnitScale sizeScale;

    /** Line unit index
     * @note added in 2.16
     */
    QgsSymbolV2::OutputUnit lineSizeUnit;

    /** Line unit scale
     * @note added in 2.16
     */
    QgsMapUnitScale lineSizeScale;

    QColor backgroundColor;
    QColor penColor;
    double penWidth;
    LabelPlacementMethod labelPlacementMethod;
    DiagramOrientation diagramOrientation;
    double barWidth;
    int transparency; // 0 - 100
    bool scaleByArea;
    int angleOffset;

    bool scaleBasedVisibility;
    //scale range (-1 if no lower / upper bound )
    double minScaleDenominator;
    double maxScaleDenominator;

    //! Scale diagrams smaller than mMinimumSize to mMinimumSize
    double minimumSize;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    /** Returns list of legend nodes for the diagram
     * @note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * @note added in 2.10
     */
    QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const;

};

/** \ingroup core
 * \class QgsDiagramInterpolationSettings
 * Additional diagram settings for interpolated size rendering.
 */
class CORE_EXPORT QgsDiagramInterpolationSettings
{
  public:
    QSizeF lowerSize;
    QSizeF upperSize;
    double lowerValue;
    double upperValue;

    /** Index of the classification attribute*/
    //TODO QGIS 3.0 - don't store index, store field name
    int classificationAttribute;

    QString classificationAttributeExpression;
    bool classificationAttributeIsExpression;
};


/** \ingroup core
 * \class QgsDiagramRendererV2
 * \brief Evaluates and returns the diagram settings relating to a diagram for a specific feature.
 */

class CORE_EXPORT QgsDiagramRendererV2
{
  public:

    QgsDiagramRendererV2();
    virtual ~QgsDiagramRendererV2();

    /** Returns new instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagramRendererV2* clone() const = 0;

    /** Returns size of the diagram for a feature in map units. Returns an invalid QSizeF in case of error*/
    virtual QSizeF sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c ) const;

    virtual QString rendererName() const = 0;

    /** Returns attribute indices needed for diagram rendering*/
    virtual QList<QString> diagramAttributes() const = 0;

    /** Returns the set of any fields required for diagram rendering
     * @param context expression context the diagrams will be drawn using
     * @param fields layer fields
     * @note added in QGIS 2.16
     */
    //TODO QGIS 3.0 - remove need for fields parameter
    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext(), const QgsFields& fields = QgsFields() ) const;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, QPointF pos ) const;

    void setDiagram( QgsDiagram* d );
    QgsDiagram* diagram() const { return mDiagram; }

    /** Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) = 0;
    virtual void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const = 0;

    /** Returns list of legend nodes for the diagram
     * @note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * @note added in 2.10
     */
    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const;

    /** Returns true if renderer will show legend items for diagram attributes.
     * @note added in QGIS 2.16
     * @see setAttributeLegend()
     * @see sizeLegend()
     */
    bool attributeLegend() const { return mShowAttributeLegend; }

    /** Sets whether the renderer will show legend items for diagram attributes.
     * @param enabled set to true to show diagram attribute legend
     * @note added in QGIS 2.16
     * @see attributeLegend()
     * @see setSizeLegend()
     */
    void setAttributeLegend( bool enabled ) { mShowAttributeLegend = enabled; }

    /** Returns true if renderer will show legend items for diagram sizes.
     * @note added in QGIS 2.16
     * @see setSizeLegend()
     * @see attributeLegend()
     * @see sizeLegendSymbol()
     */
    bool sizeLegend() const { return mShowSizeLegend; }

    /** Sets whether the renderer will show legend items for diagram sizes.
     * @param enabled set to true to show diagram size legend
     * @note added in QGIS 2.16
     * @see sizeLegend()
     * @see setAttributeLegend()
     * @see setSizeLegendSymbol()
     */
    void setSizeLegend( bool enabled ) { mShowSizeLegend = enabled; }

    /** Returns the marker symbol used for rendering the diagram size legend.
     * @note added in QGIS 2.16
     * @see setSizeLegendSymbol()
     * @see sizeLegend()
     */
    QgsMarkerSymbolV2* sizeLegendSymbol() const { return mSizeLegendSymbol.data(); }

    /** Sets the marker symbol used for rendering the diagram size legend.
     * @param symbol marker symbol, ownership is transferred to the renderer.
     * @note added in QGIS 2.16
     * @see sizeLegendSymbol()
     * @see setSizeLegend()
     */
    void setSizeLegendSymbol( QgsMarkerSymbolV2* symbol ) { mSizeLegendSymbol.reset( symbol ); }

  protected:
    QgsDiagramRendererV2( const QgsDiagramRendererV2& other );
    QgsDiagramRendererV2& operator=( const QgsDiagramRendererV2& other );

    /** Returns diagram settings for a feature (or false if the diagram for the feature is not to be rendered). Used internally within renderDiagram()
     * @param feature the feature
     * @param c render context
     * @param s out: diagram settings for the feature
     */
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const = 0;

    /** Returns size of the diagram (in painter units) or an invalid size in case of error*/
    virtual QSizeF diagramSize( const QgsFeature& features, const QgsRenderContext& c ) const = 0;

    /** Converts size from mm to map units*/
    void convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const;

    /** Returns the paint device dpi (or -1 in case of error*/
    static int dpiPaintDevice( const QPainter* );

    //read / write diagram
    void _readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void _writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    /** Reference to the object that does the real diagram rendering*/
    QgsDiagram* mDiagram;

    //! Whether to show an attribute legend for the diagrams
    bool mShowAttributeLegend;

    //! Whether to show a size legend for the diagrams
    bool mShowSizeLegend;

    //! Marker symbol to use in size legends
    QScopedPointer< QgsMarkerSymbolV2 > mSizeLegendSymbol;
};

/** \ingroup core
 * Renders the diagrams for all features with the same settings
*/
class CORE_EXPORT QgsSingleCategoryDiagramRenderer : public QgsDiagramRendererV2
{
  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    QgsSingleCategoryDiagramRenderer* clone() const override;

    QString rendererName() const override { return "SingleCategory"; }

    QList<QString> diagramAttributes() const override { return mSettings.categoryAttributes; }

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<QgsDiagramSettings> diagramSettings() const override;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) override;
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const override;

    QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const override;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const override;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) const override;

  private:
    QgsDiagramSettings mSettings;
};

/** \ingroup core
 * \class QgsLinearlyInterpolatedDiagramRenderer
 */
class CORE_EXPORT QgsLinearlyInterpolatedDiagramRenderer : public QgsDiagramRendererV2
{
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    QgsLinearlyInterpolatedDiagramRenderer* clone() const override;

    /** Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const override;

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<QString> diagramAttributes() const override;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext(), const QgsFields& fields = QgsFields() ) const override;

    QString rendererName() const override { return "LinearlyInterpolated"; }

    void setLowerValue( double val ) { mInterpolationSettings.lowerValue = val; }
    double lowerValue() const { return mInterpolationSettings.lowerValue; }

    void setUpperValue( double val ) { mInterpolationSettings.upperValue = val; }
    double upperValue() const { return mInterpolationSettings.upperValue; }

    void setLowerSize( QSizeF s ) { mInterpolationSettings.lowerSize = s; }
    QSizeF lowerSize() const { return mInterpolationSettings.lowerSize; }

    void setUpperSize( QSizeF s ) { mInterpolationSettings.upperSize = s; }
    QSizeF upperSize() const { return mInterpolationSettings.upperSize; }

    int classificationAttribute() const { return mInterpolationSettings.classificationAttribute; }
    void setClassificationAttribute( int index ) { mInterpolationSettings.classificationAttribute = index; }

    QString classificationAttributeExpression() const { return mInterpolationSettings.classificationAttributeExpression; }
    void setClassificationAttributeExpression( const QString& expression ) { mInterpolationSettings.classificationAttributeExpression = expression; }

    bool classificationAttributeIsExpression() const { return mInterpolationSettings.classificationAttributeIsExpression; }
    void setClassificationAttributeIsExpression( bool isExpression ) { mInterpolationSettings.classificationAttributeIsExpression = isExpression; }

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) override;
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const override;

    QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const override;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const override;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) const override;

  private:
    QgsDiagramSettings mSettings;
    QgsDiagramInterpolationSettings mInterpolationSettings;
};

#endif // QGSDIAGRAMRENDERERV2_H
