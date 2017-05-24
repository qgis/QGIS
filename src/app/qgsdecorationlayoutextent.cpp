/***************************************************************************
                          qgsdecorationlayoutextent.cpp
                              ----------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationlayoutextent.h"
#include "qgsdecorationlayoutextentdialog.h"

#include "qgslayoutmanager.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgsgeometry.h"
#include "qgscsexception.h"
#include "qgslinesymbollayer.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsreadwritecontext.h"

#include <QPainter>

QgsDecorationLayoutExtent::QgsDecorationLayoutExtent( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomRight;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setName( "Layout Extent" );
  projectRead();
}

void QgsDecorationLayoutExtent::projectRead()
{
  QgsDecorationItem::projectRead();

  QDomDocument doc;
  QDomElement elem;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QString xml = QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Symbol" ) );
  mSymbol.reset( nullptr );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( elem, rwContext ) );
  }
  if ( ! mSymbol )
  {
    mSymbol.reset( new QgsFillSymbol() );
    QgsSimpleLineSymbolLayer *layer = new QgsSimpleLineSymbolLayer( QColor( 0, 0, 0, 100 ), 0, Qt::DashLine );
    mSymbol->changeSymbolLayer( 0, layer );
  }
}

void QgsDecorationLayoutExtent::saveToProject()
{
  QgsDecorationItem::saveToProject();
  // write symbol info to xml
  QDomDocument doc;
  QDomElement elem;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( mSymbol )
  {
    elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "Symbol" ), mSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    // FIXME this works, but XML will not be valid as < is replaced by &lt;
    QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Symbol" ), doc.toString() );
  }
}

void QgsDecorationLayoutExtent::run()
{
  QgsDecorationLayoutExtentDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationLayoutExtent::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( !enabled() )
    return;

  if ( !mSymbol )
    return;

  context.painter()->save();
  context.painter()->setRenderHint( QPainter::Antialiasing, true );
  mSymbol->startRender( context );

  const QgsMapToPixel &m2p = mapSettings.mapToPixel();
  QTransform transform = m2p.transform();

  Q_FOREACH ( QgsComposition *composition, QgsProject::instance()->layoutManager()->compositions() )
  {
    Q_FOREACH ( const QgsComposerMap *map, composition->composerMapItems() )
    {
      QPolygonF extent = map->visibleExtentPolygon();
      QgsGeometry g = QgsGeometry::fromQPolygonF( extent );

      if ( map->crs() !=
           mapSettings.destinationCrs() )
      {
        // reproject extent
        QgsCoordinateTransform ct( map->crs(),
                                   mapSettings.destinationCrs() );
        g = g.densifyByCount( 20 );
        try
        {
          g.transform( ct );
        }
        catch ( QgsCsException & )
        {
        }
      }

      g.transform( transform );
      extent = g.asQPolygonF();
      mSymbol->renderPolygon( extent, nullptr, nullptr, context );
    }
  }
  mSymbol->stopRender( context );
  context.painter()->restore();
}

QgsFillSymbol *QgsDecorationLayoutExtent::symbol() const
{
  return mSymbol.get();
}

void QgsDecorationLayoutExtent::setSymbol( QgsFillSymbol *symbol )
{
  mSymbol.reset( symbol );
}

