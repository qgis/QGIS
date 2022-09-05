/***************************************************************************
  qgstextformat.h
  ---------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTFORMAT_H
#define QGSTEXTFORMAT_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsunittypes.h"
#include "qgstextbuffersettings.h"
#include "qgstextbackgroundsettings.h"
#include "qgstextshadowsettings.h"
#include "qgstextmasksettings.h"
#include "qgsstringutils.h"

#include <QSharedDataPointer>

class QMimeData;
class QgsTextSettingsPrivate;

/**
 * \class QgsTextFormat
  * \ingroup core
  * \brief Container for all settings relating to text rendering.
  * \note QgsTextFormat objects are implicitly shared.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsTextFormat
{
  public:

    /**
     * Default constructor for QgsTextFormat. Creates a text format initially
     * set to an invalid state (see isValid()).
     */
    QgsTextFormat();

    /**
     * Copy constructor.
     * \param other source QgsTextFormat
     */
    QgsTextFormat( const QgsTextFormat &other );

    QgsTextFormat &operator=( const QgsTextFormat &other );

    ~QgsTextFormat();

    bool operator==( const QgsTextFormat &other ) const;
    bool operator!=( const QgsTextFormat &other ) const;

    /**
     * Returns TRUE if the format is valid.
     *
     * A default constructed QgsTextFormat is invalid, until at least one or more properties
     * have been set on the format. An invalid state can be used as a representation of a "not set"
     * text format, e.g. for indicating that a default text format should be used.
     *
     * \note Calling any setter on a QgsTextFormat object will automatically set the format as valid.
     *
     * \see setValid()
     * \since QGIS 3.16
     */
    bool isValid() const;

    /**
     * Sets the format to a valid state, without changing any of the default format settings.
     *
     * \see isValid()
     * \since QGIS 3.16
     */
    void setValid();

    /**
     * Returns a reference to the text buffer settings.
     * \see setBuffer()
     */
    QgsTextBufferSettings &buffer();

    /**
     * Returns a reference to the text buffer settings.
     * \see setBuffer()
     */
    SIP_SKIP QgsTextBufferSettings buffer() const { return mBufferSettings; }

    /**
     * Sets the text's buffer settings.
     * \param bufferSettings buffer settings
     * \see buffer()
     */
    void setBuffer( const QgsTextBufferSettings &bufferSettings );

    /**
     * Returns a reference to the text background settings.
     * \see setBackground()
     */
    QgsTextBackgroundSettings &background();

    /**
     * Returns a reference to the text background settings.
     * \see setBackground()
     */
    SIP_SKIP QgsTextBackgroundSettings background() const { return mBackgroundSettings; }

    /**
     * Sets the text's background settings.q
     * \param backgroundSettings background settings
     * \see background()
     */
    void setBackground( const QgsTextBackgroundSettings &backgroundSettings );

    /**
     * Returns a reference to the text drop shadow settings.
     * \see setShadow()
     */
    QgsTextShadowSettings &shadow();

    /**
     * Returns a reference to the text drop shadow settings.
     * \see setShadow()
     */
    SIP_SKIP QgsTextShadowSettings shadow() const { return mShadowSettings; }

    /**
     * Sets the text's drop shadow settings.
     * \param shadowSettings shadow settings
     * \see shadow()
     */
    void setShadow( const QgsTextShadowSettings &shadowSettings );

    /**
     * Returns a reference to the masking settings.
     * \see setMask()
     */
    QgsTextMaskSettings &mask();

    /**
     * Returns a reference to the masking settings.
     * Masks may be defined in contexts where the text is rendered over some map layers, for labeling especially.
     * \see setMask()
     * \since QGIS 3.12
     */
    SIP_SKIP QgsTextMaskSettings mask() const { return mMaskSettings; }

    /**
     * Sets the text's masking settings.
     * Masks may be defined in contexts where the text is rendered over some map layers, for labeling especially.
     * \param maskSettings mask settings
     * \see mask()
     * \since QGIS 3.12
     */
    void setMask( const QgsTextMaskSettings &maskSettings );

    /**
     * Returns the font used for rendering text. Note that the size of the font
     * is not used, and size() should be called instead to determine the size
     * of rendered text.
     * \see scaledFont()
     * \see setFont()
     * \see namedStyle()
     * \see toQFont()
     */
    QFont font() const;

    /**
     * Returns a font with the size scaled to match the format's size settings (including
     * units and map unit scale) for a specified render context.
     * \param context destination render context
     * \param scaleFactor optional font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::FONT_WORKAROUND_SCALE and then manually scale painter devices or calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     * \param isZeroSize will be set to true if the font is scaled down to a near 0 size, and nothing should be rendered. Not available in Python bindings.
     * \returns font with scaled size
     * \see font()
     * \see size()
     */
    QFont scaledFont( const QgsRenderContext &context, double scaleFactor = 1.0, bool *isZeroSize SIP_PYARGREMOVE = nullptr ) const;

    /**
     * Sets the font used for rendering text. Note that the size of the font
     * is not used, and setSize() should be called instead to explicitly set the size
     * of rendered text.
     * \param font desired font
     * \see font()
     * \see setNamedStyle()
     * \see fromQFont()
     */
    void setFont( const QFont &font );

    /**
     * Returns the named style for the font used for rendering text (e.g., "bold").
     * \see setNamedStyle()
     * \see font()
     */
    QString namedStyle() const;

    /**
     * Sets the named style for the font used for rendering text.
     * \param style named style, e.g., "bold"
     * \see namedStyle()
     * \see setFont()
     */
    void setNamedStyle( const QString &style );

    /**
     * Returns TRUE if the format is set to force a bold style.
     *
     * \warning Unlike setting a font's style via setNamedStyle(), this will ensure that a font is
     * always rendered in bold regardless of whether the font family actually has a bold variant. A
     * "faux bold" effect will be emulated, which may result in poor quality font rendering. For this
     * reason it is greatly preferred to call setNamedStyle() instead.
     *
     * \see setForcedBold()
     * \since QGIS 3.26
     */
    bool forcedBold() const;

    /**
     * Sets whether the format is set to force a bold style.
     *
     * \warning Unlike setting a font's style via setNamedStyle(), this will ensure that a font is
     * always rendered in bold regardless of whether the font family actually has a bold variant. A
     * "faux bold" effect will be emulated, which may result in poor quality font rendering. For this
     * reason it is greatly preferred to call setNamedStyle() instead.
     *
     * \see forcedBold()
     * \since QGIS 3.26
     */
    void setForcedBold( bool forced );

    /**
     * Returns TRUE if the format is set to force an italic style.
     *
     * \warning Unlike setting a font's style via setNamedStyle(), this will ensure that a font is
     * always rendered in italic regardless of whether the font family actually has an italic variant. A
     * "faux italic" slanted text effect will be emulated, which may result in poor quality font rendering. For this
     * reason it is greatly preferred to call setNamedStyle() instead.
     *
     * \see setForcedItalic()
     * \since QGIS 3.26
     */
    bool forcedItalic() const;

    /**
     * Sets whether the format is set to force an italic style.
     *
     * \warning Unlike setting a font's style via setNamedStyle(), this will ensure that a font is
     * always rendered in italic regardless of whether the font family actually has an italic variant. A
     * "faux italic" slanted text effect will be emulated, which may result in poor quality font rendering. For this
     * reason it is greatly preferred to call setNamedStyle() instead.
     *
     * \see forcedItalic()
     * \since QGIS 3.26
     */
    void setForcedItalic( bool forced );

    /**
     * Returns the list of font families to use when restoring the text format, in order of precedence.
     *
     * \warning The list of families returned by this method is ONLY used when restoring the text format
     * from serialized versions, and will not affect the current font() familily used by the format.
     *
     * \see setFamilies()
     * \since QGIS 3.20
     */
    QStringList families() const;

    /**
     * Sets a list of font \a families to use for the text format, in order of precedence.
     *
     * When restoring serialized versions of the text format then the first matching font family
     * from this list will be used for the text format. This provides a way to specify a list of possible
     * font families which are used as fallbacks if a family isn't available on a particular QGIS install (CSS style).
     *
     * \warning The list of families set by calling this method is ONLY used when restoring the text format
     * from serialized versions, and will not affect the current font() familily used by the format.
     *
     * \see families()
     * \since QGIS 3.20
     */
    void setFamilies( const QStringList &families );

    /**
     * Returns the size for rendered text. Units are retrieved using sizeUnit().
     * \see setSize()
     * \see sizeUnit()
     */
    double size() const;

    /**
     * Sets the size for rendered text.
     * \param size size of rendered text. Units are set using setSizeUnit()
     * \see size()
     * \see setSizeUnit()
     */
    void setSize( double size );

    /**
     * Returns the units for the size of rendered text.
     * \see size()
     * \see setSizeUnit()
     * \see sizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the units for the size of rendered text.
     * \param unit size units
     * \see setSize()
     * \see sizeUnit()
     * \see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Sets the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the color that text will be rendered in.
     * \see setColor()
     */
    QColor color() const;

    /**
     * Sets the color that text will be rendered in.
     * \param color text color
     * \see color()
     */
    void setColor( const QColor &color );

    /**
     * Returns the text's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the text's opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the text's stretch factor.
     *
     * The stretch factor matches a condensed or expanded version of the font or applies a stretch
     * transform that changes the width of all characters in the font by factor percent.
     *
     * For example, a factor of 150 results in all characters in the font being 1.5 times
     * (ie. 150%) wider. The minimum stretch factor is 1, and the maximum stretch factor is 4000.
     *
     * \see setStretchFactor()
     * \since QGIS 3.24
     */
    int stretchFactor() const;

    /**
     * Sets the text's stretch \a factor.
     *
     * The stretch factor matches a condensed or expanded version of the font or applies a stretch
     * transform that changes the width of all characters in the font by factor percent.
     *
     * For example, setting \a factor to 150 results in all characters in the font being 1.5 times
     * (ie. 150%) wider. The minimum stretch factor is 1, and the maximum stretch factor is 4000.
     *
     * \see stretchFactor()
     * \since QGIS 3.24
     */
    void setStretchFactor( int factor );

    /**
     * Returns the blending mode used for drawing the text.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the blending mode used for drawing the text.
     * \param mode blending mode
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /**
     * Returns the line height for text.
     *
     * If lineHeightUnit() is QgsUnitTypes::RenderPercentage (the default), then this is a number representing
     * the leading between lines as a multiplier of line height (where 0 - 1.0 represents 0 to 100% of text line height).
     * Otherwise the line height is an absolute measurement in lineHeightUnit().
     *
     * \see setLineHeight()
     * \see lineHeightUnit()
     */
    double lineHeight() const;

    /**
     * Sets the line height for text.
     *
     * If lineHeightUnit() is QgsUnitTypes::RenderPercentage (the default), then \a height is a number representing
     * the leading between lines as a multiplier of line height (where 0 - 1.0 represents 0 to 100% of text line height).
     * Otherwise \a height is an absolute measurement in lineHeightUnit().
     *
     * \see lineHeight()
     * \see setLineHeightUnit()
     */
    void setLineHeight( double height );

    /**
     * Returns the units for the line height for text.
     *
     * \see setLineHeightUnit()
     * \see lineHeight()
     *
     * \since QGIS 3.28
     */
    QgsUnitTypes::RenderUnit lineHeightUnit() const;

    /**
     * Sets the \a unit for the line height for text.
     *
     * \see lineHeightUnit()
     * \see setLineHeight()
     *
     * \since QGIS 3.28
     */
    void setLineHeightUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the orientation of the text.
     * \see setOrientation()
     * \since QGIS 3.10
     */
    Qgis::TextOrientation orientation() const;

    /**
     * Sets the \a orientation for the text.
     * \see orientation()
     * \since QGIS 3.10
     */
    void setOrientation( Qgis::TextOrientation orientation );

    /**
     * Returns the text capitalization style.
     *
     * \see setCapitalization()
     * \since QGIS 3.16
     */
    Qgis::Capitalization capitalization() const;

    /**
     * Sets the text \a capitalization style.
     *
     * \see capitalization()
     * \since QGIS 3.16
     */
    void setCapitalization( Qgis::Capitalization capitalization );

    /**
     * Returns TRUE if text should be treated as a HTML document and HTML tags should be used for formatting
     * the rendered text.
     *
     * \warning Only a small subset of HTML formatting is supported. Currently this is restricted to:
     *
     * - text color formatting
     * - strikethrough
     * - underline
     * - overline
     *
     * \see setAllowHtmlFormatting()
     * \since QGIS 3.14
     */
    bool allowHtmlFormatting() const;

    /**
     * Sets whether text should be treated as a HTML document and HTML tags should be used for formatting
     * the rendered text.
     *
     * \warning Only a small subset of HTML formatting is supported. Currently this is restricted to:
     *
     * - text color formatting
     * - strikethrough
     * - underline
     * - overline
     *
     * \see allowHtmlFormatting()
     * \since QGIS 3.14
     */
    void setAllowHtmlFormatting( bool allow );

    /**
     * Returns the background color for text previews.
     * \see setPreviewBackgroundColor()
     * \since QGIS 3.10
     */
    QColor previewBackgroundColor() const;

    /**
     * Sets the background \a color that text will be rendered on for previews.
     * \see previewBackgroundColor()
     * \since QGIS 3.10
     */
    void setPreviewBackgroundColor( const QColor &color );

    /**
     * Reads settings from a layer's custom properties (for QGIS 2.x projects).
     * \param layer source vector layer
     */
    void readFromLayer( QgsVectorLayer *layer );

    /**
     * Read settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns new mime data representing the text format settings.
     * Caller takes responsibility for deleting the returned object.
     * \see fromMimeData()
     */
    QMimeData *toMimeData() const SIP_FACTORY;

    /**
     * Returns a text format matching the settings from an input \a font.
     * Unlike setFont(), this method also handles the size and size units
     * from \a font.
     * \see toQFont()
     * \since QGIS 3.2
     */
    static QgsTextFormat fromQFont( const QFont &font );

    /**
     * Returns a QFont matching the relevant settings from this text format.
     * Unlike font(), this method also handles the size and size units
     * from the text format.
     * \see fromQFont()
     * \since QGIS 3.2
     */
    QFont toQFont() const;

    /**
     * Attempts to parse the provided mime \a data as a QgsTextFormat.
     * If data can be parsed as a text format, \a ok will be set to TRUE.
     * \see toMimeData()
     */
    static QgsTextFormat fromMimeData( const QMimeData *data, bool *ok SIP_OUT = nullptr );

    /**
     * Returns TRUE if any component of the font format requires advanced effects
     * such as blend modes, which require output in raster formats to be fully respected.
     */
    bool containsAdvancedEffects() const;

    /**
     * Returns TRUE if the specified font was found on the system, or FALSE
     * if the font was not found and a replacement was used instead.
     * \see resolvedFontFamily()
     */
    bool fontFound() const { return mTextFontFound; }

    /**
     * Returns the family for the resolved font, ie if the specified font
     * was not found on the system this will return the name of the replacement
     * font.
     * \see fontFound()
     */
    QString resolvedFontFamily() const { return mTextFontFamily; }

    /**
     * Returns a reference to the format's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.10
     */
    QgsPropertyCollection &dataDefinedProperties();

    /**
     * Returns a reference to the format's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP;

    /**
     * Returns all field names referenced by the configuration (e.g. from data defined properties).
     * \since QGIS 3.14
     */
    QSet<QString> referencedFields( const QgsRenderContext &context ) const;

    /**
     * Sets the format's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \since QGIS 3.10
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection );

    /**
     * Updates the format by evaluating current values of data defined properties.
     * \since QGIS 3.10
     */
    void updateDataDefinedProperties( QgsRenderContext &context );

    /**
    * Returns a pixmap preview for a text \a format.
    * \param format text format
    * \param size target pixmap size
    * \param previewText text to render in preview, or empty for default text
    * \param padding space between icon edge and color ramp
    * \since QGIS 3.10
    */
    static QPixmap textFormatPreviewPixmap( const QgsTextFormat &format, QSize size, const QString &previewText = QString(), int padding = 0 );

  private:

    QgsTextBufferSettings mBufferSettings;
    QgsTextBackgroundSettings mBackgroundSettings;
    QgsTextShadowSettings mShadowSettings;
    QgsTextMaskSettings mMaskSettings;

    QString mTextFontFamily;
    bool mTextFontFound = true;

    QSharedDataPointer<QgsTextSettingsPrivate> d;

};

Q_DECLARE_METATYPE( QgsTextFormat )

#endif // QGSTEXTFORMAT_H
