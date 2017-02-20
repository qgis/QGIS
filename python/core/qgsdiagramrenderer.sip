/** \ingroup core
 * \class QgsDiagramLayerSettings
 * \brief Stores the settings for rendering of all diagrams for a layer.
 *
 * QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
 * QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
 */

class QgsDiagramLayerSettings
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End

  public:
    //avoid inclusion of QgsPalLabeling
    enum Placement
    {
      AroundPoint, // Point / Polygon
      OverPoint, // Point / Polygon
      Line, // Line / Polygon
      Curved, // Line
      Horizontal, // Polygon
      Free // Polygon
    };

    //! Line placement flags for controlling line based placements
    enum LinePlacementFlag
    {
      OnLine,
      AboveLine,
      BelowLine,
      MapOrientation,
    };
    typedef QFlags<QgsDiagramLayerSettings::LinePlacementFlag> LinePlacementFlags;

    /** Data definable properties.
     * @note added in QGIS 2.16
     */
    enum Property
    {
      BackgroundColor, /*!< Diagram background color */
      OutlineColor, /*!< Outline color */
      OutlineWidth, /*!< Outline width */
      PositionX, /*! x-coordinate data defined diagram position */
      PositionY, /*! y-coordinate data defined diagram position */
      Distance, /*! Distance to diagram from feature */
      Priority, /*! Diagram priority (between 0 and 10) */
      ZIndex, /*! Z-index for diagram ordering */
      IsObstacle, /*! Whether diagram features act as obstacles for other diagrams/labels */
      Show, /*! Whether to show the diagram */
      AlwaysShow, /*! Whether the diagram should always be shown, even if it overlaps other diagrams/labels */
      StartAngle, /*! Angle offset for pie diagram */
    };

    /**
     * Returns the diagram property definitions.
     * @note added in QGIS 3.0
     */
    static const QgsPropertiesDefinition& propertyDefinitions();

    QgsDiagramLayerSettings();

    //! Copy constructor
    QgsDiagramLayerSettings( const QgsDiagramLayerSettings& rh );

    ~QgsDiagramLayerSettings();

    /** Returns the diagram placement.
     * @see setPlacement()
     * @note added in QGIS 2.16
     */
    Placement placement() const;

    /** Sets the diagram placement.
     * @param value placement value
     * @see getPlacement()
     * @note added in QGIS 2.16
     */
    void setPlacement( Placement value );

    /** Returns the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * @see setLinePlacementFlags()
     * @note added in QGIS 2.16
     */
    LinePlacementFlags linePlacementFlags() const;

    /** Sets the the diagram placement flags. These are only used if the diagram placement
     * is set to a line type.
     * @param flags placement value
     * @see getPlacement()
     * @note added in QGIS 2.16
     */
    void setLinePlacementFlags( LinePlacementFlags flags );

    /** Returns the diagram priority.
     * @returns diagram priority, where 0 = low and 10 = high
     * @note placement priority is shared with labeling, so diagrams with a high priority may displace labels
     * and vice-versa
     * @see setPriority()
     * @note added in QGIS 2.16
     */
    int priority() const;

    /** Sets the diagram priority.
     * @param value priority, where 0 = low and 10 = high
     * @see getPriority()
     * @note added in QGIS 2.16
     */
    void setPriority( int value );

    /** Returns the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * @note z-index ordering is shared with labeling, so diagrams with a high z-index may be drawn over labels
     * with a low z-index and vice-versa
     * @see setZIndex()
     * @note added in QGIS 2.16
     */
    double zIndex() const;

    /** Sets the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
     * with a lower z-index.
     * @param index diagram z-index
     * @see getZIndex()
     * @note added in QGIS 2.16
     */
    void setZIndex( double index );

    /** Returns whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * @see setIsObstacle()
     * @note added in QGIS 2.16
     */
    bool isObstacle() const;

    /** Sets whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.
     * @param isObstacle set to true for feature to act as obstacle
     * @see isObstacle()
     * @note added in QGIS 2.16
     */
    void setIsObstacle( bool isObstacle );

    /** Returns the distance between the diagram and the feature (in mm).
     * @see setDistance()
     * @note added in QGIS 2.16
     */
    double distance() const;

    /** Sets the distance between the diagram and the feature.
     * @param distance distance in mm
     * @see distance()
     * @note added in QGIS 2.16
     */
    void setDistance( double distance );

    /** Returns the diagram renderer associated with the layer.
     * @see setRenderer()
     * @note added in QGIS 2.16
     */
    QgsDiagramRenderer* renderer();

    /** Returns the diagram renderer associated with the layer.
     * @see setRenderer()
     * @note added in QGIS 2.16
     */
    //const QgsDiagramRenderer* renderer() const;

    /** Sets the diagram renderer associated with the layer.
     * @param diagramRenderer diagram renderer. Ownership is transferred to the object.
     * @see renderer()
     * @note added in QGIS 2.16
     */
    void setRenderer( QgsDiagramRenderer* diagramRenderer /Transfer/ );

    /** Returns the coordinate transform associated with the layer, or an
     * invalid transform if no transformation is required.
     * @see setCoordinateTransform()
     * @note added in QGIS 2.16
     */
    QgsCoordinateTransform coordinateTransform() const;

    /** Sets the coordinate transform associated with the layer.
     * @param transform coordinate transform. Ownership is transferred to the object.
     * @see coordinateTransform()
     * @note added in QGIS 2.16
     */
    void setCoordinateTransform( const QgsCoordinateTransform& transform );

    /** Returns whether the layer should show all diagrams, including overlapping diagrams
     * @see setShowAllDiagrams()
     * @note added in QGIS 2.16
     */
    bool showAllDiagrams() const;

    /** Sets whether the layer should show all diagrams, including overlapping diagrams
     * @param showAllDiagrams set to true to show all diagrams
     * @see showAllDiagrams()
     * @note added in QGIS 2.16
     */
    void setShowAllDiagrams( bool showAllDiagrams );

    void readXml( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
    bool prepare( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    /** Returns the set of any fields referenced by the layer's diagrams.
     * @param context expression context the diagrams will be drawn using
     * @note added in QGIS 2.16
     */
    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    QgsPropertyCollection& dataDefinedProperties();

    //const dataDefinedProperties& properties() const;

    void setDataDefinedProperties( const QgsPropertyCollection& collection );


};

/** \ingroup core
 * \class QgsDiagramSettings
 * \brief Stores the settings for rendering a single diagram.
 *
 * QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
 * QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
 */

class QgsDiagramSettings
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End
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

    QgsDiagramSettings();
    bool enabled;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    //! @note added in 2.10
    QList< QString > categoryLabels;
    QSizeF size; //size

    /** Diagram size unit
     */
    QgsUnitTypes::RenderUnit sizeType;

    /** Diagram size unit scale
     * @note added in 2.16
     */
    QgsMapUnitScale sizeScale;

    /** Line unit index
     * @note added in 2.16
     */
    QgsUnitTypes::RenderUnit lineSizeUnit;

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

    void readXml( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXml( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    /** Returns list of legend nodes for the diagram
     * @note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * @note added in 2.10
     */
    QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

};

//additional diagram settings for interpolated size rendering
class QgsDiagramInterpolationSettings
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End
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


/** \ingroup core
 * \class QgsDiagramRenderer
 * \brief Evaluates and returns the diagram settings relating to a diagram for a specific feature.
 */

class QgsDiagramRenderer
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End
  public:

    QgsDiagramRenderer();
    virtual ~QgsDiagramRenderer();

    /** Returns new instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagramRenderer* clone() const = 0 /Factory/;

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
    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, QPointF pos, const QgsPropertyCollection& properties = QgsPropertyCollection() );

    void setDiagram( QgsDiagram* d /Transfer/ );
    QgsDiagram* diagram() const;

    /** Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXml( const QDomElement& elem, const QgsVectorLayer* layer ) = 0;
    virtual void writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const = 0;

    /** Returns list of legend nodes for the diagram
     * @note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * @note added in 2.10
     */
    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

    /** Returns true if renderer will show legend items for diagram attributes.
     * @note added in QGIS 2.16
     * @see setAttributeLegend()
     * @see sizeLegend()
     */
    bool attributeLegend() const;

    /** Sets whether the renderer will show legend items for diagram attributes.
     * @param enabled set to true to show diagram attribute legend
     * @note added in QGIS 2.16
     * @see attributeLegend()
     * @see setSizeLegend()
     */
    void setAttributeLegend( bool enabled );

    /** Returns true if renderer will show legend items for diagram sizes.
     * @note added in QGIS 2.16
     * @see setSizeLegend()
     * @see attributeLegend()
     * @see sizeLegendSymbol()
     */
    bool sizeLegend() const;

    /** Sets whether the renderer will show legend items for diagram sizes.
     * @param enabled set to true to show diagram size legend
     * @note added in QGIS 2.16
     * @see sizeLegend()
     * @see setAttributeLegend()
     * @see setSizeLegendSymbol()
     */
    void setSizeLegend( bool enabled );

    /** Returns the marker symbol used for rendering the diagram size legend.
     * @note added in QGIS 2.16
     * @see setSizeLegendSymbol()
     * @see sizeLegend()
     */
    QgsMarkerSymbol* sizeLegendSymbol() const;

    /** Sets the marker symbol used for rendering the diagram size legend.
     * @param symbol marker symbol, ownership is transferred to the renderer.
     * @note added in QGIS 2.16
     * @see sizeLegendSymbol()
     * @see setSizeLegend()
     */
    void setSizeLegendSymbol( QgsMarkerSymbol* symbol /Transfer/ );

  protected:
    QgsDiagramRenderer( const QgsDiagramRenderer& other );
    //QgsDiagramRenderer& operator=( const QgsDiagramRenderer& other );

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
    void _readXml( const QDomElement& elem, const QgsVectorLayer* layer );
    void _writeXml( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
};

/** Renders the diagrams for all features with the same settings*/
class QgsSingleCategoryDiagramRenderer : QgsDiagramRenderer
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End

  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    virtual QgsSingleCategoryDiagramRenderer* clone() const /Factory/;

    QString rendererName() const;

    QList<QString> diagramAttributes() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<QgsDiagramSettings> diagramSettings() const;

    void readXml( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) const;
};

class QgsLinearlyInterpolatedDiagramRenderer : QgsDiagramRenderer
{
%TypeHeaderCode
#include <qgsdiagramrenderer.h>
%End

  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    virtual QgsLinearlyInterpolatedDiagramRenderer* clone() const /Factory/;

    /** Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<QString> diagramAttributes() const;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    QString rendererName() const;

    void setLowerValue( double val );
    double lowerValue() const;

    void setUpperValue( double val );
    double upperValue() const;

    void setLowerSize( QSizeF s );
    QSizeF lowerSize() const;

    void setUpperSize( QSizeF s );
    QSizeF upperSize() const;

    /**
     * Returns the field name used for interpolating the diagram size.
     * @see setClassificationField()
     * @note added in QGIS 3.0
     */
    QString classificationField() const;

    /**
     * Sets the field name used for interpolating the diagram size.
     * @see classificationField()
     * @note added in QGIS 3.0
     */
    void setClassificationField( const QString& field );

    QString classificationAttributeExpression() const;
    void setClassificationAttributeExpression( const QString& expression );

    bool classificationAttributeIsExpression() const;
    void setClassificationAttributeIsExpression( bool isExpression );

    void readXml( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXml( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) const;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) const;
};
