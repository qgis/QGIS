/** \ingroup core
 * \class QgsVectorColorRampV2
 * \brief Abstract base class for color ramps
 */
//TODO QGIS 3.0 - rename to QgsColorRamp, since this is used by much more than just vectors
class QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End

%ConvertToSubClassCode
  if (sipCpp->type() == "gradient")
    sipType = sipType_QgsVectorGradientColorRampV2;
  else if (sipCpp->type() == "random")
    sipType = sipType_QgsVectorRandomColorRampV2;
  else if (sipCpp->type() == "randomcolors")
    sipType = sipType_QgsRandomColorsV2;
  else if (sipCpp->type() == "colorbrewer")
    sipType = sipType_QgsVectorColorBrewerColorRampV2;
  else if (sipCpp->type() == "cpt-city")
    sipType = sipType_QgsCptCityColorRampV2;
  else
    sipType = 0;
%End

  public:

    virtual ~QgsVectorColorRampV2();

    /** Returns number of defined colors, or -1 if undefined
     */
    virtual int count() const = 0;

    /** Returns relative value between [0,1] of color at specified index
     */
    virtual double value( int index ) const = 0;

    /** Returns the color corresponding to a specified value.
     * @param value value between [0, 1] inclusive
     * @returns color for value
     */
    virtual QColor color( double value ) const = 0;

    /** Returns a string representing the color ramp type.
     */
    virtual QString type() const = 0;

    /** Creates a clone of the color ramp.
     */
    virtual QgsVectorColorRampV2* clone() const = 0 /Factory/;

    /** Returns a string map containing all the color ramp's properties.
     */
    virtual QgsStringMap properties() const = 0;

};

/** \ingroup core
 * \class QgsGradientStop
 * \brief Represents a color stop within a gradient color ramp.
 */
class QgsGradientStop
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:

    /** Constructor for QgsGradientStop
     * @param o positional offset for stop, between 0 and 1.0
     * @param c color for stop
     */
    QgsGradientStop( double o, const QColor& c );

    //! Relative positional offset, between 0 and 1
    double offset;

    //! Gradient color at stop
    QColor color;

    bool operator==( const QgsGradientStop& other ) const;
};

//! List of gradient stops
typedef QList<QgsGradientStop> QgsGradientStopsList;

/** \ingroup core
 * \class QgsVectorGradientColorRampV2
 * \brief Gradient color ramp, which smoothly interpolates between two colors and also
 * supports optional extra color stops.
 */
//TODO QGIS 3.0 - rename to QgsGradientColorRamp, since this is used by much more than just vectors
class QgsVectorGradientColorRampV2 : QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:

    /** Constructor for QgsVectorGradientColorRampV2
     * @param color1 start color, corresponding to a position of 0.0
     * @param color2 end color, corresponding to a position of 1.0
     * @param discrete set to true for discrete interpolation instead of smoothly
     * interpolating between colors
     * @param stops optional list of additional color stops
     */
    QgsVectorGradientColorRampV2( const QColor& color1 = DEFAULT_GRADIENT_COLOR1,
                                  const QColor& color2 = QColor(0,255,0),
                                  bool discrete = false,
                                  const QgsGradientStopsList& stops = QgsGradientStopsList() );

    //! Creates a new QgsVectorColorRampV2 from a map of properties
    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual int count() const;
    virtual double value( int index ) const;
    virtual QColor color( double value ) const;
    virtual QString type() const;
    virtual QgsVectorGradientColorRampV2* clone() const /Factory/;
    virtual QgsStringMap properties() const;

    /** Returns the gradient start color.
     * @see setColor1()
     * @see color2()
     */
    QColor color1() const;

    /** Returns the gradient end color.
     * @see setColor2()
     * @see color1()
     */
    QColor color2() const;

    /** Sets the gradient start color.
     * @param color start color
     * @see color1()
     * @see setColor2()
     */
    void setColor1( const QColor& color );

    /** Sets the gradient end color.
     * @param color end color
     * @see color2()
     * @see setColor1()
     */
    void setColor2( const QColor& color );

    /** Returns true if the gradient is using discrete interpolation, rather than
     * smoothly interpolating between colors.
     * @see setDiscrete()
     */
    bool isDiscrete() const;

    /** Sets whether the gradient should use discrete interpolation, rather than
     * smoothly interpolating between colors.
     * @param discrete set to true to use discrete interpolation
     * @see convertToDiscrete()
     * @see isDiscrete()
     */
    void setDiscrete( bool discrete );

    /** Converts a gradient with existing color stops to or from discrete
     * interpolation.
     * @param discrete set to true to convert the gradient stops to discrete,
     * or false to convert them to smooth interpolation
     * @see isDiscrete()
     */
    void convertToDiscrete( bool discrete );

    /** Sets the list of intermediate gradient stops for the ramp.
     * @param stops list of stops. Any existing color stops will be replaced
     * @see stops()
     */
    void setStops( const QgsGradientStopsList& stops );

    /** Returns the list of intermediate gradient stops for the ramp.
     * @see setStops()
     */
    QgsGradientStopsList stops() const;

    /** Returns any additional info attached to the gradient ramp (eg authorship notes)
     * @see setInfo()
     */
    QgsStringMap info() const;

    /** Sets additional info to attach to the gradient ramp (eg authorship notes)
     * @param info map of string info to attach
     * @see info()
     */
    void setInfo( const QgsStringMap& info );

    /** Copy color ramp stops to a QGradient
     * @param gradient gradient to copy stops into
     * @param alpha alpha multiplier. Opacity of colors will be multiplied
     * by this factor before adding to the gradient.
     * @note added in 2.1
     */
    void addStopsToGradient( QGradient* gradient, double alpha = 1 );
};

