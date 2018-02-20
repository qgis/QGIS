/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscolorramp.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsColorRamp
{
%Docstring
 Abstract base class for color ramps

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->type() == "gradient" )
      sipType = sipType_QgsGradientColorRamp;
    else if ( sipCpp->type() == "random" )
      sipType = sipType_QgsLimitedRandomColorRamp;
    else if ( sipCpp->type() == "randomcolors" )
      sipType = sipType_QgsRandomColorRamp;
    else if ( sipCpp->type() == "preset" )
      sipType = sipType_QgsPresetSchemeColorRamp;
    else if ( sipCpp->type() == "colorbrewer" )
      sipType = sipType_QgsColorBrewerColorRamp;
    else if ( sipCpp->type() == "cpt-city" )
      sipType = sipType_QgsCptCityColorRamp;
    else
      sipType = 0;
%End
  public:

    virtual ~QgsColorRamp();

    virtual int count() const = 0;
%Docstring
Returns number of defined colors, or -1 if undefined
%End

    virtual double value( int index ) const = 0;
%Docstring
Returns relative value between [0,1] of color at specified index
%End

    virtual QColor color( double value ) const = 0;
%Docstring
Returns the color corresponding to a specified value.

:param value: value between [0, 1] inclusive

:return: color for value
%End

    virtual QString type() const = 0;
%Docstring
Returns a string representing the color ramp type.
%End


    virtual void invert();
%Docstring
Inverts the ordering of the color ramp.
%End

    virtual QgsColorRamp *clone() const = 0 /Factory/;
%Docstring
Creates a clone of the color ramp.
%End

    virtual QgsStringMap properties() const = 0;
%Docstring
Returns a string map containing all the color ramp's properties.
%End
};

class QgsGradientStop
{
%Docstring
 Represents a color stop within a QgsGradientColorRamp color ramp.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsGradientStop( double offset, const QColor &color );
%Docstring
Constructor for QgsGradientStop

:param offset: positional offset for stop, between 0 and 1.0
:param color: color for stop
%End

    double offset;
    QColor color;

    bool operator==( const QgsGradientStop &other ) const;
};

typedef QList<QgsGradientStop> QgsGradientStopsList;


class QgsGradientColorRamp : QgsColorRamp
{
%Docstring
 Gradient color ramp, which smoothly interpolates between two colors and also
supports optional extra color stops.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsGradientColorRamp( const QColor &color1 = DEFAULT_GRADIENT_COLOR1,
                          const QColor &color2 = DEFAULT_GRADIENT_COLOR2,
                          bool discrete = false,
                          const QgsGradientStopsList &stops = QgsGradientStopsList() );
%Docstring
Constructor for QgsGradientColorRamp

:param color1: start color, corresponding to a position of 0.0
:param color2: end color, corresponding to a position of 1.0
:param discrete: set to true for discrete interpolation instead of smoothly
interpolating between colors
:param stops: optional list of additional color stops
%End

    static QgsColorRamp *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Creates a new QgsColorRamp from a map of properties
%End

    virtual int count() const;
    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;
    virtual void invert();

    virtual QgsGradientColorRamp *clone() const /Factory/;

    virtual QgsStringMap properties() const;


    QColor color1() const;
%Docstring
Returns the gradient start color.

.. seealso:: :py:func:`setColor1`

.. seealso:: :py:func:`color2`
%End

    QColor color2() const;
%Docstring
Returns the gradient end color.

.. seealso:: :py:func:`setColor2`

.. seealso:: :py:func:`color1`
%End

    void setColor1( const QColor &color );
%Docstring
Sets the gradient start color.

:param color: start color

.. seealso:: :py:func:`color1`

.. seealso:: :py:func:`setColor2`
%End

    void setColor2( const QColor &color );
%Docstring
Sets the gradient end color.

:param color: end color

.. seealso:: :py:func:`color2`

.. seealso:: :py:func:`setColor1`
%End

    bool isDiscrete() const;
%Docstring
Returns true if the gradient is using discrete interpolation, rather than
smoothly interpolating between colors.

.. seealso:: :py:func:`setDiscrete`
%End

