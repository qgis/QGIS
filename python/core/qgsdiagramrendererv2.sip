struct QgsDiagramLayerSettings
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
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

    enum LinePlacementFlags
    {
      OnLine,
      AboveLine,
      BelowLine,
      MapOrientation,
    };

    QgsDiagramLayerSettings();

    ~QgsDiagramLayerSettings();

    //pal placement properties
    Placement placement;
    unsigned int placementFlags;
    int priority; // 0 = low, 10 = high
    bool obstacle; // whether it's an obstacle
    double dist; // distance from the feature (in mm)
    QgsDiagramRendererV2* renderer; // if any renderer is assigned, it is owned by this class
    int xPosColumn; //attribute index for x coordinate (or -1 if position not data defined)
    int yPosColumn;//attribute index for y coordinate (or -1 if position not data defined)
    bool showAll;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
};

//diagram settings for rendering
class QgsDiagramSettings
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  public:
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

    QgsDiagramSettings();
    bool enabled;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    //! @note added in 2.10
    QList< QString > categoryLabels;
    QSizeF size; //size
    SizeType sizeType; //mm or map units
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
    QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;
};

//additional diagram settings for interpolated size rendering
class QgsDiagramInterpolationSettings
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  public:
    QSizeF lowerSize;
    QSizeF upperSize;
    double lowerValue;
    double upperValue;
    /**Index of the classification attribute*/
    int classificationAttribute;
    QString classificationAttributeExpression;
    bool classificationAttributeIsExpression;
};

/**Returns diagram settings for a feature*/
class QgsDiagramRendererV2
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  public:

    QgsDiagramRendererV2();
    virtual ~QgsDiagramRendererV2();

    /** Returns new instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagramRendererV2* clone() const = 0;

    /**Returns size of the diagram for a feature in map units. Returns an invalid QSizeF in case of error*/
    virtual QSizeF sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c );

    virtual QString rendererName() const = 0;

    /**Returns attribute indices needed for diagram rendering*/
    virtual QList<QString> diagramAttributes() const = 0;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QPointF& pos );

    void setDiagram( QgsDiagram* d /Transfer/ );
    QgsDiagram* diagram() const;

    /**Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) = 0;
    virtual void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const = 0;

    /** Returns list of legend nodes for the diagram
     * @note caller is responsible for deletion of QgsLayerTreeModelLegendNodes
     * @note added in 2.10
     */
    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

  protected:

    /**Returns diagram settings for a feature (or false if the diagram for the feature is not to be rendered). Used internally within renderDiagram()
     * @param feature the feature
     * @param c render context
     * @param s out: diagram settings for the feature
     */
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) = 0;

    /**Returns size of the diagram (in painter units) or an invalid size in case of error*/
    virtual QSizeF diagramSize( const QgsFeature& features, const QgsRenderContext& c ) = 0;

    /**Converts size from mm to map units*/
    void convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const;

    /**Returns the paint device dpi (or -1 in case of error*/
    static int dpiPaintDevice( const QPainter* );

    //read / write diagram
    void _readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void _writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
};

/**Renders the diagrams for all features with the same settings*/
class QgsSingleCategoryDiagramRenderer : QgsDiagramRendererV2
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End

  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    QgsDiagramRendererV2* clone() const;

    QString rendererName() const;

    QList<QString> diagramAttributes() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<QgsDiagramSettings> diagramSettings() const;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s );

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c );
};

class QgsLinearlyInterpolatedDiagramRenderer : QgsDiagramRendererV2
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End

  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    QgsDiagramRendererV2* clone() const;

    /**Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<QString> diagramAttributes() const;

    QString rendererName() const;

    void setLowerValue( double val );
    double lowerValue() const;

    void setUpperValue( double val );
    double upperValue() const;

    void setLowerSize( QSizeF s );
    QSizeF lowerSize() const;

    void setUpperSize( QSizeF s );
    QSizeF upperSize() const;

    int classificationAttribute() const;
    void setClassificationAttribute( int index );

    QString classificationAttributeExpression() const;
    void setClassificationAttributeExpression( QString expression );

    bool classificationAttributeIsExpression() const;
    void setClassificationAttributeIsExpression( bool isExpression );

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    virtual QList< QgsLayerTreeModelLegendNode* > legendItems( QgsLayerTreeLayer* nodeLayer ) const /Factory/;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s );

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c );
};
