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

#include "qgssymbollayerv2.h"

/**
 * This class is used for representing lines as arrows.
 */
class CORE_EXPORT QgsArrowSymbolLayer : public QgsLineSymbolLayerV2
{
  public:
    /** Simple constructor */
    QgsArrowSymbolLayer();

    /**
     * Create a new QgsArrowSymbolLayer
     *
     * @param properties A property map to deserialize saved information from properties()
     *
     * @return A new QgsArrowSymbolLayer
     */
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    /** Virtual constructor */
    virtual QgsArrowSymbolLayer* clone() const override;

    /** Get the sub symbol used for filling */
    virtual QgsSymbolV2* subSymbol() override { return mSymbol.data(); }

    /** Set the sub symbol used for filling. Takes ownership. */
    virtual bool setSubSymbol( QgsSymbolV2* symbol ) override;

    /** Return a list of attributes required to render this feature */
    virtual QSet<QString> usedAttributes() const override;

    /** Get current arrow width */
    double arrowWidth() const { return mArrowWidth; }
    /** Set the arrow width */
    void setArrowWidth( double w ) { mArrowWidth = w; }
    /** Get the unit for the arrow width */
    QgsSymbolV2::OutputUnit arrowWidthUnit() const { return mArrowWidthUnit; }
    /** Set the unit for the arrow width */
    void setArrowWidthUnit( QgsSymbolV2::OutputUnit u ) { mArrowWidthUnit = u; }
    /** Get the scale for the arrow width */
    QgsMapUnitScale arrowWidthUnitScale() const { return mArrowWidthUnitScale; }
    /** Set the scale for the arrow width */
    void setArrowWidthUnitScale( const QgsMapUnitScale& s ) { mArrowWidthUnitScale = s; }

    /** Get current arrow start width. Only meaningfull for single headed arrows */
    double arrowStartWidth() const { return mArrowStartWidth; }
    /** Set the arrow start width */
    void setArrowStartWidth( double w ) { mArrowStartWidth = w; }
    /** Get the unit for the arrow start width */
    QgsSymbolV2::OutputUnit arrowStartWidthUnit() const { return mArrowStartWidthUnit; }
    /** Set the unit for the arrow start width */
    void setArrowStartWidthUnit( QgsSymbolV2::OutputUnit u ) { mArrowStartWidthUnit = u; }
    /** Get the scale for the arrow start width */
    QgsMapUnitScale arrowStartWidthUnitScale() const { return mArrowStartWidthUnitScale; }
    /** Set the scale for the arrow start width */
    void setArrowStartWidthUnitScale( const QgsMapUnitScale& s ) { mArrowStartWidthUnitScale = s; }

    /** Get the current arrow head width */
    double headWidth() const { return mHeadWidth; }
    /** Set the arrow head width */
    void setHeadWidth( double s ) { mHeadWidth = s; }
    /** Get the unit for the head width */
    QgsSymbolV2::OutputUnit headWidthUnit() const { return mHeadWidthUnit; }
    /** Set the unit for the head width */
    void setHeadWidthUnit( QgsSymbolV2::OutputUnit u ) { mHeadWidthUnit = u; }
    /** Get the scale for the head width */
    QgsMapUnitScale headWidthUnitScale() const { return mHeadWidthUnitScale; }
    /** Set the scale for the head width */
    void setHeadWidthUnitScale( const QgsMapUnitScale& s ) { mHeadWidthUnitScale = s; }

    /** Get the current arrow head height */
    double headHeight() const { return mHeadHeight; }
    /** Set the arrow head height */
    void setHeadHeight( double s ) { mHeadHeight = s; }
    /** Get the unit for the head height */
    QgsSymbolV2::OutputUnit headHeightUnit() const { return mHeadHeightUnit; }
    /** Set the unit for the head height */
    void setHeadHeightUnit( QgsSymbolV2::OutputUnit u ) { mHeadHeightUnit = u; }
    /** Get the scale for the head height */
    QgsMapUnitScale headHeightUnitScale() const { return mHeadHeightUnitScale; }
    /** Set the scale for the head height */
    void setHeadHeightUnitScale( const QgsMapUnitScale& s ) { mHeadHeightUnitScale = s; }

    /** Return whether it is a curved arrow or a straight one */
    bool isCurved() const { return mIsCurved; }
    /** Set whether it is a curved arrow or a straight one */
    void setIsCurved( bool isCurved ) { mIsCurved = isCurved; }

    /** Return whether the arrow is repeated along the line or not */
    bool isRepeated() const { return mIsRepeated; }
    /** Set whether the arrow is repeated along the line */
    void setIsRepeated( bool isRepeated ) { mIsRepeated = isRepeated; }

    /** Possible head types */
    enum HeadType
    {
      HeadSingle,   //< One single head at the end
      HeadReversed, //< One single head at the beginning
      HeadDouble    //< Two heads
    };

    /** Get the current head type */
    HeadType headType() const { return mHeadType; }
    /** Set the head type */
    void setHeadType( HeadType t ) { mHeadType = t; }

    /** Possible arrow types */
    enum ArrowType
    {
      ArrowPlain,     //< Regular arrow
      ArrowLeftHalf,  //< Halved arrow, only the left side of the arrow is rendered (for straight arrows) or the side toward the exterior (for curved arrows)
      ArrowRightHalf  //< Halved arrow, only the right side of the arrow is rendered (for straight arrows) or the side toward the interior (for curved arrows)
    };

    /** Get the current arrow type */
    ArrowType arrowType() const { return mArrowType; }
    /** Set the arrow type */
    void setArrowType( ArrowType t ) { mArrowType = t; }

    /**
     * Should be reimplemented by subclasses to return a string map that
     * contains the configuration information for the symbol layer. This
     * is used to serialize a symbol layer perstistently.
     */
    QgsStringMap properties() const override;

    /**
     * Returns a string that represents this layer type. Used for serialization.
     * Should match with the string used to register this symbol layer in the registry.
     */
    QString layerType() const override;

    /** Prepare the rendering */
    void startRender( QgsSymbolV2RenderContext& context ) override;

    /** End of the rendering */
    void stopRender( QgsSymbolV2RenderContext& context ) override;

    /** Main drawing method */
    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) override;

    void setColor( const QColor& c ) override;
    virtual QColor color() const override;

  private:
    /** Filling sub symbol */
    QScopedPointer<QgsFillSymbolV2> mSymbol;

    double mArrowWidth;
    QgsSymbolV2::OutputUnit mArrowWidthUnit;
    QgsMapUnitScale mArrowWidthUnitScale;

    double mArrowStartWidth;
    QgsSymbolV2::OutputUnit mArrowStartWidthUnit;
    QgsMapUnitScale mArrowStartWidthUnitScale;

    double mHeadWidth;
    QgsSymbolV2::OutputUnit mHeadWidthUnit;
    QgsMapUnitScale mHeadWidthUnitScale;
    double mHeadHeight;
    QgsSymbolV2::OutputUnit mHeadHeightUnit;
    QgsMapUnitScale mHeadHeightUnitScale;

    HeadType mHeadType;
    ArrowType mArrowType;
    bool mIsCurved;
    bool mIsRepeated;

    double mScaledArrowWidth;
    double mScaledArrowStartWidth;
    double mScaledHeadWidth;
    double mScaledHeadHeight;
    double mScaledOffset;
    HeadType mComputedHeadType;
    ArrowType mComputedArrowType;

    QScopedPointer<QgsExpressionContextScope> mExpressionScope;

    void _resolveDataDefined( QgsSymbolV2RenderContext& );
};

#endif


