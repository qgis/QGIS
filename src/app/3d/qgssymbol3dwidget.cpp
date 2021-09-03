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
#include "qgsabstractmaterialsettings.h"
#include "qgsstyleitemslistwidget.h"
#include "qgsstylesavedialog.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3dsymbolwidget.h"
#include "qgsabstract3dsymbol.h"
#include <QStackedWidget>
#include <QMessageBox>


QgsSymbol3DWidget::QgsSymbol3DWidget( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  widgetUnsupported = new QLabel( tr( "Sorry, this symbol is not supported." ), this );

  widgetStack = new QStackedWidget( this );
  widgetStack->addWidget( widgetUnsupported );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( widgetStack );

  mStyleWidget = new QgsStyleItemsListWidget( this );
  mStyleWidget->setStyle( QgsStyle::defaultStyle() );
  mStyleWidget->setEntityType( QgsStyle::Symbol3DEntity );
  mStyleWidget->setLayerType( mLayer->geometryType() );

  connect( mStyleWidget, &QgsStyleItemsListWidget::selectionChanged, this, &QgsSymbol3DWidget::setSymbolFromStyle );
  connect( mStyleWidget, &QgsStyleItemsListWidget::saveEntity, this, &QgsSymbol3DWidget::saveSymbol );

  layout->addWidget( mStyleWidget, 1 );
}

std::unique_ptr<QgsAbstract3DSymbol> QgsSymbol3DWidget::symbol()
{
  if ( Qgs3DSymbolWidget *w = qobject_cast< Qgs3DSymbolWidget * >( widgetStack->currentWidget() ) )
  {
    return std::unique_ptr< QgsAbstract3DSymbol >( w->symbol() );
  }
  return nullptr;
}

void QgsSymbol3DWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vlayer )
{
  mLayer = vlayer;
  mStyleWidget->setLayerType( mLayer->geometryType() );

  if ( Qgs3DSymbolWidget *w = qobject_cast< Qgs3DSymbolWidget * >( widgetStack->currentWidget() ) )
  {
    if ( w->symbolType() == symbol->type() )
    {
      // we can reuse the existing widget
      w->setSymbol( symbol, mLayer );
      return;
    }
  }

  updateSymbolWidget( symbol );
}

void QgsSymbol3DWidget::setSymbolFromStyle( const QString &name )
{
  // get new instance of symbol from style
  const std::unique_ptr< QgsAbstract3DSymbol > s( QgsStyle::defaultStyle()->symbol3D( name ) );
  if ( !s )
    return;

  setSymbol( s.get(), mLayer );
}

void QgsSymbol3DWidget::saveSymbol()
{
  QgsStyleSaveDialog saveDlg( this, QgsStyle::Symbol3DEntity );
  saveDlg.setDefaultTags( mStyleWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  std::unique_ptr< QgsAbstract3DSymbol > newSymbol( symbol() );

  // check if there is no symbol with same name
  if ( QgsStyle::defaultStyle()->symbol3DNames().contains( saveDlg.name() ) )
  {
    const int res = QMessageBox::warning( this, tr( "Save 3D Symbol" ),
                                          tr( "A 3D symbol with the name '%1' already exists. Overwrite?" )
                                          .arg( saveDlg.name() ),
                                          QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    QgsStyle::defaultStyle()->removeEntityByName( QgsStyle::Symbol3DEntity, saveDlg.name() );
  }

  const QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  QgsAbstract3DSymbol *s = newSymbol.get();
  QgsStyle::defaultStyle()->addSymbol3D( saveDlg.name(), newSymbol.release() );

  // make sure the symbol is stored
  QgsStyle::defaultStyle()->saveSymbol3D( saveDlg.name(), s, saveDlg.isFavorite(), symbolTags );
}

void QgsSymbol3DWidget::updateSymbolWidget( const QgsAbstract3DSymbol *newSymbol )
{
  if ( widgetStack->currentWidget() != widgetUnsupported )
  {
    // stop updating from the original widget
    if ( Qgs3DSymbolWidget *w = qobject_cast< Qgs3DSymbolWidget * >( widgetStack->currentWidget() ) )
      disconnect( w, &Qgs3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
    widgetStack->removeWidget( widgetStack->currentWidget() );
  }

  const QString symbolType = newSymbol->type();
  if ( Qgs3DSymbolAbstractMetadata *am = QgsApplication::symbol3DRegistry()->symbolMetadata( symbolType ) )
  {
    if ( Qgs3DSymbolWidget *w = am->createSymbolWidget( mLayer ) )
    {
      w->setSymbol( newSymbol, mLayer );
      widgetStack->addWidget( w );
      widgetStack->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &Qgs3DSymbolWidget::changed, this, &QgsSymbol3DWidget::widgetChanged );
      return;
    }
  }
  // When anything is not right
  widgetStack->setCurrentWidget( widgetUnsupported );
}
