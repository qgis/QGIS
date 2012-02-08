
typedef QList<QgsSymbolV2*> QgsSymbolV2List;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;


// this is a workaround for an error in generated code by SIP
// to ensure it will recognize the class name
%ModuleHeaderCode
class QgsRendererV2Widget;
class QgsSymbolLayerV2Widget;
%End


///////////////

/*
class QgsSymbolV2LevelItem
{
%TypeHeaderCode
#include <qgsrendererv2.h>
%End

public:
  QgsSymbolV2LevelItem( QgsSymbolV2* symbol, int layer );
  QgsSymbolV2* symbol();
  int layer();
};

// every level has list of items: symbol + symbol layer num
typedef QList< QgsSymbolV2LevelItem > QgsSymbolV2Level;

// this is a list of levels
typedef QList< QgsSymbolV2Level > QgsSymbolV2LevelOrder;
*/

///////////////


class QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsrendererv2.h>
%End


%ConvertToSubClassCode
  if (sipCpp->type() == "singleSymbol")
    sipClass = sipClass_QgsSingleSymbolRendererV2;
  else if (sipCpp->type() == "categorizedSymbol")
    sipClass = sipClass_QgsCategorizedSymbolRendererV2;
  else if (sipCpp->type() == "graduatedSymbol")
    sipClass = sipClass_QgsGraduatedSymbolRendererV2;
  else if (sipCpp->type() == "RuleRenderer")
    sipClass = sipClass_QgsRuleBasedRendererV2;
  else
    sipClass = 0;
%End


public:

  //! return a new renderer - used by default in vector layers
  static QgsFeatureRendererV2* defaultRenderer(QGis::GeometryType geomType) /Factory/;

  QString type() const;

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature)=0;

  virtual void startRender(QgsRenderContext& context, const QgsVectorLayer * )=0;

  virtual void stopRender(QgsRenderContext& context)=0;

  virtual QList<QString> usedAttributes()=0;

  virtual ~QgsFeatureRendererV2();

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone()=0 /Factory/;

  virtual QgsSymbolV2List symbols()=0;

  virtual bool renderFeature(QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

  bool usingSymbolLevels() const;
  void setUsingSymbolLevels(bool usingSymbolLevels);

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);

  //! set type and size of editing vertex markers for subsequent rendering
  void setVertexMarkerAppearance( int type, int size );

  //! return rotation field name (or empty string if not set or not supported by renderer)
  //! @note added in 1.9
  virtual QString rotationField() const;
  //! sets rotation field of renderer (if supported by the renderer)
  //! @note added in 1.9
  virtual void setRotationField( QString fieldName );

protected:
  QgsFeatureRendererV2(QString type);

  //! render editing vertex marker at specified point
  void renderVertexMarker( QPointF& pt, QgsRenderContext& context );
  //! render editing vertex marker for a polyline
  void renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context );
  //! render editing vertex marker for a polygon
  void renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context );

};

///////////////

class QgsSingleSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgssinglesymbolrendererv2.h>
%End

public:

  QgsSingleSymbolRendererV2(QgsSymbolV2* symbol /Transfer/);

  virtual ~QgsSingleSymbolRendererV2();

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);

  virtual void startRender(QgsRenderContext& context, const QgsVectorLayer * );

  virtual void stopRender(QgsRenderContext& context);

  virtual QList<QString> usedAttributes();

  QgsSymbolV2* symbol() const;
  void setSymbol(QgsSymbolV2* s /Transfer/);

  //! @note added in 1.5
  void setRotationField(QString fieldName);
  //! @note added in 1.5
  QString rotationField() const;

  //! @note added in 1.5
  void setSizeScaleField(QString fieldName);
  //! @note added in 1.5
  QString sizeScaleField() const;

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone() /Factory/;

  virtual QgsSymbolV2List symbols();

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem) /Factory/;

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);

};

//////////

class QgsRendererCategoryV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End

