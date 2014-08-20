/***************************************************************************
  qgstextrenderer.h
  -------------------
   begin                : August 2014
   copyright            : (C) Nyall Dawson, Martin Dobias
   email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTRENDERER_H
#define QGSTEXTRENDERER_H

#include "qgsmapunitscale.h"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QFontDatabase>

class CORE_EXPORT QgsTextRendererSettings
{
  public:
    QgsTextRendererSettings();
    QgsTextRendererSettings( const QgsTextRendererSettings& s );
    virtual ~QgsTextRendererSettings();

    enum MultiLineAlign
    {
      MultiLeft = 0,
      MultiCenter,
      MultiRight
    };

    enum ShapeType
    {
      ShapeRectangle = 0,
      ShapeSquare,
      ShapeEllipse,
      ShapeCircle,
      ShapeSVG
    };

    enum SizeType
    {
      SizeBuffer = 0,
      SizeFixed,
      SizePercent
    };

    enum RotationType
    {
      RotationSync = 0,
      RotationOffset,
      RotationFixed
    };

    /** Units used for option sizes, before being converted to rendered sizes */
    enum SizeUnit
    {
      Points = 0,
      MM,
      MapUnits,
      Percent
    };

    enum ShadowType
    {
      ShadowLowest = 0,
      ShadowText,
      ShadowBuffer,
      ShadowShape
    };

    //-- text style
    QFont textFont;
    QString textNamedStyle;
    bool fontSizeInMapUnits; //true if font size is in map units (otherwise in points)
    QgsMapUnitScale fontSizeMapUnitScale; // scale range for map units for font size
    QColor textColor;
    int textTransp;
    QPainter::CompositionMode blendMode;
    QColor previewBkgrdColor;

    //-- text formatting
    QString wrapChar;
    double multilineHeight; //0.0 to 10.0, leading between lines as multiplyer of line height
    MultiLineAlign multilineAlign; // horizontal alignment of multi-line labels

    //-- text buffer

    bool bufferDraw;
    double bufferSize; // buffer size
    bool bufferSizeInMapUnits; //true if buffer is in map units (otherwise in mm)
    QgsMapUnitScale bufferSizeMapUnitScale; // scale range for map units for buffer size
    QColor bufferColor;
    bool bufferNoFill; //set interior of buffer to 100% transparent
    int bufferTransp;
    Qt::PenJoinStyle bufferJoinStyle;
    QPainter::CompositionMode bufferBlendMode;

    //-- shape background

    bool shapeDraw;
    ShapeType shapeType;
    QString shapeSVGFile;
    SizeType shapeSizeType;
    QPointF shapeSize;
    SizeUnit shapeSizeUnits;
    QgsMapUnitScale shapeSizeMapUnitScale;
    RotationType shapeRotationType;
    double shapeRotation;
    QPointF shapeOffset;
    SizeUnit shapeOffsetUnits;
    QgsMapUnitScale shapeOffsetMapUnitScale;
    QPointF shapeRadii;
    SizeUnit shapeRadiiUnits;
    QgsMapUnitScale shapeRadiiMapUnitScale;
    int shapeTransparency;
    QPainter::CompositionMode shapeBlendMode;
    QColor shapeFillColor;
    QColor shapeBorderColor;
    double shapeBorderWidth;
    SizeUnit shapeBorderWidthUnits;
    QgsMapUnitScale shapeBorderWidthMapUnitScale;
    Qt::PenJoinStyle shapeJoinStyle;

    //-- drop shadow

    bool shadowDraw;
    ShadowType shadowUnder;
    int shadowOffsetAngle;
    double shadowOffsetDist;
    SizeUnit shadowOffsetUnits;
    QgsMapUnitScale shadowOffsetMapUnitScale;
    bool shadowOffsetGlobal;
    double shadowRadius;
    SizeUnit shadowRadiusUnits;
    QgsMapUnitScale shadowRadiusMapUnitScale;
    bool shadowRadiusAlphaOnly;
    int shadowTransparency;
    int shadowScale;
    QColor shadowColor;
    QPainter::CompositionMode shadowBlendMode;

    //-- scale factors
    double vectorScaleFactor; //scale factor painter units->pixels
    double rasterCompressFactor; //pixel resolution scale factor

    /** Calculates size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @param mapUnitScale a mapUnitScale clamper
     * @return size that will render, as double
     * @note added in 1.9, as a better precision replacement for sizeToPixel
     */
    double scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor = false, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() ) const;

    /** Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @param mapUnitScale a mapUnitScale clamper
     * @return font pixel size
     */
    int sizeToPixel( double size, const QgsRenderContext& c , SizeUnit unit, bool rasterfactor = false, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() ) const;

    // temporary stuff: set when layer gets prepared or labeled
    // NOTE: not in Python binding

    const QgsMapToPixel* xform;
    const QgsCoordinateTransform* ct;
    QgsPoint ptZero, ptOne;

    QString mTextFontFamily;
    bool mTextFontFound;

    bool showingShadowRects; // whether to show debug rectangles for drop shadows

  protected:

    QFontDatabase mFontDB;
};

/** \ingroup core
  * Maintains current state of more grainular and temporal values when creating/painting
  * component parts of an individual label (e.g. buffer, background, shadow, etc.).
  */
