
typedef QList<QgsSymbolV2*> QgsSymbolV2List;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;

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

  void renderFeature(QgsFeature& feature, QgsRenderContext& context, int layer = -1);

  bool usingSymbolLevels() const;
  void setUsingSymbolLevels(bool usingSymbolLevels);

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);

protected:
  QgsFeatureRendererV2(QString type);

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

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone() /Factory/;

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

  const QgsCategoryList& categories();

  //! return index of category with specified value (-1 if not found)
  int categoryIndexForValue(QVariant val);

  bool updateCategorySymbol(int catIndex, QgsSymbolV2* symbol /Transfer/);
  bool updateCategoryLabel(int catIndex, QString label);

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

  QString classAttribute() const;
  void setClassAttribute(QString attr);

  const QgsRangeList& ranges();

  bool updateRangeSymbol(int rangeIndex, QgsSymbolV2* symbol /Transfer/);
  bool updateRangeLabel(int rangeIndex, QString label);

  enum Mode
  {
    EqualInterval,
    Quantile,
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
                  QgsVectorColorRampV2* ramp);

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

protected:
  QgsSymbolV2* symbolForValue(double value);
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
    case QgsSymbolV2::Marker: sipClass = sipClass_QgsMarkerSymbolLayerV2; break;
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

  virtual void startRender(QgsRenderContext& context) = 0;
  virtual void stopRender(QgsRenderContext& context) = 0;

  virtual QgsSymbolLayerV2* clone() const = 0 /Factory/;

  virtual QgsStringMap properties() const = 0;

  virtual void drawPreviewIcon(QPainter* painter, QSize size) = 0;

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
  virtual void renderPoint(const QPointF& point, QgsRenderContext& context) = 0;

  void drawPreviewIcon(QPainter* painter, QSize size);

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
  virtual void renderPolyline(const QPolygonF& points, QgsRenderContext& context) = 0;

  void setWidth(double width);
  double width() const;
  	
  void drawPreviewIcon(QPainter* painter, QSize size);

protected:
  QgsLineSymbolLayerV2(bool locked = false);

};

class QgsFillSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include <qgssymbollayerv2.h>
%End

public:
  virtual void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context) = 0;

  void drawPreviewIcon(QPainter* painter, QSize size);

protected:
  QgsFillSymbolLayerV2(bool locked = false);
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

  enum SymbolType
  {
  	Marker,
  	Line,
  	Fill
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


  void startRender(QgsRenderContext& context);
  void stopRender(QgsRenderContext& context);

  void setColor(const QColor& color);
  QColor color();

  void drawPreviewIcon(QPainter* painter, QSize size);

  QImage bigSymbolPreviewImage();

  QString dump();

  virtual QgsSymbolV2* clone() const = 0 /Factory/;

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
  QgsMarkerSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void setAngle(double angle);
  double angle();

  void setSize(double size);
  double size();

  void renderPoint(const QPointF& point, QgsRenderContext& context, int layer = -1);

  virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsLineSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

public:
  QgsLineSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void setWidth(double width);
  double width();

  void renderPolyline(const QPolygonF& points, QgsRenderContext& context, int layer = -1);

  virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsFillSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

public:
  QgsFillSymbolV2(QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List());

  void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context, int layer = -1);

  virtual QgsSymbolV2* clone() const /Factory/;
};

//////////

typedef QMap<QString, QString> QgsStringMap;

//////////

class QgsSymbolLayerV2Metadata
{
%TypeHeaderCode
#include <qgssymbollayerv2registry.h>
%End

public:
  /** construct invalid metadata */
  QgsSymbolLayerV2Metadata();

  /** construct metadata */
  // TODO
  //QgsSymbolLayerV2Metadata(QString name, QgsSymbolV2::SymbolType type,
  //                         QgsSymbolLayerV2CreateFunc pfCreate,
  //                         QgsSymbolLayerV2WidgetFunc pfWidget);

  QString name() const;
  QgsSymbolV2::SymbolType type();
  // TODO QgsSymbolLayerV2CreateFunc createFunction() const { return mCreateFunc; }
  // TODO QgsSymbolLayerV2WidgetFunc widgetFunction() const { return mWidgetFunc; }

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
  QgsSymbolLayerV2Metadata symbolLayerMetadata(QString name) const;

  //! register a new symbol layer type
  void addSymbolLayerType(const QgsSymbolLayerV2Metadata& metadata);

  //! create a new instance of symbol layer given symbol layer name and properties
  QgsSymbolLayerV2* createSymbolLayer(QString name, const QgsStringMap& properties) const /Factory/;

  //! return a list of available symbol layers for a specified symbol type
  QStringList symbolLayersForType(QgsSymbolV2::SymbolType type);

  //! create a new instance of symbol layer for specified symbol type with default settings
  static QgsSymbolLayerV2* defaultSymbolLayer(QgsSymbolV2::SymbolType type) /Factory/;

protected:
  QgsSymbolLayerV2Registry();

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

  //! return a NEW copy of symbol
  QgsSymbolV2* symbol(QString name);

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

  //! return a NEW copy of color ramp
  QgsVectorColorRampV2* colorRamp(QString name);

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

public:
  virtual ~QgsVectorColorRampV2();

  virtual QColor color(double value) const = 0;

  virtual QString type() const = 0;

  virtual QgsVectorColorRampV2* clone() const = 0;

  virtual QgsStringMap properties() const = 0;

};