public:

  //! takes ownership of symbol
  QgsRendererCategoryV2(QVariant value, QgsSymbolV2* symbol /Transfer/, QString label);

  QgsRendererCategoryV2(const QgsRendererCategoryV2& cat);

  ~QgsRendererCategoryV2();

  QVariant value() const;
  QgsSymbolV2* symbol();
  QString label() const;

  void setValue( const QVariant &value );
  void setSymbol(QgsSymbolV2* s /Transfer/);
  void setLabel(QString label);

  QString dump();
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class QgsCategorizedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End

public:

  QgsCategorizedSymbolRendererV2(QString attrName = QString(), QgsCategoryList categories = QgsCategoryList());

  virtual ~QgsCategorizedSymbolRendererV2();

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);

  virtual void startRender(QgsRenderContext& context, const QgsVectorLayer * );

  virtual void stopRender(QgsRenderContext& context);

  virtual QList<QString> usedAttributes();

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone() /Factory/;

  virtual QgsSymbolV2List symbols();

  const QgsCategoryList& categories();

  //! return index of category with specified value (-1 if not found)
  int categoryIndexForValue(QVariant val);

  bool updateCategoryValue( int catIndex, const QVariant &value );
  bool updateCategorySymbol(int catIndex, QgsSymbolV2* symbol /Transfer/);
  bool updateCategoryLabel(int catIndex, QString label);

  void addCategory( const QgsRendererCategoryV2 &category );
  bool deleteCategory(int catIndex);
  void deleteAllCategories();

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem) /Factory/;

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);

  QString classAttribute() const;
  void setClassAttribute(QString attr);

  QgsSymbolV2* sourceSymbol();
  void setSourceSymbol(QgsSymbolV2* sym /Transfer/);

  QgsVectorColorRampV2* sourceColorRamp();
  void setSourceColorRamp(QgsVectorColorRampV2* ramp /Transfer/);

  //! @note added in 1.6
  void setRotationField( QString fieldName );
  //! @note added in 1.6
  QString rotationField() const;

  //! @note added in 1.6
  void setSizeScaleField( QString fieldName );
  //! @note added in 1.6
  QString sizeScaleField() const;

protected:

  QgsSymbolV2* symbolForValue(QVariant value);
};

//////////

class QgsRendererRangeV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

public:
  QgsRendererRangeV2(double lowerValue, double upperValue, QgsSymbolV2* symbol /Transfer/, QString label);
  QgsRendererRangeV2(const QgsRendererRangeV2& range);

  ~QgsRendererRangeV2();

  double lowerValue() const;
  double upperValue() const;

  QgsSymbolV2* symbol() const;
  QString label() const;

  void setSymbol(QgsSymbolV2* s /Transfer/);
  void setLabel(QString label);

  QString dump();
};

typedef QList<QgsRendererRangeV2> QgsRangeList;

class QgsGraduatedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

public:
  QgsGraduatedSymbolRendererV2(QString attrNum = QString(), QgsRangeList ranges = QgsRangeList());

  virtual ~QgsGraduatedSymbolRendererV2();

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);

  virtual void startRender(QgsRenderContext& context, const QgsVectorLayer * );

  virtual void stopRender(QgsRenderContext& context);

  virtual QList<QString> usedAttributes();

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone() /Factory/;

  virtual QgsSymbolV2List symbols();

  QString classAttribute() const;
  void setClassAttribute(QString attr);

  const QgsRangeList& ranges();

  bool updateRangeSymbol(int rangeIndex, QgsSymbolV2* symbol /Transfer/);
  bool updateRangeLabel(int rangeIndex, QString label);
  bool updateRangeUpperValue( int rangeIndex, double value );
  bool updateRangeLowerValue( int rangeIndex, double value );

  void addClass( QgsSymbolV2* symbol );
  void deleteClass( int idx );

  enum Mode
  {
    EqualInterval,
    Quantile,
    Jenks,
    StdDev,
    Pretty,
    Custom
  };

  Mode mode() const;
  void setMode(Mode mode);

  static QgsGraduatedSymbolRendererV2* createRenderer(
                  QgsVectorLayer* vlayer,
                  QString attrName,
                  int classes,
                  Mode mode,
                  QgsSymbolV2* symbol,
                  QgsVectorColorRampV2* ramp) /Factory/;

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem) /Factory/;

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);

  QgsSymbolV2* sourceSymbol();
  void setSourceSymbol(QgsSymbolV2* sym /Transfer/);

  QgsVectorColorRampV2* sourceColorRamp();
  void setSourceColorRamp(QgsVectorColorRampV2* ramp /Transfer/);

  //! @note added in 1.6
  void setRotationField( QString fieldName );
  //! @note added in 1.6
  QString rotationField() const;

  //! @note added in 1.6
  void setSizeScaleField( QString fieldName );
  //! @note added in 1.6
  QString sizeScaleField() const;

