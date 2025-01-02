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
#include "qgstextformat.h"
#include <QColor>
#include <QFont>
#include <QPen>
#include <QBrush>

class QgsNumericFormat;
class QgsLineSymbol;
class QgsFillSymbol;

/**
 * \class QgsScaleBarSettings
 * \ingroup core
 * \brief The QgsScaleBarSettings class stores the appearance and layout settings
 * for scalebar drawing with QgsScaleBarRenderer.
*/
class CORE_EXPORT QgsScaleBarSettings
{
  public:

    QgsScaleBarSettings();

    ~QgsScaleBarSettings();

    QgsScaleBarSettings( const QgsScaleBarSettings &other );

    QgsScaleBarSettings &operator=( const QgsScaleBarSettings &other );

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
     * Returns the number of subdivisions for segments included in the right part of the scalebar (only used for some scalebar types).
     *
     * \note The number of subdivisions represents the number of subdivision segments, not the number of subdivision lines. E.g.
     * if the number is 1 then NO subdivision lines will be shown.
     *
     * \see setNumberOfSubdivisions()
     * \since QGIS 3.14
     */
    int numberOfSubdivisions() const { return mNumSubdivisions; }

    /**
     * Sets the number of \a subdivisions for segments included in the right part of the scalebar (only used for some scalebar types).
     *
     * \note The number of subdivisions represents the number of subdivision segments, not the number of subdivision lines. E.g.
     * if the number is 1 then NO subdivision lines will be shown.
     *
     * \see numberOfSubdivisions()
     * \since QGIS 3.14
     */
    void setNumberOfSubdivisions( int subdivisions ) { mNumSubdivisions = subdivisions; }

    /**
     * Returns the scalebar subdivisions height (in millimeters) for segments included in the right part of the scalebar (only used for some scalebar types).
     * \see setSubdivisionsHeight()
     * \since QGIS 3.14
     */
    double subdivisionsHeight() const { return mSubdivisionsHeight; }

    /**
     * Sets the scalebar subdivisions \a height (in millimeters) for segments included in the right part of the scalebar (only used for some scalebar types).
     * \see subdivisionsHeight()
     * \since QGIS 3.14
     */
    void setSubdivisionsHeight( double height ) { mSubdivisionsHeight = height; }

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
    Qgis::ScaleBarSegmentSizeMode segmentSizeMode() const { return mSegmentSizeMode; }

    /**
     * Sets the size \a mode for scale bar segments.
     * \see segmentSizeMode()
     * \see setMinimumBarWidth()
     * \see setMaximumBarWidth()
     */
    void setSegmentSizeMode( Qgis::ScaleBarSegmentSizeMode mode ) { mSegmentSizeMode = mode; }

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
    Qgis::DistanceUnit units() const { return mUnits; }

    /**
     * Sets the distance \a units used by the scalebar.
     * \see units()
     */
    void setUnits( Qgis::DistanceUnit units ) { mUnits = units; }

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
     * \deprecated QGIS 3.40. Use textFormat() instead.
     */
    Q_DECL_DEPRECATED QFont font() const SIP_DEPRECATED { return mTextFormat.font(); }

    /**
     * Sets the \a font used for drawing text in the scalebar.
     * \see font()
     * \deprecated QGIS 3.40. Use setTextFormat() instead.
     */
    Q_DECL_DEPRECATED void setFont( const QFont &font ) SIP_DEPRECATED
    {
      mTextFormat.setFont( font );
      if ( font.pointSizeF() > 0 )
      {
        mTextFormat.setSize( font.pointSizeF() );
        mTextFormat.setSizeUnit( Qgis::RenderUnit::Points );
      }
      else if ( font.pixelSize() > 0 )
      {
        mTextFormat.setSize( font.pixelSize() );
        mTextFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
      }
    }

    /**
     * Returns the color used for drawing text in the scalebar.
     * \see setFontColor()
     * \see font()
     * \deprecated QGIS 3.40. Use textFormat() instead.
     */
    Q_DECL_DEPRECATED QColor fontColor() const SIP_DEPRECATED { return mTextFormat.color(); }

