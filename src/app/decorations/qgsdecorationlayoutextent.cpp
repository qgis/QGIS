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

#include <memory>

#include "qgisapp.h"
#include "qgsdecorationlayoutextentdialog.h"
#include "qgsexception.h"
#include "qgsfillsymbol.h"
#include "qgsgeometry.h"
#include "qgslayoutdesignerdialog.h"
#include "qgslayoutitemmap.h"
#include "qgslinesymbollayer.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"

#include <QPainter>

#include "moc_qgsdecorationlayoutextent.cpp"

QgsDecorationLayoutExtent::QgsDecorationLayoutExtent( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomRight;
  mMarginUnit = Qgis::RenderUnit::Millimeters;

  setDisplayName( tr( "Layout Extent" ) );
  mConfigurationName = u"LayoutExtent"_s;

  projectRead();
}

QgsDecorationLayoutExtent::~QgsDecorationLayoutExtent() = default;

void QgsDecorationLayoutExtent::projectRead()
{
  QgsDecorationItem::projectRead();

  QDomDocument doc;
  QDomElement elem;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QString xml = QgsProject::instance()->readEntry( mConfigurationName, u"/Symbol"_s );
  mSymbol.reset( nullptr );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mSymbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( elem, rwContext );
  }
  if ( !mSymbol )
  {
    mSymbol = std::make_unique<QgsFillSymbol>();
    QgsSimpleLineSymbolLayer *layer = new QgsSimpleLineSymbolLayer( QColor( 0, 0, 0, 100 ), 0, Qt::DashLine );
    mSymbol->changeSymbolLayer( 0, layer );
  }

  QString textXml = QgsProject::instance()->readEntry( mConfigurationName, u"/Font"_s );
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    elem = doc.documentElement();
    mTextFormat.readXml( elem, rwContext );
  }
  mLabelExtents = QgsProject::instance()->readBoolEntry( mConfigurationName, u"/Labels"_s, true );
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
    elem = QgsSymbolLayerUtils::saveSymbol( u"Symbol"_s, mSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    // FIXME this works, but XML will not be valid as < is replaced by &lt;
    QgsProject::instance()->writeEntry( mConfigurationName, u"/Symbol"_s, doc.toString() );
  }

  QDomDocument textDoc;
  QDomElement textElem = mTextFormat.writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Font"_s, textDoc.toString() );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Labels"_s, mLabelExtents );
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

  QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();

  mSymbol->startRender( context );

  const QgsMapToPixel &m2p = mapSettings.mapToPixel();
  QTransform transform = m2p.transform();

  // only loop through open layout designers
  const QSet<QgsLayoutDesignerDialog *> designers = QgisApp::instance()->layoutDesigners();

  for ( QgsLayoutDesignerDialog *designer : designers )
  {
    QgsLayout *layout = designer->currentLayout();
    QList<QgsLayoutItemMap *> maps;
    layout->layoutItems( maps );
    for ( const QgsLayoutItemMap *map : std::as_const( maps ) )
    {
      QPolygonF extent = map->visibleExtentPolygon();
      QPointF labelPoint = extent.at( 1 );
      QgsGeometry g = QgsGeometry::fromQPolygonF( extent );

      if ( map->crs() != mapSettings.destinationCrs() )
      {
        // reproject extent
        QgsCoordinateTransform ct( map->crs(), mapSettings.destinationCrs(), QgsProject::instance() );
        g = g.densifyByCount( 20 );
        try
        {
          g.transform( ct );
          labelPoint = ct.transform( labelPoint.x(), labelPoint.y() ).toQPointF();
        }
        catch ( QgsCsException & )
        {
        }
      }

      g.transform( transform );
      labelPoint = transform.map( labelPoint );
      extent = g.asQPolygonF();
      mSymbol->renderPolygon( extent, nullptr, nullptr, context );

      if ( mLabelExtents )
      {
        QgsTextRenderer::drawText( labelPoint, ( map->mapRotation() - mapSettings.rotation() ) * M_PI / 180.0, Qgis::TextHorizontalAlignment::Right, QStringList() << tr( "%1: %2" ).arg( designer->masterLayout()->name(), map->displayName() ), context, mTextFormat );
      }
    }
  }
  mSymbol->stopRender( context );
}

bool QgsDecorationLayoutExtent::labelExtents() const
{
  return mLabelExtents;
}

void QgsDecorationLayoutExtent::setLabelExtents( bool labelExtents )
{
  mLabelExtents = labelExtents;
}

QgsFillSymbol *QgsDecorationLayoutExtent::symbol() const
{
  return mSymbol.get();
}

void QgsDecorationLayoutExtent::setSymbol( QgsFillSymbol *symbol )
{
  mSymbol.reset( symbol );
}
