/***************************************************************************
 qgsarrowsymbollayer.h
 ---------------------
 begin                : January 2016
 copyright            : (C) 2016 by Hugo Mercier
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARROWSYMBOLLAYER_H
#define QGSARROWSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

class QgsFillSymbol;


/**
 * \ingroup core
 * \class QgsArrowSymbolLayer
 * \brief Line symbol layer used for representing lines as arrows.
 * \since QGIS 2.16
 */

class CORE_EXPORT QgsArrowSymbolLayer : public QgsLineSymbolLayer
{
  public:
    //! Simple constructor
    QgsArrowSymbolLayer();
    ~QgsArrowSymbolLayer() override;

    /**
     * Create a new QgsArrowSymbolLayer
     *
     * \param properties A property map to deserialize saved information from properties()
     *
     * \returns A new QgsArrowSymbolLayer
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QgsArrowSymbolLayer *clone() const override SIP_FACTORY;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    bool usesMapUnits() const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;

    //! Gets current arrow width
    double arrowWidth() const { return mArrowWidth; }
    //! Sets the arrow width
    void setArrowWidth( double width ) { mArrowWidth = width; }
    //! Gets the unit for the arrow width
    QgsUnitTypes::RenderUnit arrowWidthUnit() const { return mArrowWidthUnit; }
    //! Sets the unit for the arrow width
    void setArrowWidthUnit( QgsUnitTypes::RenderUnit unit ) { mArrowWidthUnit = unit; }
    //! Gets the scale for the arrow width
    QgsMapUnitScale arrowWidthUnitScale() const { return mArrowWidthUnitScale; }
    //! Sets the scale for the arrow width
    void setArrowWidthUnitScale( const QgsMapUnitScale &scale ) { mArrowWidthUnitScale = scale; }

    //! Gets current arrow start width. Only meaningful for single headed arrows
    double arrowStartWidth() const { return mArrowStartWidth; }
    //! Sets the arrow start width
    void setArrowStartWidth( double width ) { mArrowStartWidth = width; }
    //! Gets the unit for the arrow start width
    QgsUnitTypes::RenderUnit arrowStartWidthUnit() const { return mArrowStartWidthUnit; }
    //! Sets the unit for the arrow start width
    void setArrowStartWidthUnit( QgsUnitTypes::RenderUnit unit ) { mArrowStartWidthUnit = unit; }
    //! Gets the scale for the arrow start width
    QgsMapUnitScale arrowStartWidthUnitScale() const { return mArrowStartWidthUnitScale; }
    //! Sets the scale for the arrow start width
    void setArrowStartWidthUnitScale( const QgsMapUnitScale &scale ) { mArrowStartWidthUnitScale = scale; }

    //! Gets the current arrow head length
    double headLength() const { return mHeadLength; }
    //! Sets the arrow head length
    void setHeadLength( double length ) { mHeadLength = length; }
    //! Gets the unit for the head length
    QgsUnitTypes::RenderUnit headLengthUnit() const { return mHeadLengthUnit; }
    //! Sets the unit for the head length
    void setHeadLengthUnit( QgsUnitTypes::RenderUnit unit ) { mHeadLengthUnit = unit; }
    //! Gets the scale for the head length
    QgsMapUnitScale headLengthUnitScale() const { return mHeadLengthUnitScale; }
    //! Sets the scale for the head length
    void setHeadLengthUnitScale( const QgsMapUnitScale &scale ) { mHeadLengthUnitScale = scale; }

    //! Gets the current arrow head height
    double headThickness() const { return mHeadThickness; }
    //! Sets the arrow head height
    void setHeadThickness( double thickness ) { mHeadThickness = thickness; }
    //! Gets the unit for the head height
    QgsUnitTypes::RenderUnit headThicknessUnit() const { return mHeadThicknessUnit; }
    //! Sets the unit for the head height
    void setHeadThicknessUnit( QgsUnitTypes::RenderUnit unit ) { mHeadThicknessUnit = unit; }
    //! Gets the scale for the head height
    QgsMapUnitScale headThicknessUnitScale() const { return mHeadThicknessUnitScale; }
    //! Sets the scale for the head height
    void setHeadThicknessUnitScale( const QgsMapUnitScale &scale ) { mHeadThicknessUnitScale = scale; }

    //! Returns whether it is a curved arrow or a straight one
    bool isCurved() const { return mIsCurved; }
    //! Sets whether it is a curved arrow or a straight one
    void setIsCurved( bool isCurved ) { mIsCurved = isCurved; }

    //! Returns whether the arrow is repeated along the line or not
    bool isRepeated() const { return mIsRepeated; }
    //! Sets whether the arrow is repeated along the line
    void setIsRepeated( bool isRepeated ) { mIsRepeated = isRepeated; }

    //! Possible head types
    enum HeadType
    {
      HeadSingle,   //< One single head at the end
      HeadReversed, //< One single head at the beginning
      HeadDouble    //< Two heads
    };

    //! Gets the current head type
    HeadType headType() const { return mHeadType; }
    //! Sets the head type
    void setHeadType( HeadType type ) { mHeadType = type; }

    //! Possible arrow types
    enum ArrowType
    {
      ArrowPlain,     //< Regular arrow
      ArrowLeftHalf,  //< Halved arrow, only the left side of the arrow is rendered (for straight arrows) or the side toward the exterior (for curved arrows)
      ArrowRightHalf  //< Halved arrow, only the right side of the arrow is rendered (for straight arrows) or the side toward the interior (for curved arrows)
    };

    //! Gets the current arrow type
    ArrowType arrowType() const { return mArrowType; }
    //! Sets the arrow type
    void setArrowType( ArrowType type ) { mArrowType = type; }

    QVariantMap properties() const override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    void setColor( const QColor &c ) override;
    QColor color() const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

  private:
#ifdef SIP_RUN
    QgsArrowSymbolLayer( const QgsArrowSymbolLayer & );
#endif

    //! Filling sub symbol
    std::unique_ptr<QgsFillSymbol> mSymbol;

    double mArrowWidth = 1.0;
    QgsUnitTypes::RenderUnit mArrowWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mArrowWidthUnitScale;

    double mArrowStartWidth = 1.0;
    QgsUnitTypes::RenderUnit mArrowStartWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mArrowStartWidthUnitScale;

    double mHeadLength = 1.5;
    QgsUnitTypes::RenderUnit mHeadLengthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mHeadLengthUnitScale;
    double mHeadThickness = 1.5;
    QgsUnitTypes::RenderUnit mHeadThicknessUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mHeadThicknessUnitScale;

    HeadType mHeadType = HeadSingle;
    ArrowType mArrowType = ArrowPlain;
    bool mIsCurved = true;
    bool mIsRepeated = true;

    double mScaledArrowWidth = 1.0;
    double mScaledArrowStartWidth = 1.0;
    double mScaledHeadLength = 1.5;
    double mScaledHeadThickness = 1.5;
    double mScaledOffset = 0.0;
    HeadType mComputedHeadType = HeadSingle;
    ArrowType mComputedArrowType = ArrowPlain;

    std::unique_ptr<QgsExpressionContextScope> mExpressionScope;

    void _resolveDataDefined( QgsSymbolRenderContext & );
};

#endif