    /**
     * Sets the \a color used for drawing text in the scalebar.
     * \see fontColor()
     * \see setFont()
     * \deprecated QGIS 3.40. Use textFormat() instead.
     */
    Q_DECL_DEPRECATED void setFontColor( const QColor &color ) SIP_DEPRECATED { mTextFormat.setColor( color ); }

    /**
     * Returns the color used for fills in the scalebar.
     * \see setFillColor()
     * \see fillColor2()
     * \deprecated QGIS 3.40. Use fillSymbol() instead.
     */
    Q_DECL_DEPRECATED QColor fillColor() const SIP_DEPRECATED;

    /**
     * Sets the \a color used for fills in the scalebar.
     * \see fillColor()
     * \see setFillColor2()
     * \deprecated QGIS 3.40. Use setFillSymbol() instead.
     */
    Q_DECL_DEPRECATED void setFillColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the secondary color used for fills in the scalebar.
     * \see setFillColor2()
     * \see fillColor()
     * \deprecated QGIS 3.40. Use alternateFillSymbol() instead.
     */
    Q_DECL_DEPRECATED QColor fillColor2() const SIP_DEPRECATED;

    /**
     * Sets the secondary \a color used for fills in the scalebar.
     * \see fillColor2()
     * \see setFillColor2()
     * \deprecated QGIS 3.40. Use setAlternateFillSymbol() instead.
     */
    Q_DECL_DEPRECATED void setFillColor2( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the color used for lines in the scalebar.
     * \see setLineColor()
     * \deprecated QGIS 3.40. Use lineSymbol() instead.
     */
    Q_DECL_DEPRECATED QColor lineColor() const SIP_DEPRECATED;

    /**
     * Sets the \a color used for lines in the scalebar.
     * \see lineColor()
     * \deprecated QGIS 3.40. Use setLineSymbol() instead.
     */
    Q_DECL_DEPRECATED void setLineColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the line width in millimeters for lines in the scalebar.
     * \see setLineWidth()
     * \deprecated QGIS 3.40. Use lineSymbol() instead.
     */
    Q_DECL_DEPRECATED double lineWidth() const SIP_DEPRECATED;

    /**
     * Sets the line \a width in millimeters for lines in the scalebar.
     * \see lineWidth()
     * \deprecated QGIS 3.40. Use setLineSymbol() instead.
     */
    Q_DECL_DEPRECATED void setLineWidth( double width ) SIP_DEPRECATED;

    /**
     * Returns the pen used for drawing outlines in the scalebar.
     * \see setPen()
     * \see brush()
     * \deprecated QGIS 3.40. Use lineSymbol() instead.
     */
    Q_DECL_DEPRECATED QPen pen() const SIP_DEPRECATED;

    /**
     * Sets the pen used for drawing outlines in the scalebar.
     * \see pen()
     * \deprecated QGIS 3.40. Use setLineSymbol() instead.
     */
    Q_DECL_DEPRECATED void setPen( const QPen &pen ) SIP_DEPRECATED;

    /**
     * Returns the line symbol used to render the scalebar (only used for some scalebar types).
     *
     * Ownership is not transferred.
     *
     * \see setLineSymbol()
     * \see divisionLineSymbol()
     * \see subdivisionLineSymbol()
     * \since QGIS 3.14
     */
    QgsLineSymbol *lineSymbol() const;

    /**
     * Sets the line \a symbol used to render the scalebar (only used for some scalebar types). Ownership of \a symbol is
     * transferred to the scalebar.
     *
     * \see lineSymbol()
     * \see setDivisionLineSymbol()
     * \see setSubdivisionLineSymbol()
     * \since QGIS 3.14
     */
    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the scalebar divisions (only used for some scalebar types).
     *
     * Ownership is not transferred.
     *
     * \see setDivisionLineSymbol()
     * \see lineSymbol()
     * \see subdivisionLineSymbol()
     * \since QGIS 3.14
     */
    QgsLineSymbol *divisionLineSymbol() const;

    /**
     * Sets the line \a symbol used to render the scalebar divisions (only used for some scalebar types). Ownership of \a symbol is
     * transferred to the scalebar.
     *
     * \see divisionLineSymbol()
     * \see setLineSymbol()
     * \see setSubdivisionLineSymbol()
     * \since QGIS 3.14
     */
    void setDivisionLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used to render the scalebar subdivisions (only used for some scalebar types).
     *
     * Ownership is not transferred.
     *
     * \see setSubdivisionLineSymbol()
     * \see lineSymbol()
     * \see divisionLineSymbol()
     * \since QGIS 3.14
     */
    QgsLineSymbol *subdivisionLineSymbol() const;

    /**
     * Sets the line \a symbol used to render the scalebar subdivisions (only used for some scalebar types). Ownership of \a symbol is
     * transferred to the scalebar.
     *
     * \see subdivisionLineSymbol()
     * \see setLineSymbol()
     * \see setDivisionLineSymbol()
     * \since QGIS 3.14
     */
    void setSubdivisionLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the primary fill symbol used to render the scalebar (only used for some scalebar types).
     *
     * Ownership is not transferred.
     *
     * \see setFillSymbol()
     * \see alternateFillSymbol()
     * \since QGIS 3.14
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Sets the primary fill \a symbol used to render the scalebar (only used for some scalebar types). Ownership of \a symbol is
     * transferred to the scalebar.
     *
     * \see fillSymbol()
     * \see setAlternateFillSymbol()
     * \since QGIS 3.14
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );


