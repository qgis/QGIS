/***************************************************************************
    qgsdiagramrenderer.h
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
#ifndef QGSDIAGRAMRENDERER_H
#define QGSDIAGRAMRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QColor>
#include <QFont>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QDomDocument>

#include "qgsexpressioncontext.h"
#include "qgsfields.h"
#include "qgscoordinatetransform.h"
#include "qgsproperty.h"
#include "qgspropertycollection.h"

#include "diagram/qgsdiagram.h"
#include "qgsreadwritecontext.h"
#include "qgsmapunitscale.h"

class QgsDiagramRenderer;
class QgsFeature;
class QgsRenderContext;
class QDomElement;
class QgsMapToPixel;
class QgsReadWriteContext;
class QgsVectorLayer;
class QgsLayerTreeModelLegendNode;
class QgsLayerTreeLayer;
class QgsPaintEffect;
class QgsDataDefinedSizeLegend;
class QgsLineSymbol;

namespace pal { class Layer; } SIP_SKIP

/**
 * \ingroup core
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
    enum LinePlacementFlag
    {
      OnLine    = 1,
      AboveLine = 1 << 1,
      BelowLine = 1 << 2,
      MapOrientation = 1 << 4,
    };
    Q_DECLARE_FLAGS( LinePlacementFlags, LinePlacementFlag )

    /**
     * Data definable properties.
     * \since QGIS 3.0
     */
    enum Property
    {
      BackgroundColor, //!< Diagram background color
      StrokeColor, //!< Stroke color
      StrokeWidth, //!< Stroke width
      PositionX, //!< X-coordinate data defined diagram position
      PositionY, //!< Y-coordinate data defined diagram position
      Distance, //!< Distance to diagram from feature
      Priority, //!< Diagram priority (between 0 and 10)
      ZIndex, //!< Z-index for diagram ordering
      IsObstacle, //!< Whether diagram features act as obstacles for other diagrams/labels
      Show, //!< Whether to show the diagram
      AlwaysShow, //!< Whether the diagram should always be shown, even if it overlaps other diagrams/labels
      StartAngle, //!< Angle offset for pie diagram
    };

    /**
     * Returns the diagram property definitions.
     * \since QGIS 3.0
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    /**
     * Constructor for QgsDiagramLayerSettings.
     */
    QgsDiagramLayerSettings();

    //! Copy constructor
    QgsDiagramLayerSettings( const QgsDiagramLayerSettings &rh );

    QgsDiagramLayerSettings &operator=( const QgsDiagramLayerSettings &rh );

    ~QgsDiagramLayerSettings();

    /**
     * Returns the diagram placement.
     * \see setPlacement()
     * \since QGIS 2.16
     */
    Placement placement() const { return mPlacement; }

    /**
     * Sets the diagram placement.
     * \param value placement value
     * \see placement()
     * \since QGIS 2.16
     */
    void setPlacement( Placement value ) { mPlacement = value; }

    /**
     * Returns the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * \see setLinePlacementFlags()
     * \since QGIS 2.16
     */
    LinePlacementFlags linePlacementFlags() const { return mPlacementFlags; }

    /**
     * Sets the the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * \param flags placement value
     * \see linePlacementFlags()
     * \since QGIS 2.16
     */
    void setLinePlacementFlags( LinePlacementFlags flags ) { mPlacementFlags = flags; }

    /**
     * Returns the diagram priority.
     * \returns diagram priority, where 0 = low and 10 = high
     * \note placement priority is shared with labeling, so diagrams with a high priority may displace labels
     * and vice-versa
     * \see setPriority()
     * \since QGIS 2.16
     */
    int priority() const { return mPriority; }

    /**
     * Sets the diagram priority.
     * \param value priority, where 0 = low and 10 = high
     * \see priority()
     * \since QGIS 2.16
     */
    void setPriority( int value ) { mPriority = value; }

    /**
     * Returns the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * \note z-index ordering is shared with labeling, so diagrams with a high z-index may be drawn over labels
     * with a low z-index and vice-versa
     * \see setZIndex()
     * \since QGIS 2.16
     */
    double zIndex() const { return mZIndex; }

    /**
     * Sets the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * \param index diagram z-index
     * \see zIndex()
     * \since QGIS 2.16
     */
    void setZIndex( double index ) { mZIndex = index; }

    /**
     * Returns whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * \see setIsObstacle()
     * \since QGIS 2.16
     */
    bool isObstacle() const { return mObstacle; }

    /**
     * Sets whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * \param isObstacle set to TRUE for feature to act as obstacle
     * \see isObstacle()
     * \since QGIS 2.16
     */
    void setIsObstacle( bool isObstacle ) { mObstacle = isObstacle; }

    /**
     * Returns the distance between the diagram and the feature (in mm).
     * \see setDistance()
     * \since QGIS 2.16
     */
    double distance() const { return mDistance; }

    /**
     * Sets the distance between the diagram and the feature.
     * \param distance distance in mm
     * \see distance()
     * \since QGIS 2.16
     */
    void setDistance( double distance ) { mDistance = distance; }

    /**
     * Returns the diagram renderer associated with the layer.
     * \see setRenderer()
     * \since QGIS 2.16
     */
    QgsDiagramRenderer *renderer() { return mRenderer; }

    /**
     * Returns the diagram renderer associated with the layer.
     * \see setRenderer()
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    const QgsDiagramRenderer *renderer() const { return mRenderer; } SIP_SKIP

    /**
     * Sets the diagram renderer associated with the layer.
     * \param diagramRenderer diagram renderer. Ownership is transferred to the object.
     * \see renderer()
     * \since QGIS 2.16
     */
    void setRenderer( QgsDiagramRenderer *diagramRenderer SIP_TRANSFER );

    /**
     * Returns the coordinate transform associated with the layer, or an
     * invalid transform if no transformation is required.
     * \see setCoordinateTransform()
     * \since QGIS 2.16
     */
    QgsCoordinateTransform coordinateTransform() const { return mCt; }

    /**
     * Sets the coordinate transform associated with the layer.
     * \param transform coordinate transform. Ownership is transferred to the object.
     * \see coordinateTransform()
     * \since QGIS 2.16
     */
    void setCoordinateTransform( const QgsCoordinateTransform &transform );

    /**
     * Returns whether the layer should show all diagrams, including overlapping diagrams
     * \see setShowAllDiagrams()
     * \since QGIS 2.16
     */
    bool showAllDiagrams() const { return mShowAll; }

    /**
     * Sets whether the layer should show all diagrams, including overlapping diagrams
     * \param showAllDiagrams set to TRUE to show all diagrams
     * \see showAllDiagrams()
     * \since QGIS 2.16
     */
    void setShowAllDiagrams( bool showAllDiagrams ) { mShowAll = showAllDiagrams; }

    /**
     * Reads the diagram settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem );

    /**
     * Writes the diagram settings to a DOM element.
     * \see readXml()
     */
    void writeXml( QDomElement &layerElem, QDomDocument &doc ) const;

    /**
     * Prepares the diagrams for a specified expression context. Calling prepare before rendering
     * multiple diagrams allows precalculation of expensive setup tasks such as parsing expressions.
     * Returns TRUE if preparation was successful.
     * \since QGIS 3.0
     */
    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    /**
     * Returns the set of any fields referenced by the layer's diagrams.
     * \param context expression context the diagrams will be drawn using
     * \since QGIS 2.16
     */
    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    /**
     * Returns a reference to the diagram's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.0
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the diagram's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    /**
     * Sets the diagram's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.0
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

  private:

    //! Associated coordinate transform, or invalid transform for no transformation
    QgsCoordinateTransform mCt;

    //! Diagram placement
    Placement mPlacement = AroundPoint;

    //! Diagram placement flags
    LinePlacementFlags mPlacementFlags = OnLine;

    /**
     * Placement priority, where 0 = low and 10 = high
     * \note placement priority is shared with labeling, so diagrams with a high priority may displace labels
     * and vice-versa
     */
    int mPriority = 5;

    //! Z-index of diagrams, where diagrams with a higher z-index are drawn on top of diagrams with a lower z-index
    double mZIndex = 0.0;

    //! Whether associated feature acts as an obstacle for other labels or diagrams
    bool mObstacle = false;

    //! Distance between diagram and the feature (in mm)
    double mDistance = 0.0;

    //! Associated diagram renderer. Owned by this object.
    QgsDiagramRenderer *mRenderer = nullptr;

    //! Whether to show all diagrams, including overlapping diagrams
    bool mShowAll = true;

    //! Property collection for data defined diagram settings
    QgsPropertyCollection mDataDefinedProperties;

    static void initPropertyDefinitions();

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

};