class CORE_EXPORT QgsLabelComponent
{
  public:
    QgsLabelComponent()
        : mText( QString() )
        , mOrigin( QgsPoint() )
        , mUseOrigin( false )
        , mRotation( 0.0 )
        , mRotationOffset( 0.0 )
        , mUseRotation( false )
        , mCenter( QgsPoint() )
        , mUseCenter( false )
        , mSize( QgsPoint() )
        , mOffset( QgsPoint() )
        , mPicture( 0 )
        , mPictureBuffer( 0.0 )
        , mDpiRatio( 1.0 )
    {}

    // methods

    const QString& text() { return mText; }
    void setText( const QString& text ) { mText = text; }

    const QgsPoint& origin() { return mOrigin; }
    void setOrigin( QgsPoint point ) { mOrigin = point; }

    bool useOrigin() const { return mUseOrigin; }
    void setUseOrigin( bool use ) { mUseOrigin = use; }

    double rotation() const { return mRotation; }
    void setRotation( double rotation ) { mRotation = rotation; }

    double rotationOffset() const { return mRotationOffset; }
    void setRotationOffset( double rotation ) { mRotationOffset = rotation; }

    bool useRotation() const { return mUseRotation; }
    void setUseRotation( bool use ) { mUseRotation = use; }

    const QgsPoint& center() { return mCenter; }
    void setCenter( QgsPoint point ) { mCenter = point; }

    bool useCenter() const { return mUseCenter; }
    void setUseCenter( bool use ) { mUseCenter = use; }

    const QgsPoint& size() { return mSize; }
    void setSize( QgsPoint point ) { mSize = point; }

    const QgsPoint& offset() { return mOffset; }
    void setOffset( QgsPoint point ) { mOffset = point; }

    const QPicture* picture() { return mPicture; }
    void setPicture( QPicture* picture ) { mPicture = picture; }

    double pictureBuffer() const { return mPictureBuffer; }
    void setPictureBuffer( double buffer ) { mPictureBuffer = buffer; }

    double dpiRatio() const { return mDpiRatio; }
    void setDpiRatio( double ratio ) { mDpiRatio = ratio; }

  private:
    // current label component text,
    // e.g. single line in a multi-line label or charcater in curved labeling
    QString mText;
    // current origin point for painting (generally current painter rotation point)
    QgsPoint mOrigin;
    // whether to translate the painter to supplied origin
    bool mUseOrigin;
    // any rotation to be applied to painter (in radians)
    double mRotation;
    // any rotation to be applied to painter (in radians) after initial rotation
    double mRotationOffset;
    // whether to use the rotation to rotate the painter
    bool mUseRotation;
    // current center point of label compnent, after rotation
    QgsPoint mCenter;
    // whether to translate the painter to supplied origin based upon center
    bool mUseCenter;
    // width and height of label component, transformed and ready for painting
    QgsPoint mSize;
    // any translation offsets to be applied before painting, transformed and ready for painting
    QgsPoint mOffset;

    // a stored QPicture of painting for the component
    QPicture* mPicture;
    // buffer for component to accommodate graphic items ignored by QPicture,
    // e.g. half-width of an applied QPen, which would extend beyond boundingRect() of QPicture
    double mPictureBuffer;

    // a ratio of native painter dpi and that of rendering context's painter
    double mDpiRatio;
};

class CORE_EXPORT QgsTextRenderer
{
  public:

    enum TextComponentPart
    {
      TextPart = 0,
      LabelPart,
      ShapePart,
      SVGPart,
      ShadowPart
    };

    /**Draws text using the specified settings
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param layerSettings pal layer settings
     * @param dpiRatio scaling for dpi
     * @param drawAsOutlines set to true to draw text as outlines, rather than
     * text objects. Drawing as outlines is required for correct rendering of buffered text
     * @note added in QGIS 2.5
     */
    static void drawText( const QRectF rect, const double rotation, const QStringList textLines,
                          QgsRenderContext &context, const QgsTextRendererSettings &textSettings,
                          const double dpiRatio = 1.0, const bool drawAsOutlines = true );

    static void drawText( const QRectF rect, const double rotation, const QString text,
                          QgsRenderContext &context, QgsTextRendererSettings &textSettings,
                          const double dpiRatio = 1.0, const bool drawAsOutlines = true );

    /**Draws a part of a label using the specified settings
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param layerSettings Pal layer settings
     * @param drawType part of label to draw
     * @param dpiRatio scaling for dpi
     * @note added in QGIS 2.5
     */
    static void drawPart( const QRectF rect, const double rotation, const QStringList textLines,
                          QgsRenderContext& context, QgsTextRendererSettings& textSettings,
                          const TextComponentPart componentPart, const double dpiRatio, const bool drawAsOutlines );

    static void drawBackgroundPart( QgsRenderContext& context,
                                    QgsLabelComponent component,
                                    const QgsTextRendererSettings& textSettings );

    static void drawShadowPart( QgsRenderContext& context,
                                QgsLabelComponent component,
                                const QgsTextRendererSettings& textSettings );

    static void drawBufferPart( QgsRenderContext& context,
                                QgsLabelComponent component,
                                const QgsTextRendererSettings& textSettings );

    static void drawTextPart( const QgsPoint point, const QSizeF size, const bool drawFromTop, const QStringList textLines,
                              const TextComponentPart drawType, QgsLabelComponent component,
                              const QgsTextRendererSettings& settings, const QFontMetricsF* fontMetrics,
                              QgsRenderContext& context, const bool drawAsOutlines );

};

#endif // QGSTEXTRENDERER_H
