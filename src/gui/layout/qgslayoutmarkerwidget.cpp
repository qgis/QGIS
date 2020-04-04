/***************************************************************************
                         qgslayoutmarkerwidget.cpp
                         --------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgslayoutmarkerwidget.h"
#include "qgsstyle.h"
#include "qgslayoutitemmarker.h"
#include "qgslayout.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"

QgsLayoutMarkerWidget::QgsLayoutMarkerWidget( QgsLayoutItemMarker *marker )
  : QgsLayoutItemBaseWidget( nullptr, marker )
  , mMarker( marker )
{
  Q_ASSERT( mMarker );

  setupUi( this );
  setPanelTitle( tr( "Marker Properties" ) );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, marker );
  mItemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( mItemPropertiesWidget );

  blockAllSignals( true );

  mShapeStyleButton->setSymbolType( QgsSymbol::Marker );

  setGuiElementValues();

  blockAllSignals( false );

  connect( mMarker, &QgsLayoutObject::changed, this, &QgsLayoutMarkerWidget::setGuiElementValues );
  mShapeStyleButton->registerExpressionContextGenerator( mMarker );

  connect( mShapeStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutMarkerWidget::symbolChanged );

  mShapeStyleButton->setLayer( coverageLayer() );
  if ( mMarker->layout() )
  {
    connect( &mMarker->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mShapeStyleButton, &QgsSymbolButton::setLayer );
  }
}

void QgsLayoutMarkerWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutMarkerWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutShape )
    return false;

  if ( mMarker )
  {
    disconnect( mMarker, &QgsLayoutObject::changed, this, &QgsLayoutMarkerWidget::setGuiElementValues );
  }

  mMarker = qobject_cast< QgsLayoutItemMarker * >( item );
  mItemPropertiesWidget->setItem( mMarker );

  if ( mMarker )
  {
    connect( mMarker, &QgsLayoutObject::changed, this, &QgsLayoutMarkerWidget::setGuiElementValues );
    mShapeStyleButton->registerExpressionContextGenerator( mMarker );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutMarkerWidget::blockAllSignals( bool block )
{
  mShapeStyleButton->blockSignals( block );
}

void QgsLayoutMarkerWidget::setGuiElementValues()
{
  if ( !mMarker )
  {
    return;
  }

  blockAllSignals( true );

  mShapeStyleButton->setSymbol( mMarker->symbol()->clone() );

  blockAllSignals( false );
}

void QgsLayoutMarkerWidget::symbolChanged()
{
  if ( !mMarker )
    return;

  mMarker->layout()->undoStack()->beginCommand( mMarker, tr( "Change Marker Symbol" ), QgsLayoutItem::UndoShapeStyle );
  mMarker->setSymbol( mShapeStyleButton->clonedSymbol<QgsMarkerSymbol>() );
  mMarker->layout()->undoStack()->endCommand();
}
