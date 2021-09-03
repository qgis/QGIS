/***************************************************************************
                             qgsannotationitemwidget_impl.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsannotationitemwidget_impl.h"

#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationmarkeritem.h"

QgsAnnotationPolygonItemWidget::QgsAnnotationPolygonItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
}

QgsAnnotationItem *QgsAnnotationPolygonItemWidget::createItem()
{
  QgsAnnotationPolygonItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  return newItem;
}

void QgsAnnotationPolygonItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

QgsAnnotationPolygonItemWidget::~QgsAnnotationPolygonItemWidget() = default;

bool QgsAnnotationPolygonItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationPolygonItem *polygonItem = dynamic_cast< QgsAnnotationPolygonItem * >( item );
  if ( !polygonItem )
    return false;

  mItem.reset( polygonItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsFillSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mBlockChangedSignal = false;

  return true;
}


//
// QgsAnnotationLineItemWidget
//

QgsAnnotationLineItemWidget::QgsAnnotationLineItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
}

QgsAnnotationItem *QgsAnnotationLineItemWidget::createItem()
{
  QgsAnnotationLineItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  return newItem;
}

void QgsAnnotationLineItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

QgsAnnotationLineItemWidget::~QgsAnnotationLineItemWidget() = default;

bool QgsAnnotationLineItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationLineItem *lineItem = dynamic_cast< QgsAnnotationLineItem * >( item );
  if ( !lineItem )
    return false;

  mItem.reset( lineItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsLineSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mBlockChangedSignal = false;

  return true;
}


//
// QgsAnnotationMarkerItemWidget
//

QgsAnnotationMarkerItemWidget::QgsAnnotationMarkerItemWidget( QWidget *parent )
  : QgsAnnotationItemBaseWidget( parent )
{
  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSymbol.get(), QgsStyle::defaultStyle(), nullptr, nullptr );
  mSelector->setDockMode( dockMode() );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, [ = ]
  {
    if ( !mBlockChangedSignal )
      emit itemChanged();
  } );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );
}

QgsAnnotationItem *QgsAnnotationMarkerItemWidget::createItem()
{
  QgsAnnotationMarkerItem *newItem = mItem->clone();
  newItem->setSymbol( mSymbol->clone() );
  return newItem;
}

void QgsAnnotationMarkerItemWidget::setDockMode( bool dockMode )
{
  QgsAnnotationItemBaseWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

QgsAnnotationMarkerItemWidget::~QgsAnnotationMarkerItemWidget() = default;

bool QgsAnnotationMarkerItemWidget::setNewItem( QgsAnnotationItem *item )
{
  QgsAnnotationMarkerItem *markerItem = dynamic_cast< QgsAnnotationMarkerItem * >( item );
  if ( !markerItem )
    return false;

  mItem.reset( markerItem->clone() );
  if ( mItem->symbol() )
  {
    mSymbol.reset( mItem->symbol()->clone() );
  }
  else
  {
    mSymbol.reset( QgsMarkerSymbol::createSimple( {} ) );
  }
  mBlockChangedSignal = true;
  mSelector->loadSymbol( mSymbol.get() );
  mSelector->updatePreview();
  mBlockChangedSignal = false;

  return true;
}


