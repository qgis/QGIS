/***************************************************************************
                            qgscomposerscalebar.h
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERSCALEBAR_H
#define QGSCOMPOSERSCALEBAR_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscomposeritem.h"
#include "scalebar/qgsscalebarsettings.h"
#include "scalebar/qgsscalebarrenderer.h"
#include <QFont>
#include <QPen>
#include <QColor>

class QgsComposerMap;

/**
 * \ingroup core
 * A scale bar item that can be added to a map composition.
 */

class CORE_EXPORT QgsComposerScaleBar: public QgsComposerItem
{
    Q_OBJECT

  public:

    QgsComposerScaleBar( QgsComposition *composition SIP_TRANSFERTHIS );
    ~QgsComposerScaleBar();

    //! Return correct graphics item type.
    virtual int type() const override { return ComposerScaleBar; }

    //! \brief Reimplementation of QCanvasItem::paint
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    //getters and setters
    int numSegments() const {return mSettings.numberOfSegments();}
    void setNumSegments( int nSegments );

    int numSegmentsLeft() const {return mSettings.numberOfSegmentsLeft();}
    void setNumSegmentsLeft( int nSegmentsLeft );

    double numUnitsPerSegment() const {return mSettings.unitsPerSegment();}
    void setNumUnitsPerSegment( double units );

    /**
     * Returns the size mode for scale bar segments.
     * \see setSegmentSizeMode
     * \see minBarWidth
     * \see maxBarWidth
     * \since QGIS 2.9
     */
    QgsScaleBarSettings::SegmentSizeMode segmentSizeMode() const { return mSettings.segmentSizeMode(); }

