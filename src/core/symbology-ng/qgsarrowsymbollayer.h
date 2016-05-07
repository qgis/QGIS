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

    /** Get the current arrow head size */
    double headSize() const { return mHeadSize; }
    /** Set the arrow head size */
    void setHeadSize( double s ) { mHeadSize = s; }
    /** Get the unit for the head size */
    QgsSymbolV2::OutputUnit headSizeUnit() const { return mHeadSizeUnit; }
    /** Set the unit for the head size */
    void setHeadSizeUnit( QgsSymbolV2::OutputUnit u ) { mHeadSizeUnit = u; }
    /** Get the scale for the head size */
    QgsMapUnitScale headSizeUnitScale() const { return mHeadSizeUnitScale; }
    /** Set the scale for the head size */
    void setHeadSizeUnitScale( const QgsMapUnitScale& s ) { mHeadSizeUnitScale = s; }

    /** Return whether it is a curved arrow or a straight one */
    bool isCurved() const { return mIsCurved; }
    /** Set whether it is a curved arrow or a straight one */
    void setIsCurved( bool isCurved ) { mIsCurved = isCurved; }

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

    double mHeadSize;
    QgsSymbolV2::OutputUnit mHeadSizeUnit;
    QgsMapUnitScale mHeadSizeUnitScale;
    HeadType mHeadType;
    bool mIsCurved;

    double mScaledArrowWidth;
    double mScaledArrowStartWidth;
    double mScaledHeadSize;
    double mScaledOffset;
    HeadType mComputedHeadType;

    QScopedPointer<QgsExpressionContextScope> mExpressionScope;

    void _resolveDataDefined( QgsSymbolV2RenderContext& );
};

#endif


