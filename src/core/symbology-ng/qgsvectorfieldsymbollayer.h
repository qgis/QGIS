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

    QString layerType() const { return "VectorField"; }

    bool setSubSymbol( QgsSymbolV2* symbol );
    QgsSymbolV2* subSymbol() { return mLineSymbol; }

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsSymbolLayerV2* clone() const;
    QgsStringMap properties() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    QSet<QString> usedAttributes() const;

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

  private:
    QString mXAttribute;
    QString mYAttribute;
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