/**
 * \ingroup core
 * \class QgsDiagramSettings
 * \brief Stores the settings for rendering a single diagram.
 *
 * QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
 * QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
 */

class CORE_EXPORT QgsDiagramSettings
{
  public:

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

    /**
     * Angular directions.
     * \since QGIS 3.12
     */
    enum Direction
    {
      Clockwise, //!< Clockwise orientation
      Counterclockwise, //!< Counter-clockwise orientation
    };

    //! Constructor for QgsDiagramSettings
    QgsDiagramSettings();
    ~QgsDiagramSettings();

    //! Copy constructor
    QgsDiagramSettings( const QgsDiagramSettings &other );

    QgsDiagramSettings &operator=( const QgsDiagramSettings &other );

    bool enabled = true;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    //! \since QGIS 2.10
    QList< QString > categoryLabels;
    QSizeF size; //size

    /**
     * Diagram size unit
     */
    QgsUnitTypes::RenderUnit sizeType = QgsUnitTypes::RenderMillimeters;

    /**
     * Diagram size unit scale
     * \since QGIS 2.16
     */
    QgsMapUnitScale sizeScale;

    /**
     * Line unit index
     * \since QGIS 2.16
     */
    QgsUnitTypes::RenderUnit lineSizeUnit = QgsUnitTypes::RenderMillimeters;