    /**
     * Returns the secondary fill symbol used to render the scalebar (only used for some scalebar types).
     *
     * Ownership is not transferred.
     *
     * \see setAlternateFillSymbol()
     * \see fillSymbol()
     * \since QGIS 3.14
     */
    QgsFillSymbol *alternateFillSymbol() const;

    /**
     * Sets the secondary fill \a symbol used to render the scalebar (only used for some scalebar types). Ownership of \a symbol is
     * transferred to the scalebar.
     *
     * \see alternateFillSymbol()
     * \see setFillSymbol()
     * \since QGIS 3.14
     */
    void setAlternateFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the primary brush used for filling the scalebar.
     * \see setBrush()
     * \see brush2()
     * \see pen()
     * \deprecated QGIS 3.40. Use fillSymbol() instead.
     */
    Q_DECL_DEPRECATED QBrush brush() const SIP_DEPRECATED;

    /**
     * Sets the primary brush used for filling the scalebar.
     * \see brush()
     * \deprecated QGIS 3.40. Use setFillSymbol() instead.
     */
    Q_DECL_DEPRECATED void setBrush( const QBrush &brush ) SIP_DEPRECATED;

    /**
     * Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * \see setBrush2()
     * \see brush()
     * \deprecated QGIS 3.40. Use alternateFillSymbol() instead.
     */
    Q_DECL_DEPRECATED QBrush brush2() const SIP_DEPRECATED;

    /**
     * Sets the secondary brush used for filling the scalebar.
     * \see brush()
     * \deprecated QGIS 3.40. Use setAlternateFillSymbol() instead.
     */
    Q_DECL_DEPRECATED void setBrush2( const QBrush &brush ) SIP_DEPRECATED;

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
     * Returns the vertical placement of text labels.
     * \see setLabelVerticalPlacement()
     * \since QGIS 3.10
     */
    Qgis::ScaleBarDistanceLabelVerticalPlacement labelVerticalPlacement() const { return mLabelVerticalPlacement; }

    /**
     * Sets the vertical \a placement of text labels.
     * \see labelVerticalPlacement()
     * \since QGIS 3.10
     */
    void setLabelVerticalPlacement( Qgis::ScaleBarDistanceLabelVerticalPlacement placement ) { mLabelVerticalPlacement = placement; }

    /**
     * Returns the horizontal placement of text labels.
     * \see setLabelHorizontalPlacement()
     * \since QGIS 3.10
     */
    Qgis::ScaleBarDistanceLabelHorizontalPlacement labelHorizontalPlacement() const { return mLabelHorizontalPlacement; }

    /**
     * Sets the horizontal \a placement of text labels.
     * \see labelHorizontalPlacement()
     * \since QGIS 3.10
     */
    void setLabelHorizontalPlacement( Qgis::ScaleBarDistanceLabelHorizontalPlacement placement ) { mLabelHorizontalPlacement = placement; }

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
    Qgis::ScaleBarAlignment alignment() const { return mAlignment; }

