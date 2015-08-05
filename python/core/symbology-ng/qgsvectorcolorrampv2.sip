class QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End

%ConvertToSubClassCode
  if (sipCpp->type() == "gradient")
    sipClass = sipClass_QgsVectorGradientColorRampV2;
  else if (sipCpp->type() == "random")
    sipClass = sipClass_QgsVectorRandomColorRampV2;
  else if (sipCpp->type() == "randomcolors")
    sipClass = sipClass_QgsRandomColorsV2;
  else if (sipCpp->type() == "colorbrewer")
    sipClass = sipClass_QgsVectorColorBrewerColorRampV2;
  else if (sipCpp->type() == "cpt-city")
    sipClass = sipClass_QgsCptCityColorRampV2;
  else
    sipClass = 0;
%End

  public:

    virtual ~QgsVectorColorRampV2();

    // Number of defined colors
    virtual int count() const = 0;

    // Relative value (0,1) of color at index
    virtual double value( int index ) const = 0;

    virtual QColor color( double value ) const = 0;

    virtual QString type() const = 0;

    virtual QgsVectorColorRampV2* clone() const = 0;

    virtual QgsStringMap properties() const = 0;

};

struct QgsGradientStop
{
  double offset; // relative (0,1)
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

    virtual int count() const;

    virtual double value( int index ) const;

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
    void convertToDiscrete( bool discrete );

    void setStops( const QgsGradientStopsList& stops );
    const QgsGradientStopsList& stops() const;

    QgsStringMap info() const;
    void setInfo( const QgsStringMap& info );

    /** Copy color ramp stops to a QGradient
    * @note added in 2.1 */
    void addStopsToGradient( QGradient* gradient, double alpha = 1 );
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

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    /** Get a list of random colors
     * @note added in 2.4 */
    static QList<QColor> randomColors( int count,
                                       int hueMax = DEFAULT_RANDOM_HUE_MAX, int hueMin = DEFAULT_RANDOM_HUE_MIN,
                                       int satMax = DEFAULT_RANDOM_SAT_MAX, int satMin = DEFAULT_RANDOM_SAT_MIN,
                                       int valMax = DEFAULT_RANDOM_VAL_MAX, int valMin = DEFAULT_RANDOM_VAL_MIN );

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

class QgsRandomColorsV2 : QgsVectorColorRampV2
{
  public:
    QgsRandomColorsV2();
    ~QgsRandomColorsV2();

    int count() const;

    double value( int index ) const;

    QColor color( double value ) const;

    /** Sets the desired total number of unique colors for the resultant ramp. Calling
     * this method pregenerates a set of visually distinct colors which are returned
     * by subsequent calls to color().
     * @param colorCount number of unique colors
     * @note added in QGIS 2.5
     */
    virtual void setTotalColorCount( const int colorCount );

    QString type() const;

    QgsVectorColorRampV2* clone() const;

    QgsStringMap properties() const;
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

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    QString schemeName() const;
    virtual int count() const;
    int colors() const;

    void setSchemeName( QString schemeName );
    void setColors( int colors );

    static QStringList listSchemeNames();
    static QList<int> listSchemeVariants( QString schemeName );

  protected:

    void loadPalette();
};


class QgsCptCityColorRampV2 : QgsVectorGradientColorRampV2
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
    QgsVectorGradientColorRampV2* cloneGradientRamp() const /Factory/;

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

    QString fileName() const;
    bool loadFile();
    bool fileLoaded() const;

    QString copyingFileName() const;
    QString descFileName() const;
    QMap< QString, QString > copyingInfo() const;
};