    void setDiscrete( bool discrete );
%Docstring
Sets whether the gradient should use discrete interpolation, rather than
smoothly interpolating between colors.

:param discrete: set to true to use discrete interpolation

.. seealso:: :py:func:`convertToDiscrete`

.. seealso:: :py:func:`isDiscrete`
%End

    void convertToDiscrete( bool discrete );
%Docstring
Converts a gradient with existing color stops to or from discrete
interpolation.

:param discrete: set to true to convert the gradient stops to discrete,
or false to convert them to smooth interpolation

.. seealso:: :py:func:`isDiscrete`
%End

    void setStops( const QgsGradientStopsList &stops );
%Docstring
Sets the list of intermediate gradient stops for the ramp.

:param stops: list of stops. Any existing color stops will be replaced. The stop
list will be automatically reordered so that stops are listed in ascending offset
order.

.. seealso:: :py:func:`stops`
%End

    QgsGradientStopsList stops() const;
%Docstring
Returns the list of intermediate gradient stops for the ramp.

.. seealso:: :py:func:`setStops`
%End

    QgsStringMap info() const;
%Docstring
Returns any additional info attached to the gradient ramp (e.g., authorship notes)

.. seealso:: :py:func:`setInfo`
%End

    void setInfo( const QgsStringMap &info );
%Docstring
Sets additional info to attach to the gradient ramp (e.g., authorship notes)

:param info: map of string info to attach

.. seealso:: :py:func:`info`
%End

    void addStopsToGradient( QGradient *gradient, double opacity = 1 );
%Docstring
Copy color ramp stops to a QGradient

:param gradient: gradient to copy stops into
:param opacity: opacity multiplier. Opacity of colors will be multiplied
by this factor before adding to the gradient.

.. versionadded:: 2.1
%End

  protected:
};



class QgsLimitedRandomColorRamp : QgsColorRamp
{
%Docstring
 Constrained random color ramp, which returns random colors based on preset parameters.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsLimitedRandomColorRamp( int count = DEFAULT_RANDOM_COUNT,
                               int hueMin = DEFAULT_RANDOM_HUE_MIN, int hueMax = DEFAULT_RANDOM_HUE_MAX,
                               int satMin = DEFAULT_RANDOM_SAT_MIN, int satMax = DEFAULT_RANDOM_SAT_MAX,
                               int valMin = DEFAULT_RANDOM_VAL_MIN, int valMax = DEFAULT_RANDOM_VAL_MAX );
%Docstring
Constructor for QgsLimitedRandomColorRamp

:param count: number of colors in ramp
:param hueMin: minimum hue
:param hueMax: maximum hue
:param satMin: minimum saturation
:param satMax: maximum saturation
:param valMin: minimum color value
:param valMax: maximum color value
%End

    static QgsColorRamp *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Returns a new QgsLimitedRandomColorRamp color ramp created using the properties encoded in a string
map.

:param properties: color ramp properties

.. seealso:: :py:func:`properties`
%End

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;
    virtual QgsLimitedRandomColorRamp *clone() const /Factory/;

    virtual QgsStringMap properties() const;

    virtual int count() const;

    static QList<QColor> randomColors( int count,
                                       int hueMax = DEFAULT_RANDOM_HUE_MAX, int hueMin = DEFAULT_RANDOM_HUE_MIN,
                                       int satMax = DEFAULT_RANDOM_SAT_MAX, int satMin = DEFAULT_RANDOM_SAT_MIN,
                                       int valMax = DEFAULT_RANDOM_VAL_MAX, int valMin = DEFAULT_RANDOM_VAL_MIN );
%Docstring
Get a list of random colors

.. versionadded:: 2.4
%End

    void updateColors();
%Docstring
Must be called after changing the properties of the color ramp
to regenerate the list of random colors.
%End

    int hueMin() const;
%Docstring
Returns the minimum hue for generated colors

.. seealso:: :py:func:`setHueMin`
%End

    int hueMax() const;
%Docstring
Returns the maximum hue for generated colors

.. seealso:: :py:func:`setHueMax`
%End