    /**
     * Line unit scale
     * \since QGIS 2.16
     */
    QgsMapUnitScale lineSizeScale;

    QColor backgroundColor;
    QColor penColor;
    double penWidth = 0.0;
    LabelPlacementMethod labelPlacementMethod = QgsDiagramSettings::Height;
    DiagramOrientation diagramOrientation = QgsDiagramSettings::Up;
    double barWidth = 5.0;

    //! Opacity, from 0 (transparent) to 1.0 (opaque)
    double opacity = 1.0;

    bool scaleByArea = true;

    /**
     * Rotation offset, in degrees clockwise from horizontal.
     * \since QGIS 3.0
     */
    double rotationOffset = 270;

    bool scaleBasedVisibility = false;

    /**
     * The maximum map scale (i.e. most "zoomed in" scale) at which the diagrams will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     * \see minimumScale
    */
    double maximumScale = 0;

    /**
     * The minimum map scale (i.e. most "zoomed out" scale) at which the diagrams will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     * \see maximumScale
    */
    double minimumScale = 0;

    //! Scale diagrams smaller than mMinimumSize to mMinimumSize
    double minimumSize = 0.0;

    /**
     * Returns the spacing between diagram contents.
     *
     * Spacing units can be retrieved by calling spacingUnit().
     *
     * \see setSpacing()
     * \see spacingUnit()
     * \see spacingMapUnitScale()
     *
     * \since QGIS 3.12
     */
    double spacing() const { return mSpacing; }

    /**
     * Sets the \a spacing between diagram contents.
     *
     * Spacing units are set via setSpacingUnit().
     *
     * \see spacing()
     * \see setSpacingUnit()
     * \see setSpacingMapUnitScale()
     *
     * \since QGIS 3.12
     */
    void setSpacing( double spacing ) { mSpacing = spacing; }

