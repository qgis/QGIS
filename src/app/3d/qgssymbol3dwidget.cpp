/***************************************************************************
  qgssymbol3dwidget.cpp
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbol3dwidget.h"

#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"

#include "qgsline3dsymbolwidget.h"
#include "qgspoint3dsymbolwidget.h"
#include "qgspolygon3dsymbolwidget.h"

#include "qgsvectorlayer.h"

#include <QStackedWidget>


QgsSymbol3DWidget::QgsSymbol3DWidget( QWidget *parent ) : QWidget( parent )
{
  widgetUnsupported = new QLabel( tr( "Sorry, this symbol is not supported." ), this );
  widgetLine = new QgsLine3DSymbolWidget( this );
  widgetPoint = new QgsPoint3DSymbolWidget( this );
  widgetPolygon = new QgsPolygon3DSymbolWidget( this );

  widgetStack = new QStackedWidget( this );
  widgetStack->addWidget( widgetUnsupported );
  widgetStack->addWidget( widgetLine );
  widgetStack->addWidget( widgetPoint );
  widgetStack->addWidget( widgetPolygon );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( widgetStack );

  connect( widgetLine, &QgsLine3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
  connect( widgetPoint, &QgsPoint3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
  connect( widgetPolygon, &QgsPolygon3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
}


QgsAbstract3DSymbol *QgsSymbol3DWidget::symbol()
{
  int pageIndex = widgetStack->currentIndex();
  if ( pageIndex == 1 || pageIndex == 2 || pageIndex == 3 )
  {
    QgsAbstract3DSymbol *sym = nullptr;
    if ( pageIndex == 1 )
      sym = new QgsLine3DSymbol( widgetLine->symbol() );
    else if ( pageIndex == 2 )
      sym = new QgsPoint3DSymbol( widgetPoint->symbol() );
    else
      sym = new QgsPolygon3DSymbol( widgetPolygon->symbol() );
    return sym;
  }
  return nullptr;
}

void QgsSymbol3DWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vlayer )
{
  int pageIndex;
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      pageIndex = 2;
      if ( symbol && symbol->type() == QLatin1String( "point" ) )
      {
        whileBlocking( widgetPoint )->setSymbol( *static_cast<const QgsPoint3DSymbol *>( symbol ) );
      }
      else
      {
        whileBlocking( widgetPoint )->setSymbol( QgsPoint3DSymbol() );
      }
      break;

    case QgsWkbTypes::LineGeometry:
      pageIndex = 1;
      if ( symbol && symbol->type() == QLatin1String( "line" ) )
      {
        whileBlocking( widgetLine )->setSymbol( *static_cast<const QgsLine3DSymbol *>( symbol ) );
      }
      else
      {
        whileBlocking( widgetLine )->setSymbol( QgsLine3DSymbol() );
      }
      break;

    case QgsWkbTypes::PolygonGeometry:
      pageIndex = 3;
      if ( symbol && symbol->type() == QLatin1String( "polygon" ) )
      {
        whileBlocking( widgetPolygon )->setSymbol( *static_cast<const QgsPolygon3DSymbol *>( symbol ), vlayer );
      }
      else
      {
        whileBlocking( widgetPolygon )->setSymbol( QgsPolygon3DSymbol(), vlayer );
      }
      break;

    default:
      pageIndex = 0;   // unsupported
      break;
  }
  widgetStack->setCurrentIndex( pageIndex );
}