protected:
  QgsSymbolV2* symbolForValue(double value);
};

///////////////

class QgsRuleBasedRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2.h>
%End

  public:

    //typedef QList<QgsRuleBasedRendererV2::Rule*> RuleList;

    /**
      This class keeps data about a rules for rule-based renderer.
      A rule consists of a symbol, filter expression and range of scales.
      If filter is empty, it matches all features.
      If scale range has both values zero, it matches all scales.
      If one of the min/max scale denominators is zero, there is no lower/upper bound for scales.
      A rule matches if both filter and scale range match.
     */
    class Rule
    {
      public:
        //! Constructor takes ownership of the symbol
        Rule( QgsSymbolV2* symbol /Transfer/, int scaleMinDenom = 0, int scaleMaxDenom = 0, QString filterExp = QString() );
        //Rule( const QgsRuleBasedRendererV2::Rule& other );
        ~Rule();
        QString dump() const;
        //QStringList needsFields() const;
        bool isFilterOK( QgsFeature& f ) const;
        bool isScaleOK( double scale ) const;

        QgsSymbolV2* symbol();
        bool dependsOnScale() const;
        int scaleMinDenom() const;
        int scaleMaxDenom() const;
        QString filterExpression() const;
        QgsExpression* filter() const;

        void setScaleMinDenom( int scaleMinDenom );
        void setScaleMaxDenom( int scaleMaxDenom );
        void setFilterExpression( QString filterExp );

        //Rule& operator=( const Rule& other );
    };

    /////

    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! Constructs the renderer from given tree of rules
    QgsRuleBasedRendererV2( QgsRuleBasedRendererV2::Rule* root /Transfer/ );
    //! Constructor for convenience. Creates a root rule and adds a default rule with symbol
    QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol /Transfer/ );

    //! return symbol for current feature. Should not be used individually: there could be more symbols for a feature
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QgsFeatureRendererV2* clone() /Factory/;

    virtual QgsSymbolV2List symbols();

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend 
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ); 

    /////


    //////

    //! take a rule and create a list of new rules based on the categories from categorized symbol renderer
    //static QgsRuleBasedRendererV2::RuleList refineRuleCategories( QgsRuleBasedRendererV2::Rule* initialRule, QgsCategorizedSymbolRendererV2* r );
    //! take a rule and create a list of new rules based on the ranges from graduated symbol renderer
    //static QgsRuleBasedRendererV2::RuleList refineRuleRanges( QgsRuleBasedRendererV2::Rule* initialRule, QgsGraduatedSymbolRendererV2* r );
    //! take a rule and create a list of new rules with intervals of scales given by the passed scale denominators
    //static QgsRuleBasedRendererV2::RuleList refineRuleScales( QgsRuleBasedRendererV2::Rule* initialRule, QList<int> scales );

};


//////////

class QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbolV2::Marker:
      if( sipCpp->layerType() == "VectorField" )
      {
        sipClass = sipClass_QgsVectorFieldSymbolLayer;
      }
      else
      {
        sipClass = sipClass_QgsMarkerSymbolLayerV2;
      }
      break;
    case QgsSymbolV2::Line: sipClass = sipClass_QgsLineSymbolLayerV2; break;
    case QgsSymbolV2::Fill: sipClass = sipClass_QgsFillSymbolLayerV2; break;
    default: sipClass = 0; break;
  }
%End