    /**
     * Sets the \a unit for the content spacing.
     * \see spacingUnit()
     * \see setSpacing()
     * \see setSpacingMapUnitScale()
     *
     * \since QGIS 3.12
    */
    void setSpacingUnit( QgsUnitTypes::RenderUnit unit ) { mSpacingUnit = unit; }

    /**
     * Returns the units for the content spacing.
     * \see setSpacingUnit()
     * \see spacing()
     * \see spacingMapUnitScale()
     * \since QGIS 3.12
    */
    QgsUnitTypes::RenderUnit spacingUnit() const { return mSpacingUnit; }

    /**
     * Sets the map unit \a scale for the content spacing.
     * \see spacingMapUnitScale()
     * \see setSpacing()
     * \see setSpacingUnit()
     *
     * \since QGIS 3.12
    */
    void setSpacingMapUnitScale( const QgsMapUnitScale &scale ) { mSpacingMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the content spacing.
     * \see setSpacingMapUnitScale()
     * \see spacing()
     * \see spacingUnit()
     *
     * \since QGIS 3.12
    */
    const QgsMapUnitScale &spacingMapUnitScale() const { return mSpacingMapUnitScale; }

    /**
     * Returns the chart's angular direction.
     *
     * \see setDirection()
     * \since QGIS 3.12
     */
    Direction direction() const;

    /**
     * Sets the chart's angular \a direction.
     *
     * \see direction()
     * \since QGIS 3.12
     */
    void setDirection( Direction direction );

    //! Reads diagram settings from XML
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );
    //! Writes diagram settings to XML
    void writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;

    /**
     * Returns list of legend nodes for the diagram
     * \note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * \since QGIS 2.10
     */
    QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const SIP_FACTORY;

    /**
     * Returns the line symbol to use for rendering axis in diagrams.
     *
     * \see setAxisLineSymbol()
     * \see showAxis()
     *
     * \since QGIS 3.12
     */
    QgsLineSymbol *axisLineSymbol() const;

    /**
     * Sets the line \a symbol to use for rendering axis in diagrams.
     *
     * Ownership of \a symbol is transferred to the settings.
     *
     * \see axisLineSymbol()
     * \see setShowAxis()
     *
     * \since QGIS 3.12
     */
    void setAxisLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns TRUE if the diagram axis should be shown.
     *
     * \see setShowAxis()
     * \see axisLineSymbol()
     *
     * \since QGIS 3.12
     */
    bool showAxis() const;

    /**
     * Sets whether the diagram axis should be shown.
     *
     * \see showAxis()
     * \see setAxisLineSymbol()
     *
     * \since QGIS 3.12
     */
    void setShowAxis( bool showAxis );

    /**
     * Returns the paint effect to use while rendering diagrams.
     *
     * \see setPaintEffect()
     *
     * \since QGIS 3.12
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the paint \a effect to use while rendering diagrams.
     *
     * Ownership of \a effect is transferred to the settings.
     *
     * \see paintEffect()
     *
     * \since QGIS 3.12
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

  private:

    double mSpacing = 0;
    QgsUnitTypes::RenderUnit mSpacingUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSpacingMapUnitScale;
    Direction mDirection = Counterclockwise;

    bool mShowAxis = false;
    std::unique_ptr< QgsLineSymbol > mAxisLineSymbol;
    std::unique_ptr< QgsPaintEffect > mPaintEffect;

};

/**
 * \ingroup core
 * \class QgsDiagramInterpolationSettings
 * \brief Additional diagram settings for interpolated size rendering.
 */
class CORE_EXPORT QgsDiagramInterpolationSettings
{
  public:
    QSizeF lowerSize;
    QSizeF upperSize;
    double lowerValue;
    double upperValue;

    //! Name of the field for classification
    QString classificationField;

