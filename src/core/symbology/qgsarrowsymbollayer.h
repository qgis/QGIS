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

#ifndef QGSARROWSYMBOLLAYERV2_H
#define QGSARROWSYMBOLLAYERV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"


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

    /**
     * Create a new QgsArrowSymbolLayer
     *
     * \param properties A property map to deserialize saved information from properties()
     *
     * \returns A new QgsArrowSymbolLayer
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    virtual QgsArrowSymbolLayer *clone() const override SIP_FACTORY;
    virtual QgsSymbol *subSymbol() override { return mSymbol.get(); }
    virtual bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

    //! Get current arrow width
    double arrowWidth() const { return mArrowWidth; }
    //! Set the arrow width
    void setArrowWidth( double width ) { mArrowWidth = width; }
    //! Get the unit for the arrow width
    QgsUnitTypes::RenderUnit arrowWidthUnit() const { return mArrowWidthUnit; }
    //! Set the unit for the arrow width
    void setArrowWidthUnit( QgsUnitTypes::RenderUnit unit ) { mArrowWidthUnit = unit; }
    //! Get the scale for the arrow width
    QgsMapUnitScale arrowWidthUnitScale() const { return mArrowWidthUnitScale; }
    //! Set the scale for the arrow width
    void setArrowWidthUnitScale( const QgsMapUnitScale &scale ) { mArrowWidthUnitScale = scale; }

    //! Get current arrow start width. Only meaningfull for single headed arrows
    double arrowStartWidth() const { return mArrowStartWidth; }
    //! Set the arrow start width
    void setArrowStartWidth( double width ) { mArrowStartWidth = width; }
    //! Get the unit for the arrow start width
    QgsUnitTypes::RenderUnit arrowStartWidthUnit() const { return mArrowStartWidthUnit; }
    //! Set the unit for the arrow start width
    void setArrowStartWidthUnit( QgsUnitTypes::RenderUnit unit ) { mArrowStartWidthUnit = unit; }
    //! Get the scale for the arrow start width
    QgsMapUnitScale arrowStartWidthUnitScale() const { return mArrowStartWidthUnitScale; }
    //! Set the scale for the arrow start width
    void setArrowStartWidthUnitScale( const QgsMapUnitScale &scale ) { mArrowStartWidthUnitScale = scale; }

    //! Get the current arrow head length
    double headLength() const { return mHeadLength; }
    //! Set the arrow head length
    void setHeadLength( double length ) { mHeadLength = length; }
    //! Get the unit for the head length
    QgsUnitTypes::RenderUnit headLengthUnit() const { return mHeadLengthUnit; }
    //! Set the unit for the head length
    void setHeadLengthUnit( QgsUnitTypes::RenderUnit unit ) { mHeadLengthUnit = unit; }
    //! Get the scale for the head length
    QgsMapUnitScale headLengthUnitScale() const { return mHeadLengthUnitScale; }
    //! Set the scale for the head length
    void setHeadLengthUnitScale( const QgsMapUnitScale &scale ) { mHeadLengthUnitScale = scale; }

    //! Get the current arrow head height
    double headThickness() const { return mHeadThickness; }
    //! Set the arrow head height
    void setHeadThickness( double thickness ) { mHeadThickness = thickness; }
    //! Get the unit for the head height
    QgsUnitTypes::RenderUnit headThicknessUnit() const { return mHeadThicknessUnit; }
    //! Set the unit for the head height
    void setHeadThicknessUnit( QgsUnitTypes::RenderUnit unit ) { mHeadThicknessUnit = unit; }
    //! Get the scale for the head height
    QgsMapUnitScale headThicknessUnitScale() const { return mHeadThicknessUnitScale; }
    //! Set the scale for the head height
    void setHeadThicknessUnitScale( const QgsMapUnitScale &scale ) { mHeadThicknessUnitScale = scale; }

    //! Return whether it is a curved arrow or a straight one
    bool isCurved() const { return mIsCurved; }
    //! Set whether it is a curved arrow or a straight one
    void setIsCurved( bool isCurved ) { mIsCurved = isCurved; }

    //! Return whether the arrow is repeated along the line or not
    bool isRepeated() const { return mIsRepeated; }
    //! Set whether the arrow is repeated along the line
    void setIsRepeated( bool isRepeated ) { mIsRepeated = isRepeated; }

    //! Possible head types
    enum HeadType
    {
      HeadSingle,   //< One single head at the end
      HeadReversed, //< One single head at the beginning
      HeadDouble    //< Two heads
    };

    //! Get the current head type
    HeadType headType() const { return mHeadType; }
    //! Set the head type
    void setHeadType( HeadType type ) { mHeadType = type; }

    //! Possible arrow types
    enum ArrowType
    {
      ArrowPlain,     //< Regular arrow
      ArrowLeftHalf,  //< Halved arrow, only the left side of the arrow is rendered (for straight arrows) or the side toward the exterior (for curved arrows)
      ArrowRightHalf  //< Halved arrow, only the right side of the arrow is rendered (for straight arrows) or the side toward the interior (for curved arrows)
    };

    //! Get the current arrow type
    ArrowType arrowType() const { return mArrowType; }
    //! Set the arrow type
    void setArrowType( ArrowType type ) { mArrowType = type; }

    QgsStringMap properties() const override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    void setColor( const QColor &c ) override;
    virtual QColor color() const override;

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


