/***************************************************************************
                            qgsscalebarsettings.h
                            ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEBARSETTINGS_H
#define QGSSCALEBARSETTINGS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsunittypes.h"
#include "qgstextrenderer.h"
#include <QColor>
#include <QFont>
#include <QPen>
#include <QBrush>

/**
 * \class QgsScaleBarSettings
 * \ingroup core
 * The QgsScaleBarSettings class stores the appearance and layout settings
 * for scalebar drawing with QgsScaleBarRenderer.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsScaleBarSettings
{
  public:

    /**
     * Scalebar alignment.
     */
    enum Alignment
    {
      AlignLeft = 0, //!< Left aligned
      AlignMiddle, //!< Center aligned
      AlignRight, //!< Right aligned
    };

    /**
     * Modes for setting size for scale bar segments.
     */
    enum SegmentSizeMode
    {
      SegmentSizeFixed = 0, //!< Scale bar segment size is fixed to a map unit
      SegmentSizeFitWidth = 1 //!< Scale bar segment size is calculated to fit a size range
    };

    /**
     * Constructor for QgsScaleBarSettings.
     */
    QgsScaleBarSettings()
    {
      mPen = QPen( mLineColor );
      mPen.setJoinStyle( mLineJoinStyle );
      mPen.setCapStyle( mLineCapStyle );
      mPen.setWidthF( mLineWidth );

      mBrush.setColor( mFillColor );
      mBrush.setStyle( Qt::SolidPattern );

      mBrush2.setColor( mFillColor2 );
      mBrush2.setStyle( Qt::SolidPattern );

      mTextFormat.setSize( 12.0 );
      mTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
      mTextFormat.setColor( QColor( 0, 0, 0 ) );
    }

    /**
     * Returns the number of segments included in the scalebar.
     * \see setNumberOfSegments()
     * \see numberOfSegmentsLeft()
     */
    int numberOfSegments() const { return mNumSegments; }

    /**
     * Sets the number of \a segments included in the scalebar.
     * \see numberOfSegments()
     * \see setNumberOfSegmentsLeft()
     */
    void setNumberOfSegments( int segments ) { mNumSegments = segments; }

    /**
     * Returns the number of segments included in the left part of the scalebar.
     * \see setNumberOfSegmentsLeft()
     * \see numberOfSegments()
     */
    int numberOfSegmentsLeft() const { return mNumSegmentsLeft; }

    /**
     * Sets the number of \a segments included in the left part of the scalebar.
     * \see numberOfSegmentsLeft()
     * \see setNumberOfSegments()
     */
    void setNumberOfSegmentsLeft( int segments ) { mNumSegmentsLeft = segments; }

    /**
     * Returns the number of scalebar units per segment.
     * \see setUnitsPerSegment()
     */
    double unitsPerSegment() const { return mNumUnitsPerSegment; }

    /**
     * Sets the number of scalebar \a units per segment.
     * \see unitsPerSegment()
     */
    void setUnitsPerSegment( double units ) { mNumUnitsPerSegment = units; }

    /**
     * Returns the size mode for the scale bar segments.
     * \see setSegmentSizeMode()
     * \see minimumBarWidth()
     * \see maximumBarWidth()
     */
    SegmentSizeMode segmentSizeMode() const { return mSegmentSizeMode; }

    /**
     * Sets the size \a mode for scale bar segments.
     * \see segmentSizeMode()
     * \see setMinimumBarWidth()
     * \see setMaximumBarWidth()
     */
    void setSegmentSizeMode( SegmentSizeMode mode ) { mSegmentSizeMode = mode; }

    /**
     * Returns the minimum width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode()
     * \see setMinimumBarWidth()
     * \see maximumBarWidth()
     */
    double minimumBarWidth() const { return mMinBarWidth; }

    /**
     * Sets the minimum \a width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see minimumBarWidth()
     * \see setMaximumBarWidth()
     * \see setSegmentSizeMode()
     */
    void setMinimumBarWidth( double width ) { mMinBarWidth = width; }

    /**
     * Returns the maximum width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode()
     * \see setMaximumBarWidth()
     * \see minimumBarWidth()
     */
    double maximumBarWidth() const { return mMaxBarWidth; }

    /**
     * Sets the maximum \a width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see minimumBarWidth()
     * \see setMinimumBarWidth()
     * \see setSegmentSizeMode()
     */
    void setMaximumBarWidth( double width ) { mMaxBarWidth = width; }

    /**
     * Returns the distance units used by the scalebar.
     * \see setUnits()
     */
    QgsUnitTypes::DistanceUnit units() const { return mUnits; }

    /**
     * Sets the distance \a units used by the scalebar.
     * \see units()
     */
    void setUnits( QgsUnitTypes::DistanceUnit units ) { mUnits = units; }

    /**
     * Returns the number of map units per scale bar unit used by the scalebar.
     * \see setMapUnitsPerScaleBarUnit()
     */
    double mapUnitsPerScaleBarUnit() const { return mNumMapUnitsPerScaleBarUnit; }

    /**
     * Sets the number of map \a units per scale bar unit used by the scalebar.
     * \see mapUnitsPerScaleBarUnit()
     */
    void setMapUnitsPerScaleBarUnit( double units ) { mNumMapUnitsPerScaleBarUnit = units; }

    /**
     * Returns the label for units.
     * \see setUnitLabel()
     */
    QString unitLabel() const { return mUnitLabeling; }

    /**
     * Sets the \a label for units.
     * \see unitLabel()
     */
    void setUnitLabel( const QString &label ) { mUnitLabeling = label; }

    /**
     * Returns the text format used for drawing text in the scalebar.
     * \see setTextFormat()
     * \since QGIS 3.2
     */
    QgsTextFormat &textFormat() { return mTextFormat; }

    /**
     * Returns the text format used for drawing text in the scalebar.
     * \see setTextFormat()
     * \since QGIS 3.2
     */
    QgsTextFormat textFormat() const SIP_SKIP { return mTextFormat; }

    /**
     * Sets the text \a format used for drawing text in the scalebar.
     * \see textFormat()
     * \since QGIS 3.2
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

    /**
     * Returns the font used for drawing text in the scalebar.
     * \see setFont()
     * \deprecated use textFormat() instead
     */
    Q_DECL_DEPRECATED QFont font() const SIP_DEPRECATED { return mTextFormat.font(); }

    /**
     * Sets the \a font used for drawing text in the scalebar.
     * \see font()
     * \deprecated use setTextFormat() instead
     */
    Q_DECL_DEPRECATED void setFont( const QFont &font ) SIP_DEPRECATED
    {
      mTextFormat.setFont( font );
      if ( font.pointSizeF() > 0 )
      {
        mTextFormat.setSize( font.pointSizeF() );
        mTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
      }
      else if ( font.pixelSize() > 0 )
      {
        mTextFormat.setSize( font.pixelSize() );
        mTextFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
      }
    }

    /**
     * Returns the color used for drawing text in the scalebar.
     * \see setFontColor()
     * \see font()
     * \deprecated use textFormat() instead
     */
    Q_DECL_DEPRECATED QColor fontColor() const SIP_DEPRECATED { return mTextFormat.color(); }

    /**
     * Sets the \a color used for drawing text in the scalebar.
     * \see fontColor()
     * \see setFont()
     * \deprecated use textFormat() instead
     */
    Q_DECL_DEPRECATED void setFontColor( const QColor &color ) SIP_DEPRECATED { mTextFormat.setColor( color ); }

    /**
     * Returns the color used for fills in the scalebar.
     * \see setFillColor()
     * \see fillColor2()
     */
    QColor fillColor() const { return mFillColor; }

    /**
     * Sets the \a color used for fills in the scalebar.
     * \see fillColor()
     * \see setFillColor2()
     */
    void setFillColor( const QColor &color ) { mFillColor = color; mBrush.setColor( color ); }

    /**
     * Returns the secondary color used for fills in the scalebar.
     * \see setFillColor2()
     * \see fillColor()
     */
    QColor fillColor2() const {return mFillColor2;}

    /**
     * Sets the secondary \a color used for fills in the scalebar.
     * \see fillColor2()
     * \see setFillColor2()
     */
    void setFillColor2( const QColor &color ) { mFillColor2 = color; mBrush2.setColor( color ); }

    /**
     * Returns the color used for lines in the scalebar.
     * \see setLineColor()
     */
    QColor lineColor() const { return mLineColor; }

    /**
     * Sets the \a color used for lines in the scalebar.
     * \see lineColor()
     */
    void setLineColor( const QColor &color ) { mLineColor = color; mPen.setColor( mLineColor ); }

    /**
     * Returns the line width in millimeters for lines in the scalebar.
     * \see setLineWidth()
     */
    double lineWidth() const { return mLineWidth; }

    /**
     * Sets the line \a width in millimeters for lines in the scalebar.
     * \see lineWidth()
     */
    void setLineWidth( double width ) { mLineWidth = width; mPen.setWidthF( width ); }

    /**
     * Returns the pen used for drawing outlines in the scalebar.
     * \see setPen()
     * \see brush()
     */
    QPen pen() const { return mPen; }

    /**
     * Sets the pen used for drawing outlines in the scalebar.
     * \see pen()
     */
    void setPen( const QPen &pen ) { mPen = pen; }

    /**
     * Returns the primary brush used for filling the scalebar.
     * \see setBrush()
     * \see brush2()
     * \see pen()
     */
    QBrush brush() const { return mBrush; }

    /**
     * Sets the primary brush used for filling the scalebar.
     * \see brush()
     */
    void setBrush( const QBrush &brush ) { mBrush = brush; }

    /**
     * Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * \see setBrush2()
     * \see brush()
     */
    QBrush brush2() const { return mBrush2; }

    /**
     * Sets the secondary brush used for filling the scalebar.
     * \see brush()
     */
    void setBrush2( const QBrush &brush ) { mBrush2 = brush; }

    /**
     * Returns the scalebar height (in millimeters).
     * \see setHeight()
     */
    double height() const { return mHeight; }

    /**
     * Sets the scalebar \a height (in millimeters).
     * \see height()
     */
    void setHeight( double height ) { mHeight = height; }

    /**
     * Returns the spacing (in millimeters) between labels and the scalebar.
     * \see setLabelBarSpace()
     */
    double labelBarSpace() const { return mLabelBarSpace; }

    /**
     * Sets the spacing (in millimeters) between labels and the scalebar.
     * \see labelBarSpace()
     */
    void setLabelBarSpace( double space ) { mLabelBarSpace = space; }

    /**
     * Returns the spacing (margin) between the scalebar box and content in millimeters.
     * \see setBoxContentSpace()
     */
    double boxContentSpace() const { return mBoxContentSpace; }

    /**
     * Sets the \a space (margin) between the scalebar box and content in millimeters.
     * \see boxContentSpace()
     */
    void setBoxContentSpace( double space ) { mBoxContentSpace = space; }

    /**
     * Returns the scalebar alignment.
     * \see setAlignment()
     */
    Alignment alignment() const { return mAlignment; }

    /**
     * Sets the scalebar \a alignment.
     * \see alignment()
     */
    void setAlignment( Alignment alignment ) { mAlignment = alignment; }

    /**
     * Returns the join style used for drawing lines in the scalebar.
     * \see setLineJoinStyle()
     */
    Qt::PenJoinStyle lineJoinStyle() const { return mLineJoinStyle; }

    /**
     * Sets the join \a style used when drawing the lines in the scalebar
     * \see lineJoinStyle()
     */
    void setLineJoinStyle( Qt::PenJoinStyle style ) { mLineJoinStyle = style; mPen.setJoinStyle( style ); }

    /**
     * Returns the cap style used for drawing lines in the scalebar.
     * \see setLineCapStyle()
     */
    Qt::PenCapStyle lineCapStyle() const { return mLineCapStyle; }

    /**
     * Sets the cap \a style used when drawing the lines in the scalebar.
     * \see lineCapStyle()
     */
    void setLineCapStyle( Qt::PenCapStyle style ) { mLineCapStyle = style; mPen.setCapStyle( style ); }

  private:

    //! Number of segments on right side
    int mNumSegments = 2;
    //! Number of segments on left side
    int mNumSegmentsLeft = 0;
    //! Size of a segment (in map units)
    double mNumUnitsPerSegment = 0;
    //! Number of map units per scale bar units (e.g. 1000 to have km for a map with m units)
    double mNumMapUnitsPerScaleBarUnit = 1.0;
    //! Either fixed (i.e. mNumUnitsPerSegment) or try to best fit scale bar width (mMinBarWidth, mMaxBarWidth)
    SegmentSizeMode mSegmentSizeMode = SegmentSizeFixed;
    //! Minimum allowed bar width, when mSegmentSizeMode is FitWidth
    double mMinBarWidth = 50.0;
    //! Maximum allowed bar width, when mSegmentSizeMode is FitWidth
    double mMaxBarWidth = 150.0;

    //! Labeling of map units
    QString mUnitLabeling;

    //! Text format
    QgsTextFormat mTextFormat;

    //! Fill color
    QColor mFillColor = QColor( 0, 0, 0 );
    //! Secondary fill color
    QColor mFillColor2 = QColor( 255, 255, 255 );
    //! Line color
    QColor mLineColor = QColor( 0, 0, 0 );
    //! Line width
    double mLineWidth = 0.3;
    //! Stroke
    QPen mPen;
    //! Fill
    QBrush mBrush;
    //! Secondary fill
    QBrush mBrush2;
    //! Height of bars/lines
    double mHeight = 3.0;

    //! Space between bar and Text labels
    double mLabelBarSpace = 3.0;

    //! Space between content and item box
    double mBoxContentSpace = 1.0;

    Alignment mAlignment = AlignLeft;

    QgsUnitTypes::DistanceUnit mUnits = QgsUnitTypes::DistanceMeters;

    Qt::PenJoinStyle mLineJoinStyle = Qt::MiterJoin;
    Qt::PenCapStyle mLineCapStyle = Qt::SquareCap;

};

#endif // QGSSCALEBARSETTINGS_H

