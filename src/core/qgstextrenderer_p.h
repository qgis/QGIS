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

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgstextrenderer.h"
#include "qgsmapunitscale.h"
#include "qgsunittypes.h"
#include "qgsapplication.h"
#include "qgspainteffect.h"
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


class QgsTextBufferSettingsPrivate : public QSharedData
{
  public:

    QgsTextBufferSettingsPrivate()
      : color( Qt::white )
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
      , paintEffect( other.paintEffect ? other.paintEffect->clone() : nullptr )
    {
    }

    ~QgsTextBufferSettingsPrivate()
    {
      delete paintEffect;
    }

    bool enabled = false;
    double size = 1;
    QgsUnitTypes::RenderUnit sizeUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale sizeMapUnitScale;
    QColor color;
    double opacity = 1.0;
    bool fillBufferInterior = false;
    Qt::PenJoinStyle joinStyle = Qt::RoundJoin;
    QPainter::CompositionMode blendMode = QPainter::CompositionMode_SourceOver;
    QgsPaintEffect *paintEffect = nullptr;
};


class QgsTextBackgroundSettingsPrivate : public QSharedData
{
  public:

    QgsTextBackgroundSettingsPrivate()
      : size( QSizeF( 0.0, 0.0 ) )
      , offset( QPointF( 0.0, 0.0 ) )
      , radii( QSizeF( 0.0, 0.0 ) )
      , fillColor( Qt::white )
      , strokeColor( Qt::darkGray )
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
      , paintEffect( other.paintEffect ? other.paintEffect->clone() : nullptr )
    {
    }

    ~QgsTextBackgroundSettingsPrivate()
    {
      delete paintEffect;
    }

    bool enabled = false;
    QgsTextBackgroundSettings::ShapeType type = QgsTextBackgroundSettings::ShapeRectangle;
    QString svgFile;   //!< Absolute path to SVG file
    QgsTextBackgroundSettings::SizeType sizeType = QgsTextBackgroundSettings::SizeBuffer;
    QSizeF size;
    QgsUnitTypes::RenderUnit sizeUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale sizeMapUnitScale;
    QgsTextBackgroundSettings::RotationType rotationType = QgsTextBackgroundSettings::RotationSync;
    double rotation = 0.0;
    QPointF offset;
    QgsUnitTypes::RenderUnit offsetUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale offsetMapUnitScale;
    QSizeF radii;
    QgsUnitTypes::RenderUnit radiiUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale radiiMapUnitScale;
    QPainter::CompositionMode blendMode = QPainter::CompositionMode_SourceOver;
    QColor fillColor;
    QColor strokeColor;
    double opacity = 1.0;
    double strokeWidth = 0.0;
    QgsUnitTypes::RenderUnit strokeWidthUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale strokeWidthMapUnitScale;
    Qt::PenJoinStyle joinStyle = Qt::BevelJoin;
    QgsPaintEffect *paintEffect = nullptr;
};



class QgsTextShadowSettingsPrivate : public QSharedData
{
  public:

    QgsTextShadowSettingsPrivate()
      : color( QColor( 0, 0, 0 ) )
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

    bool enabled = false;
    QgsTextShadowSettings::ShadowPlacement shadowUnder = QgsTextShadowSettings::ShadowLowest;
    int offsetAngle = 135;
    double offsetDist = 1.0;
    QgsUnitTypes::RenderUnit offsetUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale offsetMapUnitScale;
    bool offsetGlobal = true;
    double radius = 1.5;
    QgsUnitTypes::RenderUnit radiusUnits = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale radiusMapUnitScale;
    bool radiusAlphaOnly = false;
    int scale = 100;
    QColor color;
    double opacity = 0.7;
    QPainter::CompositionMode blendMode = QPainter::CompositionMode_Multiply;
};


class QgsTextSettingsPrivate : public QSharedData
{
  public:

    QgsTextSettingsPrivate()
      : textColor( Qt::black )
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
    QgsUnitTypes::RenderUnit fontSizeUnits = QgsUnitTypes::RenderPoints;
    QgsMapUnitScale fontSizeMapUnitScale;
    double fontSize = 10 ; //may differ from size in textFont due to units (e.g., size in map units)
    QColor textColor;
    double opacity = 1.0;
    QPainter::CompositionMode blendMode = QPainter::CompositionMode_SourceOver;
    double multilineHeight = 1.0 ; //0.0 to 10.0, leading between lines as multiplyer of line height

};





/// @endcond

#endif // QGSTEXTRENDERER_PRIVATE_H
