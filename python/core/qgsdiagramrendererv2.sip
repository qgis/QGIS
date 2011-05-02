struct QgsDiagramLayerSettings
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End

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
    OnLine    = 1,
    AboveLine = 2,
    BelowLine = 4,
    MapOrientation = 8
  };

  QgsDiagramLayerSettings();

  //pal placement properties
  Placement placement;
  LinePlacementFlags placementFlags;
  int priority; // 0 = low, 10 = high
  bool obstacle; // whether it's an obstacle
  double dist; // distance from the feature (in mm)
  QgsDiagramRendererV2* renderer;
  int xPosColumn; //attribute index for x coordinate (or -1 if position not data defined)
  int yPosColumn;//attribute index for y coordinate (or -1 if position not data defined)

  void readXML( const QDomElement& elem );
  void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;
};

//diagram settings for rendering
struct QgsDiagramSettings
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  enum SizeType
  {
    MM,
    MapUnits
  };

  QgsDiagramSettings();
  QFont font;
  QList< QColor > categoryColors;
  QList< int > categoryIndices;
  QSizeF size; //size
  SizeType sizeType; //mm or map units
  QColor backgroundColor;
  QColor penColor;
  double penWidth;

  //scale range (-1 if no lower / upper bound )
  double minScaleDenominator;
  double maxScaleDenominator;

  void readXML( const QDomElement& elem );
  void writeXML( QDomElement& rendererElem, QDomDocument& doc ) const;
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

    /**Returns size of the diagram for feature f in map units. Returns an invalid QSizeF in case of error*/
    virtual QSizeF sizeMapUnits( const QgsAttributeMap& attributes, const QgsRenderContext& c );

    virtual QString rendererName() const = 0;

    /**Returns attribute indices needed for diagram rendering*/
    virtual QList<int> diagramAttributes() const = 0;

    void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QPointF& pos );

    void setDiagram( QgsDiagram* d );
    const QgsDiagram* diagram() const;

    /**Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXML( const QDomElement& elem ) = 0;
    virtual void writeXML( QDomElement& layerElem, QDomDocument& doc ) const = 0;
};

/**Renders the diagrams for all features with the same settings*/
class QgsSingleCategoryDiagramRenderer: QgsDiagramRendererV2
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    QString rendererName() const;

    QList<int> diagramAttributes() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<QgsDiagramSettings> diagramSettings() const;

    void readXML( const QDomElement& elem );
    void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;
};

class QgsLinearlyInterpolatedDiagramRenderer: QgsDiagramRendererV2
{
%TypeHeaderCode
#include <qgsdiagramrendererv2.h>
%End
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    /**Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const;

    void setDiagramSettings( const QgsDiagramSettings& s );

    QList<int> diagramAttributes() const;

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

    void readXML( const QDomElement& elem );
    void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;
};
