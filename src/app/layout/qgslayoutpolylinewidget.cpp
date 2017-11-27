/***************************************************************************
                         qgslayoutpolylinewidget.cpp
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpolylinewidget.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"

QgsLayoutPolylineWidget::QgsLayoutPolylineWidget( QgsLayoutItemPolyline *polyline )
  : QgsLayoutItemBaseWidget( nullptr, polyline )
  , mPolyline( polyline )
{
  setupUi( this );
  setPanelTitle( tr( "Polyline properties" ) );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, polyline );
  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorWidget
  mItemPropertiesWidget->showBackgroundGroup( false );
  mItemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( mItemPropertiesWidget );

  mLineStyleButton->setSymbolType( QgsSymbol::Line );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutPolylineWidget::symbolChanged );

  if ( mPolyline )
  {
    connect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
    mLineStyleButton->registerExpressionContextGenerator( mPolyline );
  }
  setGuiElementValues();

#if 0 //TODO
  mShapeStyleButton->setLayer( atlasCoverageLayer() );
#endif
}

bool QgsLayoutPolylineWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutPolyline )
    return false;

  if ( mPolyline )
  {
    disconnect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
  }

  mPolyline = qobject_cast< QgsLayoutItemPolyline * >( item );
  mItemPropertiesWidget->setItem( mPolyline );

  if ( mPolyline )
  {
    connect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
    mLineStyleButton->registerExpressionContextGenerator( mPolyline );
  }

  setGuiElementValues();

  return true;
}


void QgsLayoutPolylineWidget::setGuiElementValues()
{
  if ( !mPolyline )
    return;

  whileBlocking( mLineStyleButton )->setSymbol( mPolyline->symbol()->clone() );
}

void QgsLayoutPolylineWidget::symbolChanged()
{
  if ( !mPolyline )
    return;

  mPolyline->layout()->undoStack()->beginCommand( mPolyline, tr( "Change Shape Style" ), QgsLayoutItem::UndoShapeStyle );
  mPolyline->setSymbol( mLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  mPolyline->layout()->undoStack()->endCommand();
}
