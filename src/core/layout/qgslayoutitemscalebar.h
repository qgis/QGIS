/***************************************************************************
                            qgslayoutitemscalebar.h
                            ------------------------
    begin                : November 2017
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
#ifndef QGSLAYOUTITEMSCALEBAR_H
#define QGSLAYOUTITEMSCALEBAR_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgslayoutitem.h"
#include "scalebar/qgsscalebarsettings.h"
#include "scalebar/qgsscalebarrenderer.h"
#include <QFont>
#include <QPen>
#include <QColor>

class QgsLayoutItemMap;

/**
 * \ingroup core
 * A layout item subclass for scale bars.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemScaleBar: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemScaleBar, with the specified parent \a layout.
     */
    QgsLayoutItemScaleBar( QgsLayout *layout );

    int type() const override;
    QIcon icon() const override;

    /**
     * Returns a new scale bar item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemScaleBar *create( QgsLayout *layout ) SIP_FACTORY;
    QgsLayoutSize minimumSize() const override;

    /**
     * Returns the number of segments included in the scalebar.
     * \see setNumberOfSegments()
     * \see numberOfSegmentsLeft()
     */
    int numberOfSegments() const { return mSettings.numberOfSegments(); }

    /**
     * Sets the number of \a segments included in the scalebar.
     * \see numberOfSegments()
     * \see setNumberOfSegmentsLeft()
     */
    void setNumberOfSegments( int segments );

    /**
     * Returns the number of segments included in the left part of the scalebar.
     * \see setNumberOfSegmentsLeft()
     * \see numberOfSegments()
     */
    int numberOfSegmentsLeft() const { return mSettings.numberOfSegmentsLeft(); }

    /**
     * Sets the number of \a segments included in the left part of the scalebar.
     * \see numberOfSegmentsLeft()
     * \see setNumberOfSegments()
     */
    void setNumberOfSegmentsLeft( int segments );

    /**
     * Returns the number of scalebar units per segment.
     * \see setUnitsPerSegment()
     */
    double unitsPerSegment() const { return mSettings.unitsPerSegment(); }

    /**
     * Sets the number of scalebar \a units per segment.
     * \see unitsPerSegment()
     */
    void setUnitsPerSegment( double units );

    /**
     * Returns the size mode for the scale bar segments.
     * \see setSegmentSizeMode()
     * \see minimumBarWidth()
     * \see maximumBarWidth()
     */
    QgsScaleBarSettings::SegmentSizeMode segmentSizeMode() const { return mSettings.segmentSizeMode(); }

    /**
     * Sets the size \a mode for scale bar segments.
     * \see segmentSizeMode()
     * \see setMinimumBarWidth()
     * \see setMaximumBarWidth()
     */
    void setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode );

    /**
     * Returns the minimum width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode()
     * \see setMinimumBarWidth()
     * \see maximumBarWidth()
     */
    double minimumBarWidth() const { return mSettings.minimumBarWidth(); }

    /**
     * Sets the minimum \a width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see minimumBarWidth()
     * \see setMaximumBarWidth()
     * \see setSegmentSizeMode()
     */
    void setMinimumBarWidth( double minWidth );

    /**
     * Returns the maximum width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode()
     * \see setMaximumBarWidth()
     * \see minimumBarWidth()
     */
    double maximumBarWidth() const { return mSettings.maximumBarWidth(); }

    /**
     * Sets the maximum \a width (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see minimumBarWidth()
     * \see setMinimumBarWidth()
     * \see setSegmentSizeMode()
     */
    void setMaximumBarWidth( double maxWidth );

    /**
     * Returns the number of map units per scale bar unit used by the scalebar.
     * \see setMapUnitsPerScaleBarUnit()
     */
    double mapUnitsPerScaleBarUnit() const { return mSettings.mapUnitsPerScaleBarUnit(); }

    /**
     * Sets the number of map \a units per scale bar unit used by the scalebar.
     * \see mapUnitsPerScaleBarUnit()
     */
    void setMapUnitsPerScaleBarUnit( double units ) { mSettings.setMapUnitsPerScaleBarUnit( units ); }

    /**
     * Returns the label for units.
     * \see setUnitLabel()
     */
    QString unitLabel() const { return mSettings.unitLabel(); }

    /**
     * Sets the \a label for units.
     * \see unitLabel()
     */
    void setUnitLabel( const QString &label ) { mSettings.setUnitLabel( label );}

    /**
     * Returns the text format used for drawing text in the scalebar.
     * \see setTextFormat()
     * \since QGIS 3.2
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format used for drawing text in the scalebar.
     * \see textFormat()
     * \since QGIS 3.2
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Returns the font used for drawing text in the scalebar.
     * \see setFont()
     * \deprecated use textFormat() instead
     */
    Q_DECL_DEPRECATED QFont font() const SIP_DEPRECATED;

    /**
     * Sets the \a font used for drawing text in the scalebar.
     * \see font()
     * \deprecated use setTextFormat() instead
     */
    Q_DECL_DEPRECATED void setFont( const QFont &font ) SIP_DEPRECATED;

    /**
     * Returns the color used for drawing text in the scalebar.
     * \see setFontColor()
     * \see font()
     * \deprecated use textFormat() instead
     */
    Q_DECL_DEPRECATED QColor fontColor() const SIP_DEPRECATED;

    /**
     * Sets the \a color used for drawing text in the scalebar.
     * \see fontColor()
     * \see setFont()
     * \deprecated use setTextFormat() instead
     */
    Q_DECL_DEPRECATED void setFontColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the color used for fills in the scalebar.
     * \see setFillColor()
     * \see fillColor2()
     */
    QColor fillColor() const { return mSettings.fillColor(); }

    /**
     * Sets the \a color used for fills in the scalebar.
     * \see fillColor()
     * \see setFillColor2()
     */
    void setFillColor( const QColor &color ) { mSettings.setFillColor( color ); }

    /**
     * Returns the secondary color used for fills in the scalebar.
     * \see setFillColor2()
     * \see fillColor()
     */
    QColor fillColor2() const { return mSettings.fillColor2(); }

    /**
     * Sets the secondary \a color used for fills in the scalebar.
     * \see fillColor2()
     * \see setFillColor2()
     */
    void setFillColor2( const QColor &color ) { mSettings.setFillColor2( color ); }

    /**
     * Returns the color used for lines in the scalebar.
     * \see setLineColor()
     */
    QColor lineColor() const { return mSettings.lineColor(); }

    /**
     * Sets the \a color used for lines in the scalebar.
     * \see lineColor()
     */
    void setLineColor( const QColor &color ) { mSettings.setLineColor( color ); }

    /**
     * Returns the line width in millimeters for lines in the scalebar.
     * \see setLineWidth()
     */
    double lineWidth() const { return mSettings.lineWidth(); }

    /**
     * Sets the line \a width in millimeters for lines in the scalebar.
     * \see lineWidth()
     */
    void setLineWidth( double width ) { mSettings.setLineWidth( width ); }

    /**
     * Returns the pen used for drawing outlines in the scalebar.
     * \see brush()
     */
    QPen pen() const { return mSettings.pen(); }

    /**
     * Returns the primary brush for the scalebar.
     * \returns QBrush used for filling the scalebar
     * \see brush2
     * \see pen
     */
    QBrush brush() const {return mSettings.brush();}

    /**
     * Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * \returns QBrush used for secondary color areas
     * \see brush
     */
    QBrush brush2() const {return mSettings.brush2(); }

    /**
     * Returns the scalebar height (in millimeters).
     * \see setHeight()
     */
    double height() const { return mSettings.height(); }

    /**
     * Sets the scalebar \a height (in millimeters).
     * \see height()
     */
    void setHeight( double height ) { mSettings.setHeight( height ); }

    /**
     * Sets the \a map item linked to the scalebar.
     * \see linkedMap()
     */
    void setLinkedMap( QgsLayoutItemMap *map );

    /**
     * Returns the map item linked to the scalebar.
     * \see setLinkedMap()
     */
    QgsLayoutItemMap *linkedMap() const { return mMap; }

    /**
     * Returns the spacing (in millimeters) between labels and the scalebar.
     * \see setLabelBarSpace()
     */
    double labelBarSpace() const { return mSettings.labelBarSpace(); }

    /**
     * Sets the spacing (in millimeters) between labels and the scalebar.
     * \see labelBarSpace()
     */
    void setLabelBarSpace( double space ) {mSettings.setLabelBarSpace( space );}

    /**
     * Returns the spacing (margin) between the scalebar box and content in millimeters.
     * \see setBoxContentSpace()
     */
    double boxContentSpace() const { return mSettings.boxContentSpace(); }

    /**
     * Sets the \a space (margin) between the scalebar box and content in millimeters.
     * \see boxContentSpace()
     */
    void setBoxContentSpace( double space );

    /**
     * Returns the scalebar alignment.
     * \see setAlignment()
     */
    QgsScaleBarSettings::Alignment alignment() const { return mSettings.alignment(); }

    /**
     * Sets the scalebar \a alignment.
     * \see alignment()
     */
    void setAlignment( QgsScaleBarSettings::Alignment alignment );

    /**
     * Returns the distance units used by the scalebar.
     * \see setUnits()
     */
    QgsUnitTypes::DistanceUnit units() const { return mSettings.units(); }

    /**
     * Sets the distance \a units used by the scalebar.
     * \see units()
     */
    void setUnits( QgsUnitTypes::DistanceUnit units );

    /**
     * Returns the join style used for drawing lines in the scalebar.
     * \see setLineJoinStyle()
     */
    Qt::PenJoinStyle lineJoinStyle() const { return mSettings.lineJoinStyle(); }

    /**
     * Sets the join \a style used when drawing the lines in the scalebar
     * \see lineJoinStyle()
     */
    void setLineJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the cap style used for drawing lines in the scalebar.
     * \see setLineCapStyle()
     */
    Qt::PenCapStyle lineCapStyle() const { return mSettings.lineCapStyle(); }

    /**
     * Sets the cap \a style used when drawing the lines in the scalebar.
     * \see lineCapStyle()
     */
    void setLineCapStyle( Qt::PenCapStyle style );

    /**
     * Applies the default scalebar settings to the scale bar.
     * \see applyDefaultSize()
     */
    void applyDefaultSettings();

    /**
     * Attempts to guess the most reasonable unit choice for the scalebar, given
     * the current linked map's scale.
     *
     * This method also considers the linked map's CRS, in order to determine if
     * metric or imperial units are more appropriate.
     */
    QgsUnitTypes::DistanceUnit guessUnits() const;

    /**
     * Applies the default size to the scale bar (scale bar 1/5 of map item width)
     * \see applyDefaultSettings()
     */
    void applyDefaultSize( QgsUnitTypes::DistanceUnit units = QgsUnitTypes::DistanceMeters );

    /**
     * Resizes the scale bar to its minimum width, without changing the height.
     */
    void resizeToMinimumWidth();

    /**
     * Sets the scale bar style by \a name.
     *
     * The \a name parameter gives the (untranslated) style name.
     * Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle',
     * 'Line Ticks Down', 'Line Ticks Up', 'Numeric'
     *
     * \see style()
    */
    void setStyle( const QString &name );

    /**
     * Returns the scale bar style name.
     * \see setStyle()
     */
    QString style() const;

    /**
     * Adjusts the scale bar box size and updates the item.
     */
    void update();

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties ) override;
    void finalizeRestoreFromXml() override;
  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private slots:
    void updateScale();
    void disconnectCurrentMap();

  private:

    //! Linked map
    QgsLayoutItemMap *mMap = nullptr;
    QString mMapUuid;

    QgsScaleBarSettings mSettings;

    //! Scalebar style
    std::unique_ptr< QgsScaleBarRenderer > mStyle;

    //! Width of a segment (in mm)
    double mSegmentMillimeters = 0.0;

    //! Calculates with of a segment in mm and stores it in mSegmentMillimeters
    void refreshSegmentMillimeters();

    //! Returns diagonal of composer map in selected units (map units / meters / feet / nautical miles)
    double mapWidth() const;

    QgsScaleBarRenderer::ScaleBarContext createScaleContext() const;

    friend class QgsCompositionConverter;

};

#endif //QGSLAYOUTITEMSCALEBAR_H


