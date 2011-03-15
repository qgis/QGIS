#ifndef QGSDIAGRAMRENDERERV2_H
#define QGSDIAGRAMRENDERERV2_H

#include <QColor>
#include <QFont>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include "pal/layer.h" //pal::Layer
#include "qgsfeature.h"
#include "qgspallabeling.h"

class QgsDiagram;
class QgsDiagramRendererV2;
class QgsFeature;
class QgsRenderContext;
class QDomElement;

struct QgsDiagramLayerSettings
{
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

  QgsDiagramLayerSettings(): placement( AroundPoint ), placementFlags( OnLine ), priority( 5 ), obstacle( false ), dist( 0.0 ), renderer( 0 ),
      palLayer( 0 ), ct( 0 ), xform( 0 ), xPosColumn( -1 ), yPosColumn( -1 )
  {
  }

  //pal placement properties
  Placement placement;
  LinePlacementFlags placementFlags;
  int priority; // 0 = low, 10 = high
  bool obstacle; // whether it's an obstacle
  double dist; // distance from the feature (in mm)
  QgsDiagramRendererV2* renderer;

  //assigned when layer gets prepared
  pal::Layer* palLayer;
  const QgsCoordinateTransform* ct;
  const QgsMapToPixel* xform;
  QList<QgsPalGeometry*> geometries;

  int xPosColumn; //attribute index for x coordinate (or -1 if position not data defined)
  int yPosColumn;//attribute index for y coordinate (or -1 if position not data defined)

  void readXML( const QDomElement& elem );
  void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;
};

//diagram settings for rendering
struct QgsDiagramSettings
{
  enum SizeType
  {
    MM,
    MapUnits
  };

  QgsDiagramSettings(): minScaleDenominator( -1 ), maxScaleDenominator( -1 ), sizeType( MM )
  {}
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
    const QgsDiagram* diagram() const { return mDiagram; }

    /**Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXML( const QDomElement& elem ) = 0;
    virtual void writeXML( QDomElement& layerElem, QDomDocument& doc ) const = 0;

  protected:

    /**Returns diagram settings for a feature (or false if the diagram for the feature is not to be rendered). Used internally within renderDiagram()
      @param s out: diagram settings for the feature*/
    virtual bool diagramSettings( const QgsAttributeMap& att, const QgsRenderContext& c, QgsDiagramSettings& s ) = 0;

    /**Returns size of the diagram (in painter units) or an invalid size in case of error*/
    virtual QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c ) = 0;

    /**Converts size from mm to map units*/
    void convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const;

    /**Returns the paint device dpi (or -1 in case of error*/
    static int dpiPaintDevice( const QPainter* );

    //read / write diagram
    void _readXML( const QDomElement& elem );
    void _writeXML( QDomElement& rendererElem, QDomDocument& doc ) const;

    /**Reference to the object that does the real diagram rendering*/
    QgsDiagram* mDiagram;
};

/**Renders the diagrams for all features with the same settings*/
class QgsSingleCategoryDiagramRenderer: public QgsDiagramRendererV2
{
  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    QString rendererName() const { return "SingleCategory"; }

    QList<int> diagramAttributes() const { return mSettings.categoryIndices; }

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<QgsDiagramSettings> diagramSettings() const;

    void readXML( const QDomElement& elem );
    void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;

  protected:
    bool diagramSettings( const QgsAttributeMap&, const QgsRenderContext& c, QgsDiagramSettings& s );

    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c ) { return mSettings.size; }

  private:
    QgsDiagramSettings mSettings;
};

class QgsLinearlyInterpolatedDiagramRenderer: public QgsDiagramRendererV2
{
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    /**Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const;

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<int> diagramAttributes() const;

    QString rendererName() const { return "LinearlyInterpolated"; }

    void setLowerValue( double val ) { mLowerValue = val; }
    double lowerValue() const { return mLowerValue; }

    void setUpperValue( double val ) { mUpperValue = val; }
    double upperValue() const { return mUpperValue; }

    void setLowerSize( QSizeF s ) { mLowerSize = s; }
    QSizeF lowerSize() const { return mLowerSize; }

    void setUpperSize( QSizeF s ) { mUpperSize = s; }
    QSizeF upperSize() const { return mUpperSize; }

    int classificationAttribute() const { return mClassificationAttribute; }
    void setClassificationAttribute( int index ) { mClassificationAttribute = index; }

    void readXML( const QDomElement& elem );
    void writeXML( QDomElement& layerElem, QDomDocument& doc ) const;

  protected:
    bool diagramSettings( const QgsAttributeMap&, const QgsRenderContext& c, QgsDiagramSettings& s );

    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c );

  private:
    QgsDiagramSettings mSettings;
    QSizeF mLowerSize;
    QSizeF mUpperSize;
    double mLowerValue;
    double mUpperValue;
    /**Index of the classification attribute*/
    int mClassificationAttribute;
};

#endif // QGSDIAGRAMRENDERERV2_H