    int satMin() const;
%Docstring
Returns the minimum saturation for generated colors

.. seealso:: :py:func:`setSatMin`
%End

    int satMax() const;
%Docstring
Returns the maximum saturation for generated colors

.. seealso:: :py:func:`setSatMax`
%End

    int valMin() const;
%Docstring
Returns the minimum value for generated colors

.. seealso:: :py:func:`setValMin`
%End

    int valMax() const;
%Docstring
Returns the maximum value for generated colors

.. seealso:: :py:func:`setValMax`
%End

    void setCount( int val );
%Docstring
Sets the number of colors contained in the ramp.
%End

    void setHueMin( int val );
%Docstring
Sets the minimum hue for generated colors

.. seealso:: :py:func:`hueMin`
%End

    void setHueMax( int val );
%Docstring
Sets the maximum hue for generated colors

.. seealso:: :py:func:`hueMax`
%End

    void setSatMin( int val );
%Docstring
Sets the minimum saturation for generated colors

.. seealso:: :py:func:`satMin`
%End

    void setSatMax( int val );
%Docstring
Sets the maximum saturation for generated colors

.. seealso:: :py:func:`satMax`
%End

    void setValMin( int val );
%Docstring
Sets the minimum value for generated colors

.. seealso:: :py:func:`valMin`
%End

    void setValMax( int val );
%Docstring
Sets the maximum value for generated colors

.. seealso:: :py:func:`valMax`
%End

  protected:
};

class QgsRandomColorRamp: QgsColorRamp
{
%Docstring
 Totally random color ramp. Returns colors generated at random, but constrained
to some hardcoded saturation and value ranges to prevent ugly color generation.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsRandomColorRamp();
%Docstring
Constructor for QgsRandomColorRamp.
%End

    virtual int count() const;


    virtual double value( int index ) const;


    virtual QColor color( double value ) const;


    virtual void setTotalColorCount( const int colorCount );
%Docstring
Sets the desired total number of unique colors for the resultant ramp. Calling
this method pregenerates a set of visually distinct colors which are returned
by subsequent calls to color().

:param colorCount: number of unique colors

.. versionadded:: 2.5
%End

    virtual QString type() const;


    virtual QgsRandomColorRamp *clone() const /Factory/;


    virtual QgsStringMap properties() const;


  protected:


};


class QgsPresetSchemeColorRamp : QgsColorRamp, QgsColorScheme
{
%Docstring
 A scheme based color ramp consisting of a list of predefined colors.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsPresetSchemeColorRamp( const QList< QColor > &colors = QList< QColor >() );
%Docstring
Constructor for QgsPresetSchemeColorRamp.

:param colors: list of colors in ramp
%End

    QgsPresetSchemeColorRamp( const QgsNamedColorList &colors );
%Docstring
Constructor for :py:class:`QgsPresetColorRamp`.

:param colors: list of named colors in ramp

.. note::

   not available in Python bindings - use setColors instead
%End

    static QgsColorRamp *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Returns a new QgsPresetSchemeColorRamp color ramp created using the properties encoded in a string
map.

:param properties: color ramp properties

.. seealso:: :py:func:`properties`
%End

    virtual bool setColors( const QgsNamedColorList &colors, const QString & = QString(), const QColor & = QColor() );
%Docstring
Sets the list of colors used by the ramp.

:param colors: list of colors

.. seealso:: :py:func:`colors`
%End

    QList< QColor > colors() const;
%Docstring
Returns the list of colors used by the ramp.

.. seealso:: :py:func:`setColors`
%End

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;
    virtual void invert();

    virtual QgsPresetSchemeColorRamp *clone() const /Factory/;

    virtual QgsStringMap properties() const;

    virtual int count() const;


    virtual QString schemeName() const;
    virtual QgsNamedColorList fetchColors( const QString &context = QString(), const QColor &baseColor = QColor() );

    virtual bool isEditable() const;

};