public:

  virtual void setColor(const QColor& color);
  virtual QColor color() const;

  virtual ~QgsSymbolLayerV2();

  virtual QString layerType() const = 0;

  virtual void startRender(QgsSymbolV2RenderContext& context) = 0;
  virtual void stopRender(QgsSymbolV2RenderContext& context) = 0;

  virtual QgsSymbolLayerV2* clone() const = 0 /Factory/;

  virtual QgsStringMap properties() const = 0;

  virtual void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size) = 0;

  virtual QgsSymbolV2* subSymbol();
  virtual bool setSubSymbol(QgsSymbolV2* symbol /Transfer/);

  QgsSymbolV2::SymbolType type() const;

  void setLocked(bool locked);
  bool isLocked() const;

  // used only with rending with symbol levels is turned on (0 = first pass, 1 = second, ...)
  void setRenderingPass(int renderingPass);
  int renderingPass() const;

protected:
  QgsSymbolLayerV2(QgsSymbolV2::SymbolType type, bool locked = false);

};

///////////////

class QgsMarkerSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

public:
  virtual void renderPoint(const QPointF& point, QgsSymbolV2RenderContext& context) = 0;

  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

  void setAngle(double angle);
  double angle() const;

  void setSize(double size);
  double size() const;

protected:
  QgsMarkerSymbolLayerV2(bool locked = false);

};

class QgsLineSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

public:
  virtual void renderPolyline(const QPolygonF& points, QgsSymbolV2RenderContext& context) = 0;

  //! @note added in v1.7
  virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

  void setWidth(double width);
  double width() const;
  	
  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

protected:
  QgsLineSymbolLayerV2(bool locked = false);

};

class QgsFillSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

public:
  virtual void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context) = 0;

  void drawPreviewIcon(QgsSymbolV2RenderContext& context, QSize size);

  void setAngle( double angle );
  double angle() const;

protected:
  QgsFillSymbolLayerV2(bool locked = false);
};


///////////////

class QgsSymbolV2RenderContext
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

    public:
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u , qreal alpha = 1.0, bool selected = false, int renderHints = 0 );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext();
    //void setRenderContext( QgsRenderContext* c );

    QgsSymbolV2::OutputUnit outputUnit() const;
    void setOutputUnit( QgsSymbolV2::OutputUnit u );

    qreal alpha() const;
    void setAlpha( qreal alpha );

    bool selected() const;
    void setSelected( bool selected );

    //! @note added in 1.5
    int renderHints() const;
    //! @note added in 1.5
    void setRenderHints( int hints );

    void setFeature( const QgsFeature* f );
    const QgsFeature* feature() const;

    void setLayer( const QgsVectorLayer* layer );
    const QgsVectorLayer* layer() const;

    // Color used for selections
    static QColor selectionColor();

    double outputLineWidth(double width) const;
    double outputPixelSize(double size) const;
};

///////////////


typedef QList<QgsSymbolLayerV2*> QgsSymbolLayerV2List;

class QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbolV2::Marker: sipClass = sipClass_QgsMarkerSymbolV2; break;
    case QgsSymbolV2::Line: sipClass = sipClass_QgsLineSymbolV2; break;
    case QgsSymbolV2::Fill: sipClass = sipClass_QgsFillSymbolV2; break;
    default: sipClass = 0; break;
  }
%End

public:

    enum OutputUnit
    {
      MM,
      MapUnit
    };

  enum SymbolType
  {
  	Marker,
  	Line,
  	Fill
  };

    //! @note added in 1.5
    enum RenderHint
    {
      DataDefinedSizeScale = 1,
      DataDefinedRotation = 2
    };

  virtual ~QgsSymbolV2();

  //! return new default symbol for specified geometry type
  static QgsSymbolV2* defaultSymbol(QGis::GeometryType geomType) /Factory/;

  SymbolType type() const;

  // symbol layers handling

  QgsSymbolLayerV2* symbolLayer(int layer);

  int symbolLayerCount();

  //! insert symbol layer to specified index
  bool insertSymbolLayer(int index, QgsSymbolLayerV2* layer /Transfer/);

  //! append symbol layer at the end of the list
  bool appendSymbolLayer(QgsSymbolLayerV2* layer /Transfer/);

  //! delete symbol layer at specified index
  bool deleteSymbolLayer(int index);

  //! remove symbol layer from the list and return pointer to it
  QgsSymbolLayerV2* takeSymbolLayer(int index) /TransferBack/;

  //! delete layer at specified index and set a new one
  bool changeSymbolLayer(int index, QgsSymbolLayerV2* layer /Transfer/);

  void startRender( QgsRenderContext& context, const QgsVectorLayer* layer = 0 );
  void stopRender(QgsRenderContext& context);

  void setColor(const QColor& color);
  QColor color();

  void drawPreviewIcon(QPainter* painter, QSize size);

  QImage bigSymbolPreviewImage();

  QString dump();

  virtual QgsSymbolV2* clone() const = 0 /Factory/;

    OutputUnit outputUnit() const;
    void setOutputUnit( OutputUnit u );

    qreal alpha() const;
    void setAlpha( qreal alpha );

    //! @note added in 1.5
    int renderHints();
    //! @note added in 1.5
    void setRenderHints( int hints );