    QString classificationAttributeExpression;
    bool classificationAttributeIsExpression;
};


/**
 * \ingroup core
 * \class QgsDiagramRenderer
 * \brief Evaluates and returns the diagram settings relating to a diagram for a specific feature.
 */

class CORE_EXPORT QgsDiagramRenderer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->rendererName() == QLatin1String( "SingleCategory" ) )
      sipType = sipType_QgsSingleCategoryDiagramRenderer;
    else if ( sipCpp->rendererName() == QLatin1String( "LinearlyInterpolated" ) )
      sipType = sipType_QgsLinearlyInterpolatedDiagramRenderer;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsDiagramRenderer.
     */
    QgsDiagramRenderer() = default;
    virtual ~QgsDiagramRenderer() = default;

    /**
     * Returns new instance that is equivalent to this one
     * \since QGIS 2.4
    */
    virtual QgsDiagramRenderer *clone() const = 0 SIP_FACTORY;

    //! Returns size of the diagram for a feature in map units. Returns an invalid QSizeF in case of error
    virtual QSizeF sizeMapUnits( const QgsFeature &feature, const QgsRenderContext &c ) const;

    virtual QString rendererName() const = 0;

    //! Returns attribute indices needed for diagram rendering
    virtual QList<QString> diagramAttributes() const = 0;

    /**
     * Returns the set of any fields required for diagram rendering
     * \param context expression context the diagrams will be drawn using
     * \since QGIS 2.16
     */
    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;

    /**
     * Renders the diagram for a specified feature at a specific position in the passed render context.
     */
    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, QPointF pos, const QgsPropertyCollection &properties = QgsPropertyCollection() ) const;

    void setDiagram( QgsDiagram *d SIP_TRANSFER );
    QgsDiagram *diagram() const { return mDiagram.get(); }

    //! Returns list with all diagram settings in the renderer
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    /**
     * Reads diagram state from a DOM element. Subclasses should ensure that _readXml() is called
     * by their readXml implementation to restore the general QgsDiagramRenderer settings.
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;

    /**
     * Writes diagram state to a DOM element. Subclasses should ensure that _writeXml() is called
     * by their writeXml implementation to save the general QgsDiagramRenderer settings.
     * \see readXml()
     */
    virtual void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    /**
     * Returns list of legend nodes for the diagram
     * \note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * \since QGIS 2.10
     */
    virtual QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const SIP_FACTORY;

    /**
     * Returns TRUE if renderer will show legend items for diagram attributes.
     * \see setAttributeLegend()
     * \since QGIS 2.16
     */
    bool attributeLegend() const { return mShowAttributeLegend; }

    /**
     * Sets whether the renderer will show legend items for diagram attributes.
     * \param enabled set to TRUE to show diagram attribute legend
     * \see attributeLegend()
     * \since QGIS 2.16
     */
    void setAttributeLegend( bool enabled ) { mShowAttributeLegend = enabled; }

  protected:
    QgsDiagramRenderer( const QgsDiagramRenderer &other );
    QgsDiagramRenderer &operator=( const QgsDiagramRenderer &other );

    /**
     * Returns diagram settings for a feature (or FALSE if the diagram for the feature is not to be rendered). Used internally within renderDiagram()
     * \param feature the feature
     * \param c render context
     * \param s out: diagram settings for the feature
     */
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const = 0;

    //! Returns size of the diagram (in painter units) or an invalid size in case of error
    virtual QSizeF diagramSize( const QgsFeature &features, const QgsRenderContext &c ) const = 0;

    //! Converts size from mm to map units
    void convertSizeToMapUnits( QSizeF &size, const QgsRenderContext &context ) const;

    //! Returns the paint device dpi (or -1 in case of error
    static int dpiPaintDevice( const QPainter * );

    //read / write diagram

    /**
     * Reads internal QgsDiagramRenderer state from a DOM element.
     * \see _writeXml()
     */
    void _readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Writes internal QgsDiagramRenderer diagram state to a DOM element.
     * \see _readXml()
     */
    void _writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    //! Reference to the object that does the real diagram rendering
    std::unique_ptr< QgsDiagram > mDiagram;

    //! Whether to show an attribute legend for the diagrams
    bool mShowAttributeLegend = true;
};

/**
 * \ingroup core
 * \brief Renders the diagrams for all features with the same settings
*/
class CORE_EXPORT QgsSingleCategoryDiagramRenderer : public QgsDiagramRenderer
{
  public:

