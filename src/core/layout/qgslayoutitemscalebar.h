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
#include "qgis_sip.h"
#include "qgslayoutitem.h"
#include "scalebar/qgsscalebarsettings.h"
#include "scalebar/qgsscalebarrenderer.h"
#include <QFont>
#include <QPen>
#include <QColor>

class QgsLayoutItemMap;

/**
 * \ingroup core
 * \brief A layout item subclass for scale bars.
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
     * Returns the number of subdivisions for segments included in the right part of the scalebar (only used for some scalebar types).
     *
     * \note The number of subdivisions represents the number of subdivision segments, not the number of subdivision lines. E.g.
     * if the number is 1 then NO subdivision lines will be shown.
     *
     * \see setNumberOfSubdivisions()
     * \since QGIS 3.14
     */
    int numberOfSubdivisions()  const { return mSettings.numberOfSubdivisions(); }

    /**
     * Sets the number of \a subdivisions for segments included in the right part of the scalebar (only used for some scalebar types).
     *
     * \note The number of subdivisions represents the number of subdivision segments, not the number of subdivision lines. E.g.
     * if the number is 1 then NO subdivision lines will be shown.
     *
     * \see numberOfSubdivisions()
     * \since QGIS 3.14
     */
    void setNumberOfSubdivisions( int subdivisions ) { mSettings.setNumberOfSubdivisions( subdivisions ); }

    /**
     * Returns the scalebar subdivisions height (in millimeters) for segments included in the right part of the scalebar (only used for some scalebar types).
     * \see setSubdivisionsHeight()
     * \since QGIS 3.14
     */
    double subdivisionsHeight() const { return mSettings.subdivisionsHeight(); }

    /**
     * Sets the scalebar subdivisions \a height (in millimeters) for segments included in the right part of the scalebar (only used for some scalebar types).
     * \see subdivisionsHeight()
     * \since QGIS 3.14
     */
    void setSubdivisionsHeight( double height ) { mSettings.setSubdivisionsHeight( height ); }

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
     * \deprecated use fillSymbol() instead
     */
    Q_DECL_DEPRECATED QColor fillColor() const SIP_DEPRECATED;

    /**
     * Sets the \a color used for fills in the scalebar.
     * \see fillColor()
     * \see setFillColor2()
     * \deprecated use setFillSymbol() instead
     */
    Q_DECL_DEPRECATED void setFillColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the secondary color used for fills in the scalebar.
     * \see setFillColor2()
     * \see fillColor()
     * \deprecated use alternateFillSymbol() instead
     */
    Q_DECL_DEPRECATED QColor fillColor2() const SIP_DEPRECATED;

    /**
     * Sets the secondary \a color used for fills in the scalebar.
     * \see fillColor2()
     * \see setFillColor2()
     * \deprecated use setAlternateFillSymbol() instead
     */
    Q_DECL_DEPRECATED void setFillColor2( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the color used for lines in the scalebar.
     * \see setLineColor()
     * \deprecated use lineSymbol() instead
     */
    Q_DECL_DEPRECATED QColor lineColor() const SIP_DEPRECATED;

    /**
     * Sets the \a color used for lines in the scalebar.
     * \see lineColor()
     * \deprecated use setLineSymbol() instead
     */
    Q_DECL_DEPRECATED void setLineColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Returns the line width in millimeters for lines in the scalebar.
     * \see setLineWidth()
     * \deprecated use lineSymbol() instead
     */
    Q_DECL_DEPRECATED double lineWidth() const SIP_DEPRECATED;

    /**
     * Sets the line \a width in millimeters for lines in the scalebar.
     * \see lineWidth()
     * \deprecated use setLineSymbol() instead
     */
    Q_DECL_DEPRECATED void setLineWidth( double width ) SIP_DEPRECATED;

    /**
     * Returns the pen used for drawing outlines in the scalebar.
     * \see brush()
     * \deprecated use lineSymbol() instead
     */
    Q_DECL_DEPRECATED QPen pen() const SIP_DEPRECATED;

    /**
     * Returns the primary brush for the scalebar.
     * \returns QBrush used for filling the scalebar
     * \see brush2
     * \see pen
     * \deprecated use fillSymbol() instead
     */
    Q_DECL_DEPRECATED QBrush brush() const SIP_DEPRECATED;

    /**
     * Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * \returns QBrush used for secondary color areas
     * \see brush
     * \deprecated use alternateFillSymbol() instead
     */
    Q_DECL_DEPRECATED QBrush brush2() const SIP_DEPRECATED;

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
     * Returns the vertical placement of text labels.
     * \see setLabelVerticalPlacement()
     * \since QGIS 3.10
     */
    QgsScaleBarSettings::LabelVerticalPlacement labelVerticalPlacement() const { return mSettings.labelVerticalPlacement(); }

    /**
     * Sets the vertical \a placement of text labels.
     * \see labelVerticalPlacement()
     * \since QGIS 3.10
     */
    void setLabelVerticalPlacement( QgsScaleBarSettings::LabelVerticalPlacement placement );

    /**
     * Returns the horizontal placement of text labels.
     * \see setLabelHorizontalPlacement()
     * \since QGIS 3.10
     */
    QgsScaleBarSettings::LabelHorizontalPlacement labelHorizontalPlacement() const { return mSettings.labelHorizontalPlacement(); }

    /**
     * Sets the horizontal \a placement of text labels.
     * \see labelHorizontalPlacement()
     * \since QGIS 3.10
     */
    void setLabelHorizontalPlacement( QgsScaleBarSettings::LabelHorizontalPlacement placement );

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
     * \deprecated use lineSymbol() instead
     */
    Q_DECL_DEPRECATED Qt::PenJoinStyle lineJoinStyle() const SIP_DEPRECATED;

    /**
     * Sets the join \a style used when drawing the lines in the scalebar
     * \see lineJoinStyle()
     * \deprecated use setLineSymbol() instead
     */
    Q_DECL_DEPRECATED void setLineJoinStyle( Qt::PenJoinStyle style ) SIP_DEPRECATED;

    /**
     * Returns the cap style used for drawing lines in the scalebar.
     * \see setLineCapStyle()
     * \deprecated use lineSymbol() instead
     */
    Q_DECL_DEPRECATED Qt::PenCapStyle lineCapStyle() const SIP_DEPRECATED;

    /**
     * Sets the cap \a style used when drawing the lines in the scalebar.
     * \see lineCapStyle()
     * \deprecated use setLineSymbol() instead
     */
    Q_DECL_DEPRECATED void setLineCapStyle( Qt::PenCapStyle style ) SIP_DEPRECATED;

    /**
     * Applies the default scalebar settings to the scale bar.
     * \see applyDefaultSize()
     */
    void applyDefaultSettings();

    /**
     * Applies any default settings relating to the specified \a renderer to the item.
     *
     * Returns TRUE if settings were applied.
     *
     * \since QGIS 3.14
     */
    bool applyDefaultRendererSettings( QgsScaleBarRenderer *renderer );

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
     * 'Line Ticks Down', 'Line Ticks Up', 'Stepped Line', 'Hollow', 'Numeric'.
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
     * Returns the numeric format used for numbers in the scalebar.
     *
     * \see setNumericFormat()
     * \since QGIS 3.12
     */
    const QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for numbers in the scalebar.
     *
     * Ownership of \a format is transferred to the scalebar.
     *
     * \see numericFormat()
     * \since QGIS 3.12
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Adjusts the scale bar box size and updates the item.
     */
    void update();

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties ) override;
    void finalizeRestoreFromXml() override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;
    ExportLayerBehavior exportLayerBehavior() const override;

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

    //! Calculates width of a segment in mm and stores it in mSegmentMillimeters
    void refreshSegmentMillimeters();

    //! Returns diagonal of layout map in selected units (map units / meters / feet / nautical miles)
    double mapWidth() const;

    QgsScaleBarRenderer::ScaleBarContext createScaleContext() const;

    friend class QgsCompositionConverter;

};

#endif //QGSLAYOUTITEMSCALEBAR_H


