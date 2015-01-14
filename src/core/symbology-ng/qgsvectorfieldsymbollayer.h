/***************************************************************************
         qgsvectorfieldsymbollayer.h
         -------------------------
  begin                : Octorer 25, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDSYMBOLLAYER_H
#define QGSVECTORFIELDSYMBOLLAYER_H

#include "qgssymbollayerv2.h"

/**A symbol layer class for displaying displacement arrows based on point layer attributes*/
class CORE_EXPORT QgsVectorFieldSymbolLayer: public QgsMarkerSymbolLayerV2
{
  public:
    enum VectorFieldType
    {
      Cartesian = 0,
      Polar,
      Height
    };

    enum AngleOrientation
    {
      ClockwiseFromNorth = 0,
      CounterclockwiseFromEast
    };

    enum AngleUnits
    {
      Degrees = 0,
      Radians
    };

    QgsVectorFieldSymbolLayer();
    ~QgsVectorFieldSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const OVERRIDE { return "VectorField"; }

    bool setSubSymbol( QgsSymbolV2* symbol ) OVERRIDE;
    QgsSymbolV2* subSymbol() OVERRIDE { return mLineSymbol; }

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) OVERRIDE;
    void startRender( QgsSymbolV2RenderContext& context ) OVERRIDE;
    void stopRender( QgsSymbolV2RenderContext& context ) OVERRIDE;

    QgsSymbolLayerV2* clone() const OVERRIDE;
    QgsStringMap properties() const OVERRIDE;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const OVERRIDE;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) OVERRIDE;

    QSet<QString> usedAttributes() const OVERRIDE;

    //setters and getters
    void setXAttribute( const QString& attribute ) { mXAttribute = attribute; }
    QString xAttribute() const { return mXAttribute; }
    void setYAttribute( const QString& attribute ) { mYAttribute = attribute; }
    QString yAttribute() const { return mYAttribute; }
    void setScale( double s ) { mScale = s; }
    double scale() const { return mScale; }
    void setVectorFieldType( VectorFieldType type ) { mVectorFieldType = type; }
    VectorFieldType vectorFieldType() const { return mVectorFieldType; }
    void setAngleOrientation( AngleOrientation orientation ) { mAngleOrientation = orientation; }
    AngleOrientation angleOrientation() const { return mAngleOrientation; }
    void setAngleUnits( AngleUnits units ) { mAngleUnits = units; }
    AngleUnits angleUnits() const { return mAngleUnits; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) OVERRIDE;
    QgsSymbolV2::OutputUnit outputUnit() const OVERRIDE;

    void setMapUnitScale( const QgsMapUnitScale& scale ) OVERRIDE;
    QgsMapUnitScale mapUnitScale() const OVERRIDE;

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceUnit = unit; }
    QgsSymbolV2::OutputUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale& distanceMapUnitScale() const { return mDistanceMapUnitScale; }

  private:
    QString mXAttribute;
    QString mYAttribute;
    QgsSymbolV2::OutputUnit mDistanceUnit;
    QgsMapUnitScale mDistanceMapUnitScale;
    double mScale;
    VectorFieldType mVectorFieldType;
    AngleOrientation mAngleOrientation;
    AngleUnits mAngleUnits;

    QgsLineSymbolV2* mLineSymbol;

    //Attribute indices are resolved in startRender method
    int mXIndex;
    int mYIndex;

    //Converts length/angle to cartesian x/y
    void convertPolarToCartesian( double length, double angle, double& x, double& y ) const;
};

#endif // QGSVECTORFIELDSYMBOLLAYER_H


