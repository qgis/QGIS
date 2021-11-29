/***************************************************************************
  qgsmapboxglstyleconverter.h
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPBOXGLSTYLECONVERTER_H
#define QGSMAPBOXGLSTYLECONVERTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsproperty.h"
#include <QVariantMap>
#include <memory>
#include <QImage>

class QgsVectorTileRenderer;
class QgsVectorTileLabeling;
class QgsVectorTileBasicRendererStyle;
class QgsVectorTileBasicLabelingStyle;

/**
 * Context for a MapBox GL style conversion operation.
 * \warning This is private API only, and may change in future QGIS versions
 * \ingroup core
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMapBoxGlStyleConversionContext
{
  public:

    /**
     * Pushes a \a warning message generated during the conversion.
     */
    void pushWarning( const QString &warning );

    /**
     * Returns a list of warning messages generated during the conversion.
     */
    QStringList warnings() const { return mWarnings; }

    /**
     * Clears the list of warning messages.
     */
    void clearWarnings() { mWarnings.clear(); }

    /**
     * Returns the target unit type.
     *
     * By default this is QgsUnitTypes::RenderPixels in order to exactly match the original
     * style rendering. But rendering in pixels can cause issues on hidpi displays or with print
     * layouts, so setting a target unit of QgsUnitTypes::Millimeters or another real-world unit
     * type is often more appropriate.
     *
     * \see setTargetUnit()
     */
    QgsUnitTypes::RenderUnit targetUnit() const;

    /**
     * Sets the target unit type.
     *
     * By default this is QgsUnitTypes::RenderPixels in order to exactly match the original
     * style rendering. But rendering in pixels can cause issues on hidpi displays or with print
     * layouts, so setting a target unit of QgsUnitTypes::Millimeters or another real-world unit
     * type is often more appropriate.
     *
     * If setting to a non-pixel unit, be sure to call setPixelSizeConversionFactor() in order
     * to setup an appropriate pixel-to-unit conversion factor to scale converted sizes
     * using. E.g. if the target unit is millimeters, the size conversion factor should be
     * set to a pixel-to-millimeter value.
     *
     * \see targetUnit()
     */
    void setTargetUnit( QgsUnitTypes::RenderUnit targetUnit );

    /**
     * Returns the pixel size conversion factor, used to scale the original pixel sizes
     * when converting styles.
     *
     * \see setPixelSizeConversionFactor()
     */
    double pixelSizeConversionFactor() const;

    /**
     * Sets the pixel size conversion factor, used to scale the original pixel sizes
     * when converting styles.
     *
     * \see pixelSizeConversionFactor()
     */
    void setPixelSizeConversionFactor( double sizeConversionFactor );

    /**
     * Returns the sprite image to use during conversion, or an invalid image if this is not set.
     *
     * \see spriteDefinitions()
     * \see setSprites()
     */
    QImage spriteImage() const;

    /**
     * Returns the sprite definitions to use during conversion.
     *
     * \see spriteImage()
     * \see setSprites()
     */
    QVariantMap spriteDefinitions() const;

    /**
     * Sets the sprite \a image and \a definitions JSON to use during conversion.
     *
     * \see spriteImage()
     * \see spriteDefinitions()
     */
    void setSprites( const QImage &image, const QVariantMap &definitions );

    /**
     * Sets the sprite \a image and \a definitions JSON string to use during conversion.
     *
     * \see spriteImage()
     * \see spriteDefinitions()
     */
    void setSprites( const QImage &image, const QString &definitions );

    /**
     * Returns the layer ID of the layer currently being converted.
     *
     * \see setLayerId()
     */
    QString layerId() const;

    /**
     * Sets the layer ID of the layer currently being converted.
     *
     * \see layerId()
     */
    void setLayerId( const QString &value );

  private:

    QStringList mWarnings;

    QString mLayerId;

    QgsUnitTypes::RenderUnit mTargetUnit = QgsUnitTypes::RenderPixels;

    double mSizeConversionFactor = 1.0;

    QImage mSpriteImage;
    QVariantMap mSpriteDefinitions;
};

