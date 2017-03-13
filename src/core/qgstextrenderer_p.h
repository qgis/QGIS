/***************************************************************************
  qgstextrenderer.h
  -----------------
   begin                : September 2015
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTRENDERER_PRIVATE_H
#define QGSTEXTRENDERER_PRIVATE_H

#include "qgis_core.h"
#include "qgstextrenderer.h"
#include "qgsmapunitscale.h"
#include "qgsunittypes.h"
#include "qgsapplication.h"
#include <QSharedData>
#include <QPainter>

/// @cond

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


class CORE_EXPORT QgsTextBufferSettingsPrivate : public QSharedData
{
  public:

    QgsTextBufferSettingsPrivate()
      : enabled( false )
      , size( 1 )
      , sizeUnit( QgsUnitTypes::RenderMillimeters )
      , color( Qt::white )
      , opacity( 1.0 )
      , fillBufferInterior( false )
      , joinStyle( Qt::RoundJoin )
      , blendMode( QPainter::CompositionMode_SourceOver )
    {
    }

    QgsTextBufferSettingsPrivate( const QgsTextBufferSettingsPrivate &other )
      : QSharedData( other )
      , enabled( other.enabled )
      , size( other.size )
      , sizeUnit( other.sizeUnit )
      , sizeMapUnitScale( other.sizeMapUnitScale )
      , color( other.color )
      , opacity( other.opacity )
      , fillBufferInterior( other.fillBufferInterior )
      , joinStyle( other.joinStyle )
      , blendMode( other.blendMode )
    {
    }

    bool enabled;
    double size;
    QgsUnitTypes::RenderUnit sizeUnit;
    QgsMapUnitScale sizeMapUnitScale;
    QColor color;
    double opacity;
    bool fillBufferInterior;
    Qt::PenJoinStyle joinStyle;
    QPainter::CompositionMode blendMode;
};


class CORE_EXPORT QgsTextBackgroundSettingsPrivate : public QSharedData
{
  public:

    QgsTextBackgroundSettingsPrivate()
      : enabled( false )
      , type( QgsTextBackgroundSettings::ShapeRectangle )
      , sizeType( QgsTextBackgroundSettings::SizeBuffer )
      , size( QSizeF( 0.0, 0.0 ) )
      , sizeUnits( QgsUnitTypes::RenderMillimeters )
      , rotationType( QgsTextBackgroundSettings::RotationSync )
      , rotation( 0.0 )
      , offset( QPointF( 0.0, 0.0 ) )
      , offsetUnits( QgsUnitTypes::RenderMillimeters )
      , radii( QSizeF( 0.0, 0.0 ) )
      , radiiUnits( QgsUnitTypes::RenderMillimeters )
      , blendMode( QPainter::CompositionMode_SourceOver )
      , fillColor( Qt::white )
      , strokeColor( Qt::darkGray )
      , opacity( 1.0 )
      , strokeWidth( 0.0 )
      , strokeWidthUnits( QgsUnitTypes::RenderMillimeters )
      , joinStyle( Qt::BevelJoin )
    {
    }

    QgsTextBackgroundSettingsPrivate( const QgsTextBackgroundSettingsPrivate &other )
      : QSharedData( other )
      , enabled( other.enabled )
      , type( other.type )
      , svgFile( other.svgFile )
      , sizeType( other.sizeType )
      , size( other.size )
      , sizeUnits( other.sizeUnits )
      , sizeMapUnitScale( other.sizeMapUnitScale )
      , rotationType( other.rotationType )
      , rotation( other.rotation )
      , offset( other.offset )
      , offsetUnits( other.offsetUnits )
      , offsetMapUnitScale( other.offsetMapUnitScale )
      , radii( other.radii )
      , radiiUnits( other.radiiUnits )
      , radiiMapUnitScale( other.radiiMapUnitScale )
      , blendMode( other.blendMode )
      , fillColor( other.fillColor )
      , strokeColor( other.strokeColor )
      , opacity( other.opacity )
      , strokeWidth( other.strokeWidth )
      , strokeWidthUnits( other.strokeWidthUnits )
      , strokeWidthMapUnitScale( other.strokeWidthMapUnitScale )
      , joinStyle( other.joinStyle )
    {
    }

    bool enabled;
    QgsTextBackgroundSettings::ShapeType type;
    QString svgFile;
    QgsTextBackgroundSettings::SizeType sizeType;
    QSizeF size;
    QgsUnitTypes::RenderUnit sizeUnits;
    QgsMapUnitScale sizeMapUnitScale;
    QgsTextBackgroundSettings::RotationType rotationType;
    double rotation;
    QPointF offset;
    QgsUnitTypes::RenderUnit offsetUnits;
    QgsMapUnitScale offsetMapUnitScale;
    QSizeF radii;
    QgsUnitTypes::RenderUnit radiiUnits;
    QgsMapUnitScale radiiMapUnitScale;
    QPainter::CompositionMode blendMode;
    QColor fillColor;
    QColor strokeColor;
    double opacity;
    double strokeWidth;
    QgsUnitTypes::RenderUnit strokeWidthUnits;
    QgsMapUnitScale strokeWidthMapUnitScale;
    Qt::PenJoinStyle joinStyle;
};



class CORE_EXPORT QgsTextShadowSettingsPrivate : public QSharedData
{
  public:

    QgsTextShadowSettingsPrivate()
      : enabled( false )
      , shadowUnder( QgsTextShadowSettings::ShadowLowest )
      , offsetAngle( 135 )
      , offsetDist( 1.0 )
      , offsetUnits( QgsUnitTypes::RenderMillimeters )
      , offsetGlobal( true )
      , radius( 1.5 )
      , radiusUnits( QgsUnitTypes::RenderMillimeters )
      , radiusAlphaOnly( false )
      , scale( 100 )
      , color( QColor( 0, 0, 0 ) )
      , opacity( 0.7 )
      , blendMode( QPainter::CompositionMode_Multiply )
    {

    }

    QgsTextShadowSettingsPrivate( const QgsTextShadowSettingsPrivate &other )
      : QSharedData( other )
      , enabled( other.enabled )
      , shadowUnder( other.shadowUnder )
      , offsetAngle( other.offsetAngle )
      , offsetDist( other.offsetDist )
      , offsetUnits( other.offsetUnits )
      , offsetMapUnitScale( other.offsetMapUnitScale )
      , offsetGlobal( other.offsetGlobal )
      , radius( other.radius )
      , radiusUnits( other.radiusUnits )
      , radiusMapUnitScale( other.radiusMapUnitScale )
      , radiusAlphaOnly( other.radiusAlphaOnly )
      , scale( other.scale )
      , color( other.color )
      , opacity( other.opacity )
      , blendMode( other.blendMode )
    {
    }

    bool enabled;
    QgsTextShadowSettings::ShadowPlacement shadowUnder;
    int offsetAngle;
    double offsetDist;
    QgsUnitTypes::RenderUnit offsetUnits;
    QgsMapUnitScale offsetMapUnitScale;
    bool offsetGlobal;
    double radius;
    QgsUnitTypes::RenderUnit radiusUnits;
    QgsMapUnitScale radiusMapUnitScale;
    bool radiusAlphaOnly;
    int scale;
    QColor color;
    double opacity;
    QPainter::CompositionMode blendMode;
};


class CORE_EXPORT QgsTextSettingsPrivate : public QSharedData
{
  public:

    QgsTextSettingsPrivate()
      : textFont( QApplication::font() )
      , fontSizeUnits( QgsUnitTypes::RenderPoints )
      , fontSize( 10 )
      , textColor( Qt::black )
      , opacity( 1.0 )
      , blendMode( QPainter::CompositionMode_SourceOver )
      , multilineHeight( 1.0 )
    {
    }

    QgsTextSettingsPrivate( const QgsTextSettingsPrivate &other )
      : QSharedData( other )
      , textFont( other.textFont )
      , textNamedStyle( other.textNamedStyle )
      , fontSizeUnits( other.fontSizeUnits )
      , fontSizeMapUnitScale( other.fontSizeMapUnitScale )
      , fontSize( other.fontSize )
      , textColor( other.textColor )
      , opacity( other.opacity )
      , blendMode( other.blendMode )
      , multilineHeight( other.multilineHeight )
    {
    }

    QFont textFont;
    QString textNamedStyle;
    QgsUnitTypes::RenderUnit fontSizeUnits;
    QgsMapUnitScale fontSizeMapUnitScale;
    double fontSize; //may differ from size in textFont due to units (e.g., size in map units)
    QColor textColor;
    double opacity;
    QPainter::CompositionMode blendMode;
    double multilineHeight; //0.0 to 10.0, leading between lines as multiplyer of line height

};





/// @endcond

#endif // QGSTEXTRENDERER_PRIVATE_H
