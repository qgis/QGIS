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
#include "moc_qgslayoutmarkerwidget.cpp"
#include "qgsstyle.h"
#include "qgslayoutitemmarker.h"
#include "qgslayout.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"
#include "qgslayoutitemmap.h"
#include "qgsmarkersymbol.h"
#include "qgslayoutreportcontext.h"

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

  mShapeStyleButton->setSymbolType( Qgis::SymbolType::Marker );

  blockAllSignals( false );

  connect( mMarker, &QgsLayoutObject::changed, this, &QgsLayoutMarkerWidget::setGuiElementValues );
  mShapeStyleButton->registerExpressionContextGenerator( mMarker );

  connect( mShapeStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutMarkerWidget::symbolChanged );

  connect( mRotationFromMapCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutMarkerWidget::rotationFromMapCheckBoxChanged );
  connect( mNorthOffsetSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMarkerWidget::northOffsetSpinBoxChanged );
  connect( mNorthTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMarkerWidget::northTypeComboBoxChanged );

  mNorthTypeComboBox->blockSignals( true );
  mNorthTypeComboBox->addItem( tr( "Grid North" ), QgsLayoutNorthArrowHandler::GridNorth );
  mNorthTypeComboBox->addItem( tr( "True North" ), QgsLayoutNorthArrowHandler::TrueNorth );
  mNorthTypeComboBox->blockSignals( false );
  mNorthOffsetSpinBox->setClearValue( 0.0 );

  mShapeStyleButton->setLayer( coverageLayer() );
  if ( mMarker->layout() )
  {
    connect( &mMarker->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mShapeStyleButton, &QgsSymbolButton::setLayer );
    mMapComboBox->setCurrentLayout( mMarker->layout() );
    mMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
    connect( mMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutMarkerWidget::mapChanged );
  }

  setGuiElementValues();
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

  mMarker = qobject_cast<QgsLayoutItemMarker *>( item );
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
  mMapComboBox->blockSignals( block );
  mRotationFromMapCheckBox->blockSignals( block );
  mNorthTypeComboBox->blockSignals( block );
  mNorthOffsetSpinBox->blockSignals( block );
}

void QgsLayoutMarkerWidget::setGuiElementValues()
{
  if ( !mMarker )
  {
    return;
  }

  blockAllSignals( true );

  mShapeStyleButton->setSymbol( mMarker->symbol()->clone() );

  mMapComboBox->setItem( mMarker->linkedMap() );
  if ( mMarker->linkedMap() )
  {
    mRotationFromMapCheckBox->setCheckState( Qt::Checked );
    mMapComboBox->setEnabled( true );
    mNorthTypeComboBox->setEnabled( true );
    mNorthOffsetSpinBox->setEnabled( true );
  }
  else
  {
    mRotationFromMapCheckBox->setCheckState( Qt::Unchecked );
    mMapComboBox->setEnabled( false );
    mNorthTypeComboBox->setEnabled( false );
    mNorthOffsetSpinBox->setEnabled( false );
  }
  mNorthTypeComboBox->setCurrentIndex( mNorthTypeComboBox->findData( mMarker->northMode() ) );
  mNorthOffsetSpinBox->setValue( mMarker->northOffset() );

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

void QgsLayoutMarkerWidget::rotationFromMapCheckBoxChanged( int state )
{
  if ( !mMarker )
  {
    return;
  }

  mMarker->beginCommand( tr( "Toggle Rotation Sync" ) );
  if ( state == Qt::Unchecked )
  {
    mMarker->setLinkedMap( nullptr );
    mMapComboBox->setEnabled( false );
    mNorthTypeComboBox->setEnabled( false );
    mNorthOffsetSpinBox->setEnabled( false );
  }
  else
  {
    QgsLayoutItemMap *map = qobject_cast<QgsLayoutItemMap *>( mMapComboBox->currentItem() );
    mMarker->setLinkedMap( map );
    mNorthTypeComboBox->setEnabled( true );
    mNorthOffsetSpinBox->setEnabled( true );
    mMapComboBox->setEnabled( true );
  }
  mMarker->endCommand();
}

void QgsLayoutMarkerWidget::mapChanged( QgsLayoutItem *item )
{
  if ( !mMarker )
  {
    return;
  }

  const QgsLayout *layout = mMarker->layout();
  if ( !layout )
  {
    return;
  }

  QgsLayoutItemMap *map = qobject_cast<QgsLayoutItemMap *>( item );
  if ( !map )
  {
    return;
  }

  mMarker->beginCommand( tr( "Change Rotation Map" ) );
  mMarker->setLinkedMap( map );
  mMarker->update();
  mMarker->endCommand();
}

void QgsLayoutMarkerWidget::northOffsetSpinBoxChanged( double d )
{
  mMarker->beginCommand( tr( "Change Marker North Offset" ), QgsLayoutItem::UndoPictureNorthOffset );
  mMarker->setNorthOffset( d );
  mMarker->endCommand();
  mMarker->update();
}

void QgsLayoutMarkerWidget::northTypeComboBoxChanged( int index )
{
  mMarker->beginCommand( tr( "Change Marker North Mode" ) );
  mMarker->setNorthMode( static_cast<QgsLayoutNorthArrowHandler::NorthMode>( mNorthTypeComboBox->itemData( index ).toInt() ) );
  mMarker->endCommand();
  mMarker->update();
}