class QgsColorBrewerColorRamp : QgsColorRamp
{
%Docstring
 Color ramp utilising "Color Brewer" preset color schemes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsColorBrewerColorRamp( const QString &schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                             int colors = DEFAULT_COLORBREWER_COLORS,
                             bool inverted = false );
%Docstring
Constructor for QgsColorBrewerColorRamp

:param schemeName: color brewer scheme name
:param colors: number of colors in ramp
:param inverted: invert ramp ordering
%End

    static QgsColorRamp *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Returns a new QgsColorBrewerColorRamp color ramp created using the properties encoded in a string
map.

:param properties: color ramp properties

.. seealso:: :py:func:`properties`
%End

    virtual double value( int index ) const;

    virtual QColor color( double value ) const;

    virtual QString type() const;
    virtual void invert();

    virtual QgsColorBrewerColorRamp *clone() const /Factory/;

    virtual QgsStringMap properties() const;

    virtual int count() const;

    QString schemeName() const;
%Docstring
Returns the name of the color brewer color scheme.

.. seealso:: :py:func:`setSchemeName`
%End

    int colors() const;
%Docstring
Returns the number of colors in the ramp.

.. seealso:: :py:func:`setColors`
%End

    void setSchemeName( const QString &schemeName );
%Docstring
Sets the name of the color brewer color scheme.

:param schemeName: scheme name, must match a valid color brewer scheme name

.. seealso:: :py:func:`schemeName`

.. seealso:: :py:func:`listSchemeNames`
%End

    void setColors( int colors );
%Docstring
Sets the number of colors in the ramp.

:param colors: number of colors. Must match a valid value for the scheme,
which can be retrieved using listSchemeVariants()

.. seealso:: :py:func:`colors`
%End

    static QStringList listSchemeNames();
%Docstring
Returns a list of all valid color brewer scheme names.

.. seealso:: :py:func:`listSchemeVariants`
%End

    static QList<int> listSchemeVariants( const QString &schemeName );
%Docstring
Returns a list of the valid variants (numbers of colors) for a specified
color brewer scheme name

:param schemeName: color brewer scheme name

.. seealso:: :py:func:`listSchemeNames`
%End

  protected:

    void loadPalette();
%Docstring
Generates the scheme using the current name and number of colors
%End

};



class QgsCptCityColorRamp : QgsGradientColorRamp
{

%TypeHeaderCode
#include "qgscolorramp.h"
%End
  public:

    QgsCptCityColorRamp( const QString &schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                         const QString &variantName = DEFAULT_CPTCITY_VARIANTNAME,
                         bool inverted = false,
                         bool doLoadFile = true );
%Docstring
Constructor for QgsCptCityColorRamp

:param schemeName: cpt-city scheme name
:param variantName: cpt-city variant name
:param inverted: invert ramp ordering
:param doLoadFile: load cpt-city ramp from file
%End

    QgsCptCityColorRamp( const QString &schemeName, const QStringList &variantList,
                         const QString &variantName = QString(), bool inverted = false,
                         bool doLoadFile = true );
%Docstring
Constructor for QgsCptCityColorRamp

:param schemeName: cpt-city scheme name
:param variantList: cpt-city variant list
:param variantName: cpt-city variant name
:param inverted: invert ramp ordering
:param doLoadFile: load cpt-city ramp from file
%End

    static QgsColorRamp *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;

    virtual QString type() const;

    virtual void invert();


    virtual QgsCptCityColorRamp *clone() const /Factory/;

    void copy( const QgsCptCityColorRamp *other );
    QgsGradientColorRamp *cloneGradientRamp() const /Factory/;

    virtual QgsStringMap properties() const;


    QString schemeName() const;
    QString variantName() const;
    QStringList variantList() const;

    void setSchemeName( const QString &schemeName );
    void setVariantName( const QString &variantName );
    void setVariantList( const QStringList &variantList );
    void setName( const QString &schemeName, const QString &variantName = QString(), const QStringList &variantList = QStringList() );

    void loadPalette();
    bool hasMultiStops() const;

    QString fileName() const;
    bool loadFile();
    bool fileLoaded() const;

    QString copyingFileName() const;
    QString descFileName() const;
    QgsStringMap copyingInfo() const;

  protected:

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscolorramp.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
