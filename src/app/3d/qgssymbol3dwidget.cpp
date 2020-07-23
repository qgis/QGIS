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
#include "qgsabstractmaterialsettings.h"
#include "qgsstyleitemslistwidget.h"
#include "qgsstylesavedialog.h"
#include "qgsvectorlayer.h"

#include <QStackedWidget>
#include <QMessageBox>


QgsSymbol3DWidget::QgsSymbol3DWidget( QWidget *parent )
  : QWidget( parent )
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

  mStyleWidget = new QgsStyleItemsListWidget( this );
  mStyleWidget->setStyle( QgsStyle::defaultStyle() );
  mStyleWidget->setEntityType( QgsStyle::Symbol3DEntity );

  connect( mStyleWidget, &QgsStyleItemsListWidget::selectionChanged, this, &QgsSymbol3DWidget::setSymbolFromStyle );
  connect( mStyleWidget, &QgsStyleItemsListWidget::saveEntity, this, &QgsSymbol3DWidget::saveSymbol );

  layout->addWidget( mStyleWidget );

  connect( widgetLine, &QgsLine3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
  connect( widgetPoint, &QgsPoint3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
  connect( widgetPolygon, &QgsPolygon3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
}

std::unique_ptr<QgsAbstract3DSymbol> QgsSymbol3DWidget::symbol()
{
  int pageIndex = widgetStack->currentIndex();
  if ( pageIndex == 1 || pageIndex == 2 || pageIndex == 3 )
  {
    std::unique_ptr< QgsAbstract3DSymbol > sym;
    if ( pageIndex == 1 )
      sym.reset( widgetLine->symbol() );
    else if ( pageIndex == 2 )
      sym.reset( widgetPoint->symbol() );
    else
      sym.reset( widgetPolygon->symbol() );
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

void QgsSymbol3DWidget::setSymbolFromStyle( const QString &name )
{
  // get new instance of symbol from style
  std::unique_ptr< QgsAbstract3DSymbol > s( QgsStyle::defaultStyle()->symbol3D( name ) );
  if ( !s )
    return;

  if ( s->type() == QStringLiteral( "point" ) )
    widgetPoint->setSymbol( s.release(), nullptr );
  else if ( s->type() == QStringLiteral( "line" ) )
    widgetLine->setSymbol( s.release(), nullptr );
  else if ( s->type() == QStringLiteral( "polygon" ) )
    widgetPolygon->setSymbol( s.release(), nullptr );
}

void QgsSymbol3DWidget::saveSymbol()
{
  QgsStyleSaveDialog saveDlg( this, QgsStyle::Symbol3DEntity );
  saveDlg.setDefaultTags( mStyleWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( QgsStyle::defaultStyle()->symbol3DNames().contains( saveDlg.name() ) )
  {
    int res = QMessageBox::warning( this, tr( "Save 3D Symbol" ),
                                    tr( "A 3D symbol with the name '%1' already exists. Overwrite?" )
                                    .arg( saveDlg.name() ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    QgsStyle::defaultStyle()->removeEntityByName( QgsStyle::Symbol3DEntity, saveDlg.name() );
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  QgsAbstract3DSymbol *newSymbol = symbol();
  QgsStyle::defaultStyle()->addSymbol3D( saveDlg.name(), newSymbol );

  // make sure the symbol is stored
  QgsStyle::defaultStyle()->saveSymbol3D( saveDlg.name(), newSymbol, saveDlg.isFavorite(), symbolTags );
}