protected:
  QgsSymbolV2(SymbolType type, QgsSymbolLayerV2List layers /Transfer/); // can't be instantiated

};

//////////


class QgsMarkerSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

public:
  /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
    This is a convenience method for easier creation of marker symbols.
    \note added in v1.7
  */
  static QgsMarkerSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

  QgsMarkerSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void setAngle(double angle);
  double angle();

  void setSize(double size);
  double size();

  void renderPoint(const QPointF& point, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

  virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsLineSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

public:
  /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
    This is a convenience method for easier creation of line symbols.
    \note added in v1.7
  */
  static QgsLineSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

  QgsLineSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void setWidth(double width);
  double width();

  void renderPolyline(const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

  virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsFillSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

public:
  /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
    This is a convenience method for easier creation of fill symbols.
    \note added in v1.7
  */
  static QgsFillSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

  QgsFillSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void setAngle( double angle );
  void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

  virtual QgsSymbolV2* clone() const /Factory/;
};

//////////

typedef QMap<QString, QString> QgsStringMap;

typedef QMap<int, QgsField> QgsFieldMap;

//////////

class QgsSymbolLayerV2Widget /External/;

class QgsSymbolLayerV2AbstractMetadata
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End

  public:
    /** construct metadata */
    QgsSymbolLayerV2AbstractMetadata( QString name, QString visibleName, QgsSymbolV2::SymbolType type );

    QString name() const;
    QString visibleName() const;
    QgsSymbolV2::SymbolType type() const;

    /** create a symbol layer of this type given the map of properties. */
    virtual QgsSymbolLayerV2* createSymbolLayer( const QgsStringMap& map ) = 0 /Factory/;
    /** create widget for symbol layer of this type. Can return NULL if there's no GUI */
    virtual QgsSymbolLayerV2Widget* createSymbolLayerWidget( const QgsVectorLayer* vl ) /Factory/;
};

//////////

class QgsSymbolLayerV2Registry
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End

public:

  //! return the single instance of this class (instantiate it if not exists)
  static QgsSymbolLayerV2Registry* instance();

  //! return metadata for specified symbol layer
  QgsSymbolLayerV2AbstractMetadata* symbolLayerMetadata(QString name) const;

  //! register a new symbol layer type
  void addSymbolLayerType(QgsSymbolLayerV2AbstractMetadata* metadata /Transfer/);

  //! create a new instance of symbol layer given symbol layer name and properties
  // TODO: disabled in PyQGIS because if used with symbol layer from Python
  //   the combination of /Factory/ annotation QgsSymbolLayerV2AbstractMetadata::createSymbolLayer()
  //   and here is deadly: results in premature deallocation of the symbol layer -> segfaults
  //QgsSymbolLayerV2* createSymbolLayer(QString name, const QgsStringMap& properties) const /Factory/;

  //! return a list of available symbol layers for a specified symbol type
  QStringList symbolLayersForType(QgsSymbolV2::SymbolType type);

  //! create a new instance of symbol layer for specified symbol type with default settings
  static QgsSymbolLayerV2* defaultSymbolLayer(QgsSymbolV2::SymbolType type) /Factory/;

protected:
  QgsSymbolLayerV2Registry();
  ~QgsSymbolLayerV2Registry();

};