/**
 * \ingroup core
 * \brief Handles conversion of MapBox GL styles to QGIS vector tile renderers and labeling
 * settings.
 *
 * Conversions are performed by calling convert() with either a JSON map or JSON
 * string value, and then retrieving the results by calling renderer() or labeling()
 * respectively.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMapBoxGlStyleConverter
{
  public:

    /**
     * Constructor for QgsMapBoxGlStyleConverter.
     */
    QgsMapBoxGlStyleConverter();

    //! QgsMapBoxGlStyleConverter cannot be copied
    QgsMapBoxGlStyleConverter( const QgsMapBoxGlStyleConverter &other ) = delete;
    //! QgsMapBoxGlStyleConverter cannot be copied
    QgsMapBoxGlStyleConverter &operator=( const QgsMapBoxGlStyleConverter &other ) = delete;

    ~QgsMapBoxGlStyleConverter();

    //! Result of conversion
    enum Result
    {
      Success = 0, //!< Conversion was successful
      NoLayerList = 1, //!< No layer list was found in JSON input
    };

    /**
     * Converts a JSON \a style map, and returns the resultant status of the conversion.
     *
     * If an error occurs during conversion then a descriptive error message can be retrieved
     * by calling errorMessage().
     *
     * After conversion, the resultant labeling and style rules can be retrieved by calling
     * renderer() or labeling() respectively.
     *
     * The optional \a context argument can be set to use a specific context during the conversion.
     */
    Result convert( const QVariantMap &style, QgsMapBoxGlStyleConversionContext *context = nullptr );

    /**
     * Converts a JSON \a style string, and returns the resultant status of the conversion.
     *
     * If an error occurs during conversion then a descriptive error message can be retrieved
     * by calling errorMessage().
     *
     * After conversion, the resultant labeling and style rules can be retrieved by calling
     * renderer() or labeling() respectively.
     *
     * The optional \a context argument can be set to use a specific context during the conversion.
     */
    Result convert( const QString &style, QgsMapBoxGlStyleConversionContext *context = nullptr );

    /**
     * Returns a descriptive error message if an error was encountered during the style conversion,
     * or an empty string if no error was encountered.
     *
     * \see warnings()
     */
    QString errorMessage() const { return mError; }

    /**
     * Returns a list of user-friendly warnings generated during the conversion, e.g. as a result
     * of MapBox GL style settings which cannot be translated to QGIS styles.
     *
     * \see errorMessage()
     */
    QStringList warnings() const { return mWarnings; }

    /**
     * Returns a new instance of a vector tile renderer representing the converted style,
     * or NULLPTR if the style could not be converted successfully.
     */
    QgsVectorTileRenderer *renderer() const SIP_FACTORY;

    /**
     * Returns a new instance of a vector tile labeling representing the converted style,
     * or NULLPTR if the style could not be converted successfully.
     */
    QgsVectorTileLabeling *labeling() const SIP_FACTORY;

  protected:

    /**
     * Property types, for interpolated value conversion
     * \warning This is private API only, and may change in future QGIS versions
     */
    enum PropertyType
    {
      Color, //!< Color property
      Numeric, //!< Numeric property (e.g. line width, text size)
      Opacity, //!< Opacity property
      Point, //!< Point/offset property
    };

    /**
     * Parse list of \a layers from JSON.
     * \warning This is private API only, and may change in future QGIS versions
     */
    void parseLayers( const QVariantList &layers, QgsMapBoxGlStyleConversionContext *context = nullptr );

    /**
     * Parses a fill layer.
     *
     * \warning This is private API only, and may change in future QGIS versions
     *
     * \param jsonLayer fill layer to parse
     * \param style generated QGIS vector tile style
     * \param context conversion context
     * \param isBackgroundStyle set to TRUE if the layer should be parsed as background layer
     * \returns TRUE if the layer was successfully parsed.
     */
    static bool parseFillLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style SIP_OUT, QgsMapBoxGlStyleConversionContext &context, bool isBackgroundStyle = false );

    /**
     * Parses a line layer.
     *
     * \warning This is private API only, and may change in future QGIS versions
     *
     * \param jsonLayer line layer to parse
     * \param style generated QGIS vector tile style
     * \param context conversion context
     * \returns TRUE if the layer was successfully parsed.
     */
    static bool parseLineLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style SIP_OUT, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a circle layer.
     *
     * \warning This is private API only, and may change in future QGIS versions
     *
     * \param jsonLayer circle layer to parse
     * \param style generated QGIS vector tile style
     * \param context conversion context
     * \returns TRUE if the layer was successfully parsed.
     */
    static bool parseCircleLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style SIP_OUT, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a symbol layer as renderer or labeling.
     *
     * \warning This is private API only, and may change in future QGIS versions
     *
     * \param jsonLayer symbol layer to parse
     * \param rendererStyle generated QGIS vector tile style
     * \param hasRenderer will be set to TRUE if symbol layer generated a renderer style
     * \param labelingStyle generated QGIS vector tile labeling
     * \param hasLabeling will be set to TRUE if symbol layer generated a labeling style
     * \param context conversion context
    */
    static void parseSymbolLayer( const QVariantMap &jsonLayer,
                                  QgsVectorTileBasicRendererStyle &rendererStyle SIP_OUT,
                                  bool &hasRenderer SIP_OUT,
                                  QgsVectorTileBasicLabelingStyle &labelingStyle SIP_OUT,
                                  bool &hasLabeling SIP_OUT, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a symbol layer as a renderer
     *
     * \warning This is private API only, and may change in future QGIS versions
     *
     * \param jsonLayer fill layer to parse
     * \param rendererStyle generated QGIS vector tile style
     * \param context conversion context
     *
     * \returns TRUE if symbol layer was converted to renderer
    */
    static bool parseSymbolLayerAsRenderer( const QVariantMap &jsonLayer,
                                            QgsVectorTileBasicRendererStyle &rendererStyle SIP_OUT, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a color value which is interpolated by zoom range.
     *
     * \param json definition of color interpolation
     * \param context conversion context
     * \param defaultColor optional storage for a reasonable "default" color representing the overall property.
     *
     * \returns QgsProperty representing interpolation settings
     */
    static QgsProperty parseInterpolateColorByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, QColor *defaultColor SIP_OUT = nullptr );

    /**
     * Parses a numeric value which is interpolated by zoom range.
     *
     * \param json definition of interpolation
     * \param context conversion context
     * \param multiplier optional multiplication factor
     * \param defaultNumber optional storage for a reasonable "default" number representing the overall property.
     *
     * \returns QgsProperty representing interpolation settings
     */
    static QgsProperty parseInterpolateByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1, double *defaultNumber SIP_OUT = nullptr );

    /**
     * Interpolates opacity with either scale_linear() or scale_exp() (depending on base value).
     * For \a json with intermediate stops it uses parseOpacityStops() function.
     * It uses QGIS set_color_part() function to set alpha component of color.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseInterpolateOpacityByZoom( const QVariantMap &json, int maxOpacity, QgsMapBoxGlStyleConversionContext *contextPtr = 0 );

    /**
     * Takes values from stops and uses either scale_linear() or scale_exp() functions
     * to interpolate alpha component of color.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QString parseOpacityStops( double base, const QVariantList &stops, int maxOpacity, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Interpolates a point/offset with either scale_linear() or scale_exp() (depending on base value).
     * For \a json with intermediate stops it uses parsePointStops() function.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseInterpolatePointByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1, QPointF *defaultPoint SIP_OUT = nullptr );

    /**
     * Interpolates a string by zoom.
     * For \a json with intermediate stops it uses parseStringStops() function.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseInterpolateStringByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context,
        const QVariantMap &conversionMap,
        QString *defaultString SIP_OUT = nullptr );


    /**
     * Takes values from stops and uses either scale_linear() or scale_exp() functions
     * to interpolate point/offset values.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QString parsePointStops( double base, const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1 );

    /**
     * Takes numerical arrays from stops.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QString parseArrayStops( const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1 );

    /**
     * Parses a list of interpolation stops
     *
     * \param base interpolation exponent base
     * \param stops definition of interpolation stops
     * \param multiplier optional multiplication factor
     * \param context conversion context
     */
    static QString parseStops( double base, const QVariantList &stops, double multiplier, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a list of interpolation stops containing string values.
     *
     * \param stops definition of interpolation stops
     * \param context conversion context
     * \param conversionMap map of input string to output expression value
     * \param defaultString reasonable default value taken from stops
     *
     * \returns converted expression
     */
    static QString parseStringStops( const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context,
                                     const QVariantMap &conversionMap,
                                     QString *defaultString SIP_OUT = nullptr );


    /**
     * Parses and converts a value list (e.g. an interpolate list).
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseValueList( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1,
                                       int maxOpacity = 255, QColor *defaultColor SIP_OUT = nullptr, double *defaultNumber SIP_OUT = nullptr );


    /**
     * Parses and converts a match function value list.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseMatchList( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1,
                                       int maxOpacity = 255, QColor *defaultColor SIP_OUT = nullptr, double *defaultNumber SIP_OUT = nullptr );

    /**
     * Interpolates a list which starts with the interpolate function.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QgsProperty parseInterpolateListByZoom( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier = 1,
        int maxOpacity = 255, QColor *defaultColor SIP_OUT = nullptr, double *defaultNumber SIP_OUT = nullptr );

    /**
     * Converts an expression representing a color to a string (can be color string or an expression where a color is expected)
     * \param colorExpression the color expression
     * \param context the style conversion context
     * \returns the QGIS expression string
     * since QGIS 3.22
     */
    static QString parseColorExpression( const QVariant &colorExpression, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Parses a \a color in one of these supported formats:
     *
     * - \c \#fff or \c \#ffffff
     * - ``hsl(30, 19%, 90%)`` or ``hsla(30, 19%, 90%, 0.4)``
     * - ``rgb(10, 20, 30)`` or ``rgba(10, 20, 30, 0.5)``
     *
     * Returns an invalid color if the color could not be parsed.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QColor parseColor( const QVariant &color, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Takes a QColor object and returns HSLA components in required format for QGIS color_hsla() expression function.
     * \param color input color
     * \param hue an integer value from 0 to 360
     * \param saturation an integer value from 0 to 100
     * \param lightness an integer value from 0 to 100
     * \param alpha an integer value from 0 (completely transparent) to 255 (opaque).
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static void colorAsHslaComponents( const QColor &color, int &hue, int &saturation, int &lightness, int &alpha );

    /**
     * Generates an interpolation for values between \a valueMin and \a valueMax, scaled between the
     * ranges \a zoomMin to \a zoomMax.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QString interpolateExpression( double zoomMin, double zoomMax, QVariant valueMin, QVariant valueMax, double base, double multiplier = 1, QgsMapBoxGlStyleConversionContext *contextPtr = 0 );

    /**
     * Converts a value to Qt::PenCapStyle enum from JSON value.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static Qt::PenCapStyle parseCapStyle( const QString &style );

    /**
     * Converts a value to Qt::PenJoinStyle enum from JSON value.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static Qt::PenJoinStyle parseJoinStyle( const QString &style );

    /**
     * Converts a MapBox GL expression to a QGIS expression.
     *
     * \warning This is private API only, and may change in future QGIS versions
     */
    static QString parseExpression( const QVariantList &expression, QgsMapBoxGlStyleConversionContext &context, bool colorExpected = false );

    /**
     * Retrieves the sprite image with the specified \a name, taken from the specified \a context.
     *
     * The \a context must have valid sprite definitions and images set via QgsMapBoxGlStyleConversionContext::setSprites()
     * prior to conversion.
     */
    static QImage retrieveSprite( const QString &name, QgsMapBoxGlStyleConversionContext &context, QSize &spriteSize );

    /**
     * Retrieves the sprite image with the specified \a name, taken from the specified \a context as a base64 encoded value
     *
     * The \a context must have valid sprite definitions and images set via QgsMapBoxGlStyleConversionContext::setSprites()
     * prior to conversion.
     */
    static QString retrieveSpriteAsBase64( const QVariant &value, QgsMapBoxGlStyleConversionContext &context, QSize &spriteSize, QString &spriteProperty, QString &spriteSizeProperty );

  private:

#ifdef SIP_RUN
    QgsMapBoxGlStyleConverter( const QgsMapBoxGlStyleConverter &other );
#endif

    static QString parseValue( const QVariant &value, QgsMapBoxGlStyleConversionContext &context, bool colorExpected = false );

    static QString parseKey( const QVariant &value, QgsMapBoxGlStyleConversionContext &context );

    /**
     * Checks if interpolation bottom/top values are numeric values
     * \param bottomVariant bottom value
     * \param topVariant top value
     * \param bottom out: bottom value converted to double
     * \param top out: top value converted to double
     * \return true if both bottom and top value are numeric. False else (e.g. if one of the values is an expression)
     */
    static bool numericArgumentsOnly( const QVariant &bottomVariant, const QVariant &topVariant, double &bottom, double &top );

    QString mError;
    QStringList mWarnings;

    std::unique_ptr< QgsVectorTileRenderer > mRenderer;
    std::unique_ptr< QgsVectorTileLabeling > mLabeling;

};

#endif // QGSMAPBOXGLSTYLECONVERTER_H
