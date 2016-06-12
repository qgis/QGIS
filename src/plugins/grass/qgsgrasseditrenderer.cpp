/***************************************************************************
    qgsgrasseditrenderer.cpp
                             -------------------
    begin                : February, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include  <QVBoxLayout>

#include "qgscategorizedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsfeature.h"
#include "qgslinesymbollayerv2.h"
#include "qgslogger.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsrendererv2registry.h"
#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include "qgsgrasseditrenderer.h"
#include "qgsgrassprovider.h"

QgsGrassEditRenderer::QgsGrassEditRenderer()
    : QgsFeatureRendererV2( "grassEdit" )
    , mLineRenderer( 0 )
    , mMarkerRenderer( 0 )
{
  QHash<int, QColor> colors;
  //colors.insert( QgsGrassVectorMap::TopoUndefined, QColor( 125, 125, 125 ) );
  colors.insert( QgsGrassVectorMap::TopoLine, QColor( Qt::black ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryError, QColor( Qt::red ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryErrorLeft, QColor( 255, 125, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryErrorRight, QColor( 255, 125, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryOk, QColor( Qt::green ) );

  QHash<int, QString> labels;
  //labels.insert( QgsGrassVectorMap::TopoUndefined, "Unknown type" );
  labels.insert( QgsGrassVectorMap::TopoLine, "Line" );
  labels.insert( QgsGrassVectorMap::TopoBoundaryError, "Boundary (topological error on both sides)" );
  labels.insert( QgsGrassVectorMap::TopoBoundaryErrorLeft, "Boundary (topological error on the left side)" );
  labels.insert( QgsGrassVectorMap::TopoBoundaryErrorRight, "Boundary (topological error on the right side)" );
  labels.insert( QgsGrassVectorMap::TopoBoundaryOk, "Boundary (correct)" );

  QgsCategoryList categoryList;

  // first/last vertex marker to distinguish vertices from nodes
  QgsMarkerLineSymbolLayerV2 * firstVertexMarkerLine = new QgsMarkerLineSymbolLayerV2( false );
  QgsSimpleMarkerSymbolLayerV2 *markerSymbolLayer = new QgsSimpleMarkerSymbolLayerV2( QgsSimpleMarkerSymbolLayerBase::Cross2, 2 );
  markerSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  markerSymbolLayer->setBorderColor( QColor( 255, 0, 0 ) );
  markerSymbolLayer->setOutlineWidth( 0.5 );
  QgsSymbolLayerV2List markerLayers;
  markerLayers << markerSymbolLayer;
  QgsMarkerSymbolV2 * markerSymbol = new QgsMarkerSymbolV2( markerLayers );
  firstVertexMarkerLine->setSubSymbol( markerSymbol );
  firstVertexMarkerLine->setPlacement( QgsMarkerLineSymbolLayerV2::FirstVertex );
  QgsMarkerLineSymbolLayerV2 * lastVertexMarkerLine = static_cast<QgsMarkerLineSymbolLayerV2 *>( firstVertexMarkerLine->clone() );
  lastVertexMarkerLine->setPlacement( QgsMarkerLineSymbolLayerV2::LastVertex );
  Q_FOREACH ( int value, colors.keys() )
  {
    QgsSymbolV2 * symbol = QgsSymbolV2::defaultSymbol( QGis::Line );
    symbol->setColor( colors.value( value ) );
    symbol->appendSymbolLayer( firstVertexMarkerLine->clone() );
    symbol->appendSymbolLayer( lastVertexMarkerLine->clone() );
    categoryList << QgsRendererCategoryV2( QVariant( value ), symbol, labels.value( value ) );
  }
  delete firstVertexMarkerLine;
  delete lastVertexMarkerLine;
  mLineRenderer = new QgsCategorizedSymbolRendererV2( "topo_symbol", categoryList );

  colors.clear();
  labels.clear();

  colors.insert( QgsGrassVectorMap::TopoPoint, QColor( 0, 255, 255 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidIn, QColor( 0, 255, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidOut, QColor( 255, 0, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidDupl, QColor( 255, 0, 255 ) );

  labels.insert( QgsGrassVectorMap::TopoPoint, "Point" );
  labels.insert( QgsGrassVectorMap::TopoCentroidIn, "Centroid in area" );
  labels.insert( QgsGrassVectorMap::TopoCentroidOut, "Centroid outside area" );
  labels.insert( QgsGrassVectorMap::TopoCentroidDupl, "Duplicate centroid" );

  categoryList.clear();

  Q_FOREACH ( int value, colors.keys() )
  {
    QgsSymbolV2 * symbol = QgsSymbolV2::defaultSymbol( QGis::Point );
    symbol->setColor( colors.value( value ) );
    categoryList << QgsRendererCategoryV2( QVariant( value ), symbol, labels.value( value ) );
  }

  mMarkerRenderer = new QgsCategorizedSymbolRendererV2( "topo_symbol", categoryList );
}

QgsGrassEditRenderer::~QgsGrassEditRenderer()
{
  delete mMarkerRenderer;
  delete mLineRenderer;
}

void QgsGrassEditRenderer::setLineRenderer( QgsFeatureRendererV2 *renderer )
{
  delete mLineRenderer;
  mLineRenderer = renderer;
}
void QgsGrassEditRenderer::setMarkerRenderer( QgsFeatureRendererV2 *renderer )
{
  delete mMarkerRenderer;
  mMarkerRenderer = renderer;
}

QgsSymbolV2* QgsGrassEditRenderer::symbolForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  int symbolCode = feature.attribute( "topo_symbol" ).toInt();
  QgsDebugMsgLevel( QString( "fid = %1 symbolCode = %2" ).arg( feature.id() ).arg( symbolCode ), 3 );

  QgsSymbolV2* symbol = 0;
  if ( symbolCode == QgsGrassVectorMap::TopoPoint || symbolCode == QgsGrassVectorMap::TopoCentroidIn ||
       symbolCode == QgsGrassVectorMap::TopoCentroidOut || symbolCode == QgsGrassVectorMap::TopoCentroidDupl ||
       symbolCode == QgsGrassVectorMap::TopoNode0 || symbolCode == QgsGrassVectorMap::TopoNode1 ||
       symbolCode == QgsGrassVectorMap::TopoNode2 )
  {
    symbol = mMarkerRenderer->symbolForFeature( feature, context );
  }
  else if ( symbolCode == QgsGrassVectorMap::TopoLine || symbolCode == QgsGrassVectorMap::TopoBoundaryError ||
            symbolCode == QgsGrassVectorMap::TopoBoundaryErrorLeft || symbolCode == QgsGrassVectorMap::TopoBoundaryErrorRight ||
            symbolCode == QgsGrassVectorMap::TopoBoundaryOk )
  {
    symbol = mLineRenderer->symbolForFeature( feature, context );
  }
  else
  {
    // should not happen
    QgsDebugMsg( "unknown symbol code" );
  }

  if ( symbol )
  {
    QgsDebugMsgLevel( "color = " + symbol->color().name(), 3 );
  }
  else
  {
    QgsDebugMsgLevel( "no symbol", 3 );
  }

  return symbol;
}

void QgsGrassEditRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  Q_UNUSED( fields );
  // TODO better
  //QgsFields topoFields;
  //topoFields.append( QgsField( "topo_symbol", QVariant::Int, "int" ) );
  mLineRenderer->startRender( context, fields );
  mMarkerRenderer->startRender( context, fields );
}

void QgsGrassEditRenderer::stopRender( QgsRenderContext& context )
{
  mLineRenderer->stopRender( context );
  mMarkerRenderer->stopRender( context );
}

QList<QString> QgsGrassEditRenderer::usedAttributes()
{
  return mLineRenderer->usedAttributes();
}

QgsFeatureRendererV2* QgsGrassEditRenderer::clone() const
{
  QgsGrassEditRenderer* r = new QgsGrassEditRenderer();
  if ( mLineRenderer )
  {
    r->mLineRenderer = mLineRenderer->clone();
  }
  if ( mMarkerRenderer )
  {
    r->mMarkerRenderer = mMarkerRenderer->clone();
  }
  return r;
}

QgsSymbolV2List QgsGrassEditRenderer::symbols( QgsRenderContext& context )
{
  return mLineRenderer->symbols( context );
}

QString QgsGrassEditRenderer::dump() const
{
  return "GRASS edit renderer";
}

QDomElement QgsGrassEditRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "grassEdit" );

  QDomElement lineElem = doc.createElement( "line" );
  rendererElem.appendChild( lineElem );
  lineElem.appendChild( mLineRenderer->save( doc ) );

  QDomElement pointElem = doc.createElement( "marker" );
  rendererElem.appendChild( pointElem );
  pointElem.appendChild( mMarkerRenderer->save( doc ) );

  return rendererElem;
}


QgsFeatureRendererV2* QgsGrassEditRenderer::create( QDomElement& element )
{
  QgsGrassEditRenderer *renderer = new QgsGrassEditRenderer();

  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QDomElement elem = childElem.firstChildElement();
    if ( !elem.isNull() )
    {
      QString rendererType = elem.attribute( "type" );
      QgsDebugMsg( "childElem.tagName() = " + childElem.tagName() + " rendererType = " + rendererType );
      QgsRendererV2AbstractMetadata* meta = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
      if ( meta )
      {
        QgsFeatureRendererV2* subRenderer = meta->createRenderer( elem );
        if ( subRenderer )
        {
          QgsDebugMsg( "renderer created : " + renderer->type() );
          if ( childElem.tagName() == "line" )
          {
            renderer->setLineRenderer( subRenderer );
          }
          else if ( childElem.tagName() == "marker" )
          {
            renderer->setMarkerRenderer( subRenderer );
          }
        }
      }
    }
    childElem = childElem.nextSiblingElement();
  }
  return renderer;
}

//--------------------------------------- QgsGrassEditRendererWidget --------------------------------------------

QgsRendererV2Widget* QgsGrassEditRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsGrassEditRendererWidget( layer, style, renderer );
}

QgsGrassEditRendererWidget::QgsGrassEditRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( 0 )
    , mLineRendererWidget( 0 )
    , mPointRendererWidget( 0 )
{
  mRenderer = dynamic_cast<QgsGrassEditRenderer*>( renderer->clone() );
  if ( !mRenderer )
  {
    mRenderer = new QgsGrassEditRenderer();
  }

  QVBoxLayout* layout = new QVBoxLayout( this );

  mLineRendererWidget = QgsCategorizedSymbolRendererV2Widget::create( layer, style, mRenderer->lineRenderer()->clone() );
  layout->addWidget( mLineRendererWidget );

  mPointRendererWidget = QgsCategorizedSymbolRendererV2Widget::create( layer, style, mRenderer->pointRenderer()->clone() );
  layout->addWidget( mPointRendererWidget );
}

QgsGrassEditRendererWidget::~QgsGrassEditRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRendererV2* QgsGrassEditRendererWidget::renderer()
{
  mRenderer->setLineRenderer( mLineRendererWidget->renderer()->clone() );
  mRenderer->setMarkerRenderer( mPointRendererWidget->renderer()->clone() );
  return mRenderer;
}