//////////

class QgsStyleV2
{
%TypeHeaderCode
#include <qgsstylev2.h>
%End

public:

  QgsStyleV2();
  ~QgsStyleV2();

  //! return default application-wide style
  static QgsStyleV2* defaultStyle();

  //! remove all contents of the style
  void clear();

  //! add symbol to style. takes symbol's ownership
  bool addSymbol(QString name, QgsSymbolV2* symbol);

  //! remove symbol from style (and delete it)
  bool removeSymbol(QString name);

  //! change symbol's name
  //! @note added in v1.7
  bool renameSymbol( QString oldName, QString newName );

  //! return a NEW copy of symbol
  QgsSymbolV2* symbol(QString name) /Factory/;

  //! return a const pointer to a symbol (doesn't create new instance)
  const QgsSymbolV2* symbolRef(QString name) const;

  //! return count of symbols in style
  int symbolCount();

  //! return a list of names of symbols
  QStringList symbolNames();


  //! add color ramp to style. takes ramp's ownership
  bool addColorRamp(QString name, QgsVectorColorRampV2* colorRamp);

  //! remove color ramp from style (and delete it)
  bool removeColorRamp(QString name);

  //! change ramp's name
  //! @note added in v1.7
  bool renameColorRamp( QString oldName, QString newName );

  //! return a NEW copy of color ramp
  QgsVectorColorRampV2* colorRamp(QString name) /Factory/;

  //! return a const pointer to a symbol (doesn't create new instance)
  const QgsVectorColorRampV2* colorRampRef(QString name) const;

  //! return count of color ramps
  int colorRampCount();

  //! return a list of names of color ramps
  QStringList colorRampNames();


  //! load a file into the style
  bool load(QString filename);

  //! save style into a file (will use current filename if empty string is passed)
  bool save(QString filename = QString());

  //! return last error from load/save operation
  QString errorString();

  //! return current file name of the style
  QString fileName();

};

//////////

class QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End

%ConvertToSubClassCode
  if (sipCpp->type() == "gradient")
  {
    sipClass = sipClass_QgsVectorGradientColorRampV2;
  }
  else
    sipClass = 0;
%End

public:
  virtual ~QgsVectorColorRampV2();

  virtual QColor color(double value) const = 0;

  virtual QString type() const = 0;

  virtual QgsVectorColorRampV2* clone() const = 0 /Factory/;

  virtual QgsStringMap properties() const = 0;

};

class QgsVectorGradientColorRampV2 : QgsVectorColorRampV2
{
  public:
    QgsVectorGradientColorRampV2( QColor color1 = QColor(0,0,255),
                                  QColor color2 = QColor(0,255,0) );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    QColor color1() const;
    QColor color2() const;

    void setColor1( QColor color );
    void setColor2( QColor color );

    typedef QMap<double, QColor> StopsMap;

    void setStops(const StopsMap& stops);
    const StopsMap& stops() const;

};



//////////

class QgsSymbologyV2Conversion
{
%TypeHeaderCode
#include <qgssymbologyv2conversion.h>
%End

public:

  //! return a symbol in new symbology as close as possible to old symbol
  //! @note not all properties will be preserved
  static QgsSymbolV2* symbolV1toV2(const QgsSymbol* s) /Factory/;

  //! return a symbol in old symbology as close as possible to new symbol
  //! @note not all properties will be preserved
  static QgsSymbol* symbolV2toV1(QgsSymbolV2* s) /Factory/;

  //! convert layer from old symbology to new symbology
  //! @note not all properties will be preserved
  static void rendererV1toV2(QgsVectorLayer* layer);

  //! convert layer from new symbology to old symbology
  //! @note not all properties will be preserved
  static void rendererV2toV1(QgsVectorLayer* layer);

};

////////////

class QgsRendererV2Widget /External/;

class QgsRendererV2AbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End

  public:
    QgsRendererV2AbstractMetadata( QString name, QString visibleName, QIcon icon = QIcon() );

    QString name() const;
    QString visibleName() const;

    QIcon icon() const;
    void setIcon(const QIcon& icon);

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) = 0 /Factory/;
    /** Return new instance of settings widget for the renderer. Returns NULL on error. */
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

};


