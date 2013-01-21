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

    virtual int count() const = 0;

    virtual double value( int index ) const = 0;

    virtual QColor color( double value ) const = 0;

    virtual QString type() const = 0;

    virtual QgsVectorColorRampV2* clone() const = 0;

    virtual QgsStringMap properties() const = 0;

};

struct QgsGradientStop
{
  double offset;
  QColor color;
  QgsGradientStop( double o, const QColor& c );
};

typedef QList<QgsGradientStop> QgsGradientStopsList;

class QgsVectorGradientColorRampV2 : QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsVectorGradientColorRampV2( QColor color1 = DEFAULT_GRADIENT_COLOR1,
                                  QColor color2 = QColor(0,255,0),
                                  bool discrete = false,
                                  QgsGradientStopsList stops = QgsGradientStopsList() );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    QColor color1() const;
    QColor color2() const;

    void setColor1( QColor color );
    void setColor2( QColor color );

    bool isDiscrete() const;
    void setDiscrete( bool discrete );

    void setStops( const QgsGradientStopsList& stops );
    const QgsGradientStopsList& stops() const;
};

class QgsVectorRandomColorRampV2 : QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsVectorRandomColorRampV2( int count = DEFAULT_RANDOM_COUNT,
                                int hueMin = DEFAULT_RANDOM_HUE_MIN, int hueMax = DEFAULT_RANDOM_HUE_MAX,
                                int satMin = DEFAULT_RANDOM_SAT_MIN, int satMax = DEFAULT_RANDOM_SAT_MAX,
                                int valMin = DEFAULT_RANDOM_VAL_MIN, int valMax = DEFAULT_RANDOM_VAL_MAX );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    void updateColors();

    int count() const;
    int hueMin() const;
    int hueMax() const;
    int satMin() const;
    int satMax() const;
    int valMin() const;
    int valMax() const;

    void setCount( int val );
    void setHueMin( int val );
    void setHueMax( int val );
    void setSatMin( int val );
    void setSatMax( int val );
    void setValMin( int val );
    void setValMax( int val );
};


class QgsVectorColorBrewerColorRampV2 : QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsVectorColorBrewerColorRampV2( QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                                     int colors = DEFAULT_COLORBREWER_COLORS );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    QString schemeName() const;
    int colors() const;

    void setSchemeName( QString schemeName );
    void setColors( int colors );

    static QStringList listSchemeNames();
    static QList<int> listSchemeVariants( QString schemeName );

  protected:

    void loadPalette();
};


class QgsCptCityColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsCptCityColorRampV2( QString schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           QString variantName = DEFAULT_CPTCITY_VARIANTNAME,
                           bool doLoadFile = true );
    QgsCptCityColorRampV2( QString schemeName, QStringList variantList,
                           QString variantName = QString(), bool doLoadFile = true );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;
    void copy( const QgsCptCityColorRampV2* other );

    virtual QgsStringMap properties() const;

    QString schemeName() const;
    QString variantName() const;
    QStringList variantList() const;

    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( QString schemeName );
    void setVariantName( QString variantName );
    void setVariantList( QStringList variantList );
    void setName( QString schemeName, QString variantName = "", QStringList variantList = QStringList() );

    void loadPalette();
    bool hasMultiStops() const;
    int count() const;

    QString fileName() const;
    bool loadFile();
    bool fileLoaded() const;

    QString copyingFileName() const;
    QString descFileName() const;
    QMap< QString, QString > copyingInfo() const;
};
