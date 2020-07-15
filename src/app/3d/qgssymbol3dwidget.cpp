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
  layout->setContentsMargins( 0, 0, 0, 0 );
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
      sym = widgetLine->symbol();
    else if ( pageIndex == 2 )
      sym = widgetPoint->symbol();
    else
      sym = widgetPolygon->symbol();
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
        whileBlocking( widgetPoint )->setSymbol( symbol, vlayer );
      }
      else
      {
        QgsPoint3DSymbol defaultSymbol;
        whileBlocking( widgetPoint )->setSymbol( &defaultSymbol, vlayer );
      }
      break;

    case QgsWkbTypes::LineGeometry:
      pageIndex = 1;
      if ( symbol && symbol->type() == QLatin1String( "line" ) )
      {
        whileBlocking( widgetLine )->setSymbol( symbol, vlayer );
      }
      else
      {
        QgsLine3DSymbol defaultSymbol;
        whileBlocking( widgetLine )->setSymbol( &defaultSymbol, vlayer );
      }
      break;

    case QgsWkbTypes::PolygonGeometry:
      pageIndex = 3;
      if ( symbol && symbol->type() == QLatin1String( "polygon" ) )
      {
        whileBlocking( widgetPolygon )->setSymbol( symbol, vlayer );
      }
      else
      {
        QgsPolygon3DSymbol defaultSymbol;
        whileBlocking( widgetPolygon )->setSymbol( &defaultSymbol, vlayer );
      }
      break;

    default:
      pageIndex = 0;   // unsupported
      break;
  }
  widgetStack->setCurrentIndex( pageIndex );
}