    /**
     * Sets the scalebar \a alignment.
     * \see alignment()
     */
    void setAlignment( Qgis::ScaleBarAlignment alignment ) { mAlignment = alignment; }

    /**
     * Returns the join style used for drawing lines in the scalebar.
     * \see setLineJoinStyle()
     * \deprecated QGIS 3.40. Use lineSymbol() instead.
     */
    Q_DECL_DEPRECATED Qt::PenJoinStyle lineJoinStyle() const SIP_DEPRECATED;

    /**
     * Sets the join \a style used when drawing the lines in the scalebar
     * \see lineJoinStyle()
     * \deprecated QGIS 3.40. Use setLineSymbol() instead.
     */
    Q_DECL_DEPRECATED void setLineJoinStyle( Qt::PenJoinStyle style ) SIP_DEPRECATED;

    /**
     * Returns the cap style used for drawing lines in the scalebar.
     * \see setLineCapStyle()
     * \deprecated QGIS 3.40. Use lineSymbol() instead.
     */
    Q_DECL_DEPRECATED Qt::PenCapStyle lineCapStyle() const SIP_DEPRECATED;

    /**
     * Sets the cap \a style used when drawing the lines in the scalebar.
     * \see lineCapStyle()
     * \deprecated QGIS 3.40. Use setLineSymbol() instead.
     */
    Q_DECL_DEPRECATED void setLineCapStyle( Qt::PenCapStyle style ) SIP_DEPRECATED;

    /**
     * Returns the numeric format used for numbers in the scalebar.
     *
     * \see setNumericFormat()
     * \since QGIS 3.12
     */
    const QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for numbers in the scalebar.
     *
     * Ownership of \a format is transferred to the settings.
     *
     * \see numericFormat()
     * \since QGIS 3.12
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

  private:

    //! Number of segments on right side
    int mNumSegments = 2;
    //! Number of segments on left side
    int mNumSegmentsLeft = 0;
    //! Number of subdivisions on right side
    int mNumSubdivisions = 1;
    //! Height of subdivisions on right side
    double mSubdivisionsHeight = 1.5;
    //! Size of a segment (in map units)
    double mNumUnitsPerSegment = 0;
    //! Number of map units per scale bar units (e.g. 1000 to have km for a map with m units)
    double mNumMapUnitsPerScaleBarUnit = 1.0;
    //! Either fixed (i.e. mNumUnitsPerSegment) or try to best fit scale bar width (mMinBarWidth, mMaxBarWidth)
    Qgis::ScaleBarSegmentSizeMode mSegmentSizeMode = Qgis::ScaleBarSegmentSizeMode::Fixed;
    //! Minimum allowed bar width, when mSegmentSizeMode is FitWidth
    double mMinBarWidth = 50.0;
    //! Maximum allowed bar width, when mSegmentSizeMode is FitWidth
    double mMaxBarWidth = 150.0;

    //! Labeling of map units
    QString mUnitLabeling;

    //! Text format
    QgsTextFormat mTextFormat;

    //! Height of bars/lines
    double mHeight = 3.0;

    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsLineSymbol > mDivisionLineSymbol;
    std::unique_ptr< QgsLineSymbol > mSubdivisionLineSymbol;
    std::unique_ptr< QgsFillSymbol > mFillSymbol;
    std::unique_ptr< QgsFillSymbol > mAlternateFillSymbol;

    //! Space between bar and Text labels
    double mLabelBarSpace = 3.0;
    //! Label's vertical placement
    Qgis::ScaleBarDistanceLabelVerticalPlacement mLabelVerticalPlacement = Qgis::ScaleBarDistanceLabelVerticalPlacement::AboveSegment;
    //! Label's horizontal placement
    Qgis::ScaleBarDistanceLabelHorizontalPlacement mLabelHorizontalPlacement = Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredEdge;

    //! Space between content and item box
    double mBoxContentSpace = 1.0;

    Qgis::ScaleBarAlignment mAlignment = Qgis::ScaleBarAlignment::Left;

    Qgis::DistanceUnit mUnits = Qgis::DistanceUnit::Meters;

    std::unique_ptr< QgsNumericFormat > mNumericFormat;

};

#endif // QGSSCALEBARSETTINGS_H