class QgsRendererV2Registry
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End

  public:

    static QgsRendererV2Registry* instance();

    //! add a renderer to registry. Takes ownership of the metadata object.
    bool addRenderer( QgsRendererV2AbstractMetadata* metadata /Transfer/ );

    //! remove renderer from registry
    bool removeRenderer( QString rendererName );

    //! get metadata for particular renderer. Returns NULL if not found in registry.
    QgsRendererV2AbstractMetadata* rendererMetadata( QString rendererName );

    //! return a list of available renderers
    QStringList renderersList();

  protected:
    //! protected constructor
    QgsRendererV2Registry();
    ~QgsRendererV2Registry();
};

///////////////

typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

class QgsSymbolLayerV2Utils
{
%TypeHeaderCode
#include <qgssymbollayerv2utils.h>
%End

  public:

    static QString encodeColor( QColor color );
    static QColor decodeColor( QString str );

    static QString encodePenStyle( Qt::PenStyle style );
    static Qt::PenStyle decodePenStyle( QString str );

    static QString encodePenJoinStyle( Qt::PenJoinStyle style );
    static Qt::PenJoinStyle decodePenJoinStyle( QString str );

    static QString encodePenCapStyle( Qt::PenCapStyle style );
    static Qt::PenCapStyle decodePenCapStyle( QString str );

    static QString encodeBrushStyle( Qt::BrushStyle style );
    static Qt::BrushStyle decodeBrushStyle( QString str );

    static QString encodePoint( QPointF point );
    static QPointF decodePoint( QString str );

    static QString encodeRealVector( const QVector<double>& v );
    static QVector<double> decodeRealVector( const QString& s );

    static QString encodeOutputUnit( QgsSymbolV2::OutputUnit unit );
    static QgsSymbolV2::OutputUnit decodeOutputUnit( QString str );

    static QIcon symbolPreviewIcon( QgsSymbolV2* symbol, QSize size );
    static QIcon symbolLayerPreviewIcon( QgsSymbolLayerV2* layer, QgsSymbolV2::OutputUnit u, QSize size );
    static QIcon colorRampPreviewIcon( QgsVectorColorRampV2* ramp, QSize size );

    static QPixmap symbolPreviewPixmap( QgsSymbolV2* symbol, QSize size );
    static QPixmap colorRampPreviewPixmap( QgsVectorColorRampV2* ramp, QSize size );

    static QgsSymbolV2* loadSymbol( QDomElement& element ) /Factory/;
    static QgsSymbolLayerV2* loadSymbolLayer( QDomElement& element ) /Factory/;
    static QDomElement saveSymbol( QString name, QgsSymbolV2* symbol, QDomDocument& doc, QgsSymbolV2Map* subSymbols = NULL );

    static QgsStringMap parseProperties( QDomElement& element );
    static void saveProperties( QgsStringMap props, QDomDocument& doc, QDomElement& element );

    static QgsSymbolV2Map loadSymbols( QDomElement& element ) /Factory/;
    static QDomElement saveSymbols( QgsSymbolV2Map& symbols, QString tagName, QDomDocument& doc );

    static void clearSymbolMap( QgsSymbolV2Map& symbols );

    static QgsVectorColorRampV2* loadColorRamp( QDomElement& element ) /Factory/;
    static QDomElement saveColorRamp( QString name, QgsVectorColorRampV2* ramp, QDomDocument& doc );

    /**Returns the line width scale factor depending on the unit and the paint device*/
    static double lineWidthScaleFactor( QgsRenderContext& c, QgsSymbolV2::OutputUnit u );
    /**Returns scale factor painter units -> pixel dimensions*/
    static double pixelSizeScaleFactor( QgsRenderContext& c, QgsSymbolV2::OutputUnit u );
    /**Creates a render context for a pixel based device*/
    static QgsRenderContext createRenderContext( QPainter* p );

    /**Multiplies opacity of image pixel values with a (global) transparency value*/
    static void multiplyImageOpacity( QImage* image, qreal alpha );
};