/** \ingroup core
 * \class QgsVectorRandomColorRampV2
 * \brief Random color ramp, which returns random colors based on preset parameters.
 */
//TODO QGIS 3.0 - rename to QgsRandomColorRamp, since this is used by much more than just vectors
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

    virtual QgsVectorRandomColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    /** Get a list of random colors
     * @note added in 2.4
     */
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

    virtual QgsRandomColorsV2* clone() const /Factory/;

    QgsStringMap properties() const;
};

class QgsVectorColorBrewerColorRampV2 : QgsVectorColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsVectorColorBrewerColorRampV2( const QString& schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                                     int colors = DEFAULT_COLORBREWER_COLORS );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;

    virtual QgsVectorColorBrewerColorRampV2* clone() const /Factory/;

    virtual QgsStringMap properties() const;

    QString schemeName() const;
    virtual int count() const;
    int colors() const;

    void setSchemeName( const QString& schemeName );
    void setColors( int colors );

    static QStringList listSchemeNames();
    static QList<int> listSchemeVariants( const QString& schemeName );

  protected:

    void loadPalette();
};


class QgsCptCityColorRampV2 : QgsVectorGradientColorRampV2
{
%TypeHeaderCode
#include <qgsvectorcolorrampv2.h>
%End
  public:
    QgsCptCityColorRampV2( const QString& schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           const QString& variantName = DEFAULT_CPTCITY_VARIANTNAME,
                           bool doLoadFile = true );
    QgsCptCityColorRampV2( const QString& schemeName, const QStringList& variantList,
                           const QString& variantName = QString(), bool doLoadFile = true );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    virtual QString type() const;

    virtual QgsCptCityColorRampV2* clone() const /Factory/;
    void copy( const QgsCptCityColorRampV2* other );
    QgsVectorGradientColorRampV2* cloneGradientRamp() const /Factory/;

    virtual QgsStringMap properties() const;

    QString schemeName() const;
    QString variantName() const;
    QStringList variantList() const;

    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( const QString& schemeName );
    void setVariantName( const QString& variantName );
    void setVariantList( const QStringList& variantList );
    void setName( const QString& schemeName, const QString& variantName = "", const QStringList& variantList = QStringList() );

    void loadPalette();
    bool hasMultiStops() const;

    QString fileName() const;
    bool loadFile();
    bool fileLoaded() const;

    QString copyingFileName() const;
    QString descFileName() const;
    QMap< QString, QString > copyingInfo() const;
};
