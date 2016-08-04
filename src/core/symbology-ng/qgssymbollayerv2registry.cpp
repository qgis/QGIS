/***************************************************************************
    qgssymbollayerv2registry.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerv2registry.h"

#include "qgsarrowsymbollayer.h"
#include "qgsellipsesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgsvectorfieldsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayerv2.h"

QgsSymbolLayerV2Registry::QgsSymbolLayerV2Registry()
{
  // init registry with known symbol layers
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "SimpleLine", QObject::tr( "Simple line" ), QgsSymbolV2::Line,
                      QgsSimpleLineSymbolLayerV2::create, QgsSimpleLineSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "MarkerLine", QObject::tr( "Marker line" ), QgsSymbolV2::Line,
                      QgsMarkerLineSymbolLayerV2::create, QgsMarkerLineSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "ArrowLine", QObject::tr( "Arrow" ), QgsSymbolV2::Line, QgsArrowSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "SimpleMarker", QObject::tr( "Simple marker" ), QgsSymbolV2::Marker,
                      QgsSimpleMarkerSymbolLayerV2::create, QgsSimpleMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "FilledMarker", QObject::tr( "Filled marker" ), QgsSymbolV2::Marker,
                      QgsFilledMarkerSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "SvgMarker", QObject::tr( "SVG marker" ), QgsSymbolV2::Marker,
                      QgsSvgMarkerSymbolLayerV2::create, QgsSvgMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "FontMarker", QObject::tr( "Font marker" ), QgsSymbolV2::Marker,
                      QgsFontMarkerSymbolLayerV2::create, QgsFontMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "EllipseMarker", QObject::tr( "Ellipse marker" ), QgsSymbolV2::Marker,
                      QgsEllipseSymbolLayerV2::create, QgsEllipseSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "VectorField", QObject::tr( "Vector field marker" ), QgsSymbolV2::Marker,
                      QgsVectorFieldSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "SimpleFill", QObject::tr( "Simple fill" ), QgsSymbolV2::Fill,
                      QgsSimpleFillSymbolLayerV2::create, QgsSimpleFillSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "GradientFill", QObject::tr( "Gradient fill" ), QgsSymbolV2::Fill,
                      QgsGradientFillSymbolLayerV2::create ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "ShapeburstFill", QObject::tr( "Shapeburst fill" ), QgsSymbolV2::Fill,
                      QgsShapeburstFillSymbolLayerV2::create ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "RasterFill", QObject::tr( "Raster image fill" ), QgsSymbolV2::Fill,
                      QgsRasterFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "SVGFill", QObject::tr( "SVG fill" ), QgsSymbolV2::Fill,
                      QgsSVGFillSymbolLayer::create, QgsSVGFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "CentroidFill", QObject::tr( "Centroid fill" ), QgsSymbolV2::Fill,
                      QgsCentroidFillSymbolLayerV2::create, QgsCentroidFillSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "LinePatternFill", QObject::tr( "Line pattern fill" ), QgsSymbolV2::Fill,
                      QgsLinePatternFillSymbolLayer::create, QgsLinePatternFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "PointPatternFill", QObject::tr( "Point pattern fill" ), QgsSymbolV2::Fill,
                      QgsPointPatternFillSymbolLayer::create, QgsPointPatternFillSymbolLayer::createFromSld ) );

  addSymbolLayerType( new QgsSymbolLayerV2Metadata( "GeometryGenerator", QObject::tr( "Geometry generator" ), QgsSymbolV2::Hybrid,
                      QgsGeometryGeneratorSymbolLayerV2::create ) );
}

QgsSymbolLayerV2Registry::~QgsSymbolLayerV2Registry()
{
  qDeleteAll( mMetadata );
}

bool QgsSymbolLayerV2Registry::addSymbolLayerType( QgsSymbolLayerV2AbstractMetadata* metadata )
{
  if ( !metadata || mMetadata.contains( metadata->name() ) )
    return false;

  mMetadata[metadata->name()] = metadata;
  return true;
}


QgsSymbolLayerV2AbstractMetadata* QgsSymbolLayerV2Registry::symbolLayerMetadata( const QString& name ) const
{
  return mMetadata.value( name );
}

QgsSymbolLayerV2Registry* QgsSymbolLayerV2Registry::instance()
{
  static QgsSymbolLayerV2Registry mInstance;
  return &mInstance;
}

QgsSymbolLayerV2* QgsSymbolLayerV2Registry::defaultSymbolLayer( QgsSymbolV2::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbolV2::Marker:
      return QgsSimpleMarkerSymbolLayerV2::create();

    case QgsSymbolV2::Line:
      return QgsSimpleLineSymbolLayerV2::create();

    case QgsSymbolV2::Fill:
      return QgsSimpleFillSymbolLayerV2::create();

    case QgsSymbolV2::Hybrid:
      return nullptr;
  }

  return nullptr;
}


QgsSymbolLayerV2* QgsSymbolLayerV2Registry::createSymbolLayer( const QString& name, const QgsStringMap& properties ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayer( properties );
}

QgsSymbolLayerV2* QgsSymbolLayerV2Registry::createSymbolLayerFromSld( const QString& name, QDomElement& element ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayerFromSld( element );
}

QStringList QgsSymbolLayerV2Registry::symbolLayersForType( QgsSymbolV2::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerV2AbstractMetadata*>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type || it.value()->type() == QgsSymbolV2::Hybrid )
      lst.append( it.key() );
  }
  return lst;
}