    //! Constructor for QgsSingleCategoryDiagramRenderer
    QgsSingleCategoryDiagramRenderer() = default;

    QgsSingleCategoryDiagramRenderer *clone() const override SIP_FACTORY;

    QString rendererName() const override { return QStringLiteral( "SingleCategory" ); }

    QList<QString> diagramAttributes() const override { return mSettings.categoryAttributes; }

    void setDiagramSettings( const QgsDiagramSettings &s ) { mSettings = s; }

    QList<QgsDiagramSettings> diagramSettings() const override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const override SIP_FACTORY;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const override;

    QSizeF diagramSize( const QgsFeature &, const QgsRenderContext &c ) const override;

  private:
    QgsDiagramSettings mSettings;
};

/**
 * \ingroup core
 * \class QgsLinearlyInterpolatedDiagramRenderer
 */
class CORE_EXPORT QgsLinearlyInterpolatedDiagramRenderer : public QgsDiagramRenderer
{
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer() override;
    //! Copy constructor
    QgsLinearlyInterpolatedDiagramRenderer( const QgsLinearlyInterpolatedDiagramRenderer &other );

    QgsLinearlyInterpolatedDiagramRenderer &operator=( const QgsLinearlyInterpolatedDiagramRenderer &other );

    QgsLinearlyInterpolatedDiagramRenderer *clone() const override SIP_FACTORY;

    //! Returns list with all diagram settings in the renderer
    QList<QgsDiagramSettings> diagramSettings() const override;

    void setDiagramSettings( const QgsDiagramSettings &s ) { mSettings = s; }

    QList<QString> diagramAttributes() const override;

    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const override;

    QString rendererName() const override { return QStringLiteral( "LinearlyInterpolated" ); }

    void setLowerValue( double val ) { mInterpolationSettings.lowerValue = val; }
    double lowerValue() const { return mInterpolationSettings.lowerValue; }

    void setUpperValue( double val ) { mInterpolationSettings.upperValue = val; }
    double upperValue() const { return mInterpolationSettings.upperValue; }

    void setLowerSize( QSizeF s ) { mInterpolationSettings.lowerSize = s; }
    QSizeF lowerSize() const { return mInterpolationSettings.lowerSize; }

    void setUpperSize( QSizeF s ) { mInterpolationSettings.upperSize = s; }
    QSizeF upperSize() const { return mInterpolationSettings.upperSize; }

    /**
     * Returns the field name used for interpolating the diagram size.
     * \see setClassificationField()
     * \since QGIS 3.0
     */
    QString classificationField() const { return mInterpolationSettings.classificationField; }

    /**
     * Sets the field name used for interpolating the diagram size.
     * \see classificationField()
     * \since QGIS 3.0
     */
    void setClassificationField( const QString &field ) { mInterpolationSettings.classificationField = field; }

    QString classificationAttributeExpression() const { return mInterpolationSettings.classificationAttributeExpression; }
    void setClassificationAttributeExpression( const QString &expression ) { mInterpolationSettings.classificationAttributeExpression = expression; }

    bool classificationAttributeIsExpression() const { return mInterpolationSettings.classificationAttributeIsExpression; }
    void setClassificationAttributeIsExpression( bool isExpression ) { mInterpolationSettings.classificationAttributeIsExpression = isExpression; }

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const override SIP_FACTORY;

    /**
     * Configures appearance of legend. Takes ownership of the passed settings objects.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend. Will return NULLPTR if no configuration has been set.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const override;

    QSizeF diagramSize( const QgsFeature &, const QgsRenderContext &c ) const override;

  private:
    QgsDiagramSettings mSettings;
    QgsDiagramInterpolationSettings mInterpolationSettings;

    //! Stores more settings about how legend for varying size of symbols should be rendered
    QgsDataDefinedSizeLegend *mDataDefinedSizeLegend = nullptr;
};

#endif // QGSDIAGRAMRENDERER_H