    /**
     * Sets the size mode for scale bar segments.
     * \param mode size mode
     * \see segmentSizeMode
     * \see setMinBarWidth
     * \see setMaxBarWidth
     * \since QGIS 2.9
     */
    void setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode );

    /**
     * Returns the minimum size (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode
     * \see setMinBarWidth
     * \see maxBarWidth
     * \since QGIS 2.9
     */
    double minBarWidth() const { return mSettings.minimumBarWidth(); }

    /**
     * Sets the minimum size (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \param minWidth minimum width in millimeters
     * \see minBarWidth
     * \see setMaxBarWidth
     * \see setSegmentSizeMode
     * \since QGIS 2.9
     */
    void setMinBarWidth( double minWidth );

    /**
     * Returns the maximum size (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \see segmentSizeMode
     * \see setMaxBarWidth
     * \see minBarWidth
     * \since QGIS 2.9
     */
    double maxBarWidth() const { return mSettings.maximumBarWidth(); }

    /**
     * Sets the maximum size (in millimeters) for scale bar segments. This
     * property is only effective if the segmentSizeMode() is set
     * to SegmentSizeFitWidth.
     * \param maxWidth maximum width in millimeters
     * \see minBarWidth
     * \see setMaxBarWidth
     * \see setSegmentSizeMode
     * \since QGIS 2.9
     */
    void setMaxBarWidth( double maxWidth );

    double numMapUnitsPerScaleBarUnit() const {return mSettings.mapUnitsPerScaleBarUnit();}
    void setNumMapUnitsPerScaleBarUnit( double d ) { mSettings.setMapUnitsPerScaleBarUnit( d );}

    QString unitLabeling() const {return mSettings.unitLabel();}
    void setUnitLabeling( const QString &label ) { mSettings.setUnitLabel( label );}

    QFont font() const;
    void setFont( const QFont &font );

    /**
     * Returns the color used for drawing text in the scalebar.
     * \returns font color for scalebar.
     * \see setFontColor
     * \see font
     */
    QColor fontColor() const {return mSettings.fontColor();}

    /**
     * Sets the color used for drawing text in the scalebar.
     * \param c font color for scalebar.
     * \see fontColor
     * \see setFont
     */
    void setFontColor( const QColor &c ) {mSettings.setFontColor( c );}

    /**
     * Returns the color used for fills in the scalebar.
     * \see setFillColor()
     * \see fillColor2()
     * \since QGIS 3.0
     */
    QColor fillColor() const {return mSettings.fillColor();}

    /**
     * Sets the color used for fills in the scalebar.
     * \see fillColor()
     * \see setFillColor2()
     * \since QGIS 3.0
     */
    void setFillColor( const QColor &color ) {mSettings.setFillColor( color ); }

    /**
     * Returns the secondary color used for fills in the scalebar.
     * \see setFillColor2()
     * \see fillColor()
     * \since QGIS 3.0
     */
    QColor fillColor2() const {return mSettings.fillColor2();}

    /**
     * Sets the secondary color used for fills in the scalebar.
     * \see fillColor2()
     * \see setFillColor2()
     * \since QGIS 3.0
     */
    void setFillColor2( const QColor &color ) {mSettings.setFillColor2( color ); }

    /**
     * Returns the color used for lines in the scalebar.
     * \see setLineColor()
     * \since QGIS 3.0
     */
    QColor lineColor() const {return mSettings.lineColor();}

    /**
     * Sets the color used for lines in the scalebar.
     * \see lineColor()
     * \since QGIS 3.0
     */
    void setLineColor( const QColor &color ) { mSettings.setLineColor( color ); }

    /**
     * Returns the line width in millimeters for lines in the scalebar.
     * \see setLineWidth()
     * \since QGIS 3.0
     */
    double lineWidth() const {return mSettings.lineWidth();}

    /**
     * Sets the line width in millimeters for lines in the scalebar.
     * \see lineWidth()
     * \since QGIS 3.0
     */
    void setLineWidth( double width ) { mSettings.setLineWidth( width ); }

    /**
     * Returns the pen used for drawing the scalebar.
     * \returns QPen used for drawing the scalebar outlines.
     * \see setPen
     * \see brush
     */
    QPen pen() const {return mSettings.pen();}

    /**
     * Returns the primary brush for the scalebar.
     * \returns QBrush used for filling the scalebar
     * \see setBrush
     * \see brush2
     * \see pen
     */
    QBrush brush() const {return mSettings.brush();}

    /**
     * Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * \returns QBrush used for secondary color areas
     * \see setBrush2
     * \see brush
     */
    QBrush brush2() const {return mSettings.brush2(); }

    double height() const { return mSettings.height(); }
    void setHeight( double h ) { mSettings.setHeight( h );}

    /**
     * Sets the \a map item linked to the scalebar.
     */
    void setComposerMap( QgsComposerMap *map );

    /**
     * Returns the map item linked to the scalebar.
     */
    QgsComposerMap *composerMap() const {return mComposerMap;}

    double labelBarSpace() const {return mSettings.labelBarSpace();}
    void setLabelBarSpace( double space ) {mSettings.setLabelBarSpace( space );}

    double boxContentSpace() const {return mSettings.boxContentSpace();}
    void setBoxContentSpace( double space );

    /**
     * Returns the alignment of the scalebar.
     */
    QgsScaleBarSettings::Alignment alignment() const { return mSettings.alignment(); }

    /**
     * Sets the \a alignment of the scalebar.
     */
    void setAlignment( QgsScaleBarSettings::Alignment alignment );

    /**
     * Returns the scalebar distance units.
     */
    QgsUnitTypes::DistanceUnit units() const { return mSettings.units(); }

    /**
     * Sets the scalebar distance units.
     */
    void setUnits( QgsUnitTypes::DistanceUnit u );

    /**
     * Returns the join style used for drawing lines in the scalebar
     * \returns Join style for lines
     * \since QGIS 2.3
     * \see setLineJoinStyle
     */
    Qt::PenJoinStyle lineJoinStyle() const { return mSettings.lineJoinStyle(); }

    /**
     * Sets join style used when drawing the lines in the scalebar
     * \param style Join style for lines
     * \returns nothing
     * \since QGIS 2.3
     * \see lineJoinStyle
     */
    void setLineJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the cap style used for drawing lines in the scalebar
     * \returns Cap style for lines
     * \since QGIS 2.3
     * \see setLineCapStyle
     */
    Qt::PenCapStyle lineCapStyle() const { return mSettings.lineCapStyle(); }

    /**
     * Sets cap style used when drawing the lines in the scalebar
     * \param style Cap style for lines
     * \returns nothing
     * \since QGIS 2.3
     * \see lineCapStyle
     */
    void setLineCapStyle( Qt::PenCapStyle style );

    //! Apply default settings
    void applyDefaultSettings();
    //! Apply default size (scale bar 1/5 of map item width)
    void applyDefaultSize( QgsUnitTypes::DistanceUnit u = QgsUnitTypes::DistanceMeters );

    /**
     * Sets style by name
     \param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*/
    void setStyle( const QString &styleName );

    //! Returns style name
    QString style() const;

    //! Sets box size suitable to content
    void adjustBoxSize();

    //! Adjusts box size and calls QgsComposerItem::update()
    void update();


    /**
     * Stores state in Dom element
     * \param elem is Dom element corresponding to 'Composer' tag
     * \param doc Dom document
     */
    bool writeXml( QDomElement &elem, QDomDocument &doc ) const override;

    /**
     * Sets state from Dom document
     * \param itemElem is Dom node corresponding to item tag
     * \param doc is Dom document
     */
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc ) override;

    //! Moves scalebar position to the left / right depending on alignment and change in item width
    void correctXPositionAlignment( double width, double widthAfter );

    //overridden to apply minimum size
    void setSceneRect( const QRectF &rectangle ) override;

  public slots:
    void updateSegmentSize();
    //! Sets mCompositionMap to 0 if the map is deleted
    void invalidateCurrentMap();
    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = nullptr ) override;

  private:

    //! Reference to composer map object
    QgsComposerMap *mComposerMap = nullptr;

    QgsScaleBarSettings mSettings;

    //! Scalebar style
    QgsScaleBarRenderer *mStyle = nullptr;

    //! Width of a segment (in mm)
    double mSegmentMillimeters;

    //! Calculates with of a segment in mm and stores it in mSegmentMillimeters
    void refreshSegmentMillimeters();

    //! Returns diagonal of composer map in selected units (map units / meters / feet / nautical miles)
    double mapWidth() const;

    QgsScaleBarRenderer::ScaleBarContext createScaleContext() const;

};

#endif //QGSCOMPOSERSCALEBAR_H


