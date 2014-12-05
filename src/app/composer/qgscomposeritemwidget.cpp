/***************************************************************************
                         qgscomposeritemwidget.cpp
                         -------------------------
    begin                : August 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposeritemwidget.h"
#include "qgscomposeritem.h"
#include "qgscomposermap.h"
#include "qgsatlascomposition.h"
#include "qgscomposition.h"
#include "qgspoint.h"
#include "qgsdatadefinedbutton.h"
#include <QColorDialog>
#include <QPen>


//QgsComposerItemBaseWidget

QgsComposerItemBaseWidget::QgsComposerItemBaseWidget( QWidget* parent, QgsComposerObject *composerObject ): QWidget( parent ), mComposerObject( composerObject )
{

}

QgsComposerItemBaseWidget::~QgsComposerItemBaseWidget()
{

}

void QgsComposerItemBaseWidget::updateDataDefinedProperty()
{
  //match data defined button to item's data defined property
  QgsDataDefinedButton* ddButton = dynamic_cast<QgsDataDefinedButton*>( sender() );
  if ( !ddButton )
  {
    return;
  }
  QgsComposerObject::DataDefinedProperty property = ddPropertyForWidget( ddButton );
  if ( property == QgsComposerObject::NoProperty )
  {
    return;
  }

  //set the data defined property and refresh the item
  setDataDefinedProperty( ddButton, property );
  mComposerObject->refreshDataDefinedProperty( property );
}

void QgsComposerItemBaseWidget::setDataDefinedProperty( const QgsDataDefinedButton *ddBtn, QgsComposerObject::DataDefinedProperty p )
{
  if ( !mComposerObject )
  {
    return;
  }

  const QMap< QString, QString >& map = ddBtn->definedProperty();
  mComposerObject->setDataDefinedProperty( p, map.value( "active" ).toInt(), map.value( "useexpr" ).toInt(), map.value( "expression" ), map.value( "field" ) );
}

QgsComposerObject::DataDefinedProperty QgsComposerItemBaseWidget::ddPropertyForWidget( QgsDataDefinedButton *widget )
{
  Q_UNUSED( widget );

  //base implementation, return no property
  return QgsComposerObject::NoProperty;
}

QgsAtlasComposition* QgsComposerItemBaseWidget::atlasComposition() const
{
  if ( !mComposerObject )
  {
    return 0;
  }

  QgsComposition* composition = mComposerObject->composition();

  if ( !composition )
  {
    return 0;
  }

  return &composition->atlasComposition();
}

QgsVectorLayer* QgsComposerItemBaseWidget::atlasCoverageLayer() const
{
  QgsAtlasComposition* atlasMap = atlasComposition();

  if ( atlasMap && atlasMap->enabled() )
  {
    return atlasMap->coverageLayer();
  }

  return 0;
}


//QgsComposerItemWidget

QgsComposerItemWidget::QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item )
    : QgsComposerItemBaseWidget( parent, item )
    , mItem( item )
    , mFreezeXPosSpin( false )
    , mFreezeYPosSpin( false )
    , mFreezeWidthSpin( false )
    , mFreezeHeightSpin( false )
    , mFreezePageSpin( false )
{

  setupUi( this );

  //make button exclusive
  QButtonGroup* buttonGroup = new QButtonGroup( this );
  buttonGroup->addButton( mUpperLeftCheckBox );
  buttonGroup->addButton( mUpperMiddleCheckBox );
  buttonGroup->addButton( mUpperRightCheckBox );
  buttonGroup->addButton( mMiddleLeftCheckBox );
  buttonGroup->addButton( mMiddleCheckBox );
  buttonGroup->addButton( mMiddleRightCheckBox );
  buttonGroup->addButton( mLowerLeftCheckBox );
  buttonGroup->addButton( mLowerMiddleCheckBox );
  buttonGroup->addButton( mLowerRightCheckBox );
  buttonGroup->setExclusive( true );

  setValuesForGuiElements();
  connect( mItem->composition(), SIGNAL( paperSizeChanged() ), this, SLOT( setValuesForGuiPositionElements() ) );
  connect( mItem, SIGNAL( sizeChanged() ), this, SLOT( setValuesForGuiPositionElements() ) );
  connect( mItem, SIGNAL( itemChanged() ), this, SLOT( setValuesForGuiNonPositionElements() ) );

  connect( mTransparencySlider, SIGNAL( valueChanged( int ) ), mTransparencySpnBx, SLOT( setValue( int ) ) );

  //connect atlas signals to data defined buttons
  QgsAtlasComposition* atlas = atlasComposition();
  if ( atlas )
  {
    //repopulate data defined buttons if atlas layer changes
    connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
             this, SLOT( populateDataDefinedButtons() ) );
    connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
  }

  //connect data defined buttons
  connect( mXPositionDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mXPositionDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mYPositionDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mYPositionDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mWidthDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mWidthDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mHeightDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mHeightDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mItemRotationDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mItemRotationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mTransparencyDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mTransparencyDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mBlendModeDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mBlendModeDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mExcludePrintsDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mExcludePrintsDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
}

QgsComposerItemWidget::QgsComposerItemWidget(): QgsComposerItemBaseWidget( 0, 0 )
{

}

QgsComposerItemWidget::~QgsComposerItemWidget()
{

}

void QgsComposerItemWidget::showBackgroundGroup( bool showGroup )
{
  mBackgroundGroupBox->setVisible( showGroup );
}

void QgsComposerItemWidget::showFrameGroup( bool showGroup )
{
  mFrameGroupBox->setVisible( showGroup );
}

//slots

void QgsComposerItemWidget::on_mFrameColorButton_colorChanged( const QColor& newFrameColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->beginCommand( tr( "Frame color changed" ) );
  mItem->setFrameOutlineColor( newFrameColor );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mBackgroundColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }
}

void QgsComposerItemWidget::on_mBackgroundColorButton_colorChanged( const QColor& newBackgroundColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->beginCommand( tr( "Background color changed" ) );
  mItem->setBackgroundColor( newBackgroundColor );

  //if the item is a composer map, we need to regenerate the map image
  //because it usually is cached
  QgsComposerMap* cm = dynamic_cast<QgsComposerMap *>( mItem );
  if ( cm )
  {
    cm->cache();
  }
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::changeItemPosition()
{
  mItem->beginCommand( tr( "Item position changed" ) );

  double x = mXPosSpin->value();
  double y = mYPosSpin->value();
  double width = mWidthSpin->value();
  double height = mHeightSpin->value();

  mItem->setItemPosition( x, y, width, height, positionMode(), false, mPageSpinBox->value() );

  mItem->update();
  mItem->endCommand();
}

QgsComposerItem::ItemPositionMode QgsComposerItemWidget::positionMode() const
{
  if ( mUpperLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperLeft;
  }
  else if ( mUpperMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperMiddle;
  }
  else if ( mUpperRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperRight;
  }
  else if ( mMiddleLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::MiddleLeft;
  }
  else if ( mMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::Middle;
  }
  else if ( mMiddleRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::MiddleRight;
  }
  else if ( mLowerLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerLeft;
  }
  else if ( mLowerMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerMiddle;
  }
  else if ( mLowerRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerRight;
  }
  return QgsComposerItem::UpperLeft;
}

void QgsComposerItemWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item outline width" ), QgsComposerMergeCommand::ItemOutlineWidth );
  mItem->setFrameOutlineWidth( d );
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mFrameJoinStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item frame join style" ) );
  mItem->setFrameJoinStyle( mFrameJoinStyleCombo->penJoinStyle() );
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mFrameGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item frame toggled" ) );
  mItem->setFrameEnabled( state );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mBackgroundGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item background toggled" ) );
  mItem->setBackgroundEnabled( state );

  //if the item is a composer map, we need to regenerate the map image
  //because it usually is cached
  QgsComposerMap* cm = dynamic_cast<QgsComposerMap *>( mItem );
  if ( cm )
  {
    cm->cache();
  }

  mItem->update();
  mItem->endCommand();
}


void QgsComposerItemWidget::setValuesForGuiPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  mXPosSpin->blockSignals( true );
  mYPosSpin->blockSignals( true );
  mWidthSpin->blockSignals( true );
  mHeightSpin->blockSignals( true );
  mUpperLeftCheckBox->blockSignals( true );
  mUpperMiddleCheckBox->blockSignals( true );
  mUpperRightCheckBox->blockSignals( true );
  mMiddleLeftCheckBox->blockSignals( true );
  mMiddleCheckBox->blockSignals( true );
  mMiddleRightCheckBox->blockSignals( true );
  mLowerLeftCheckBox->blockSignals( true );
  mLowerMiddleCheckBox->blockSignals( true );
  mLowerRightCheckBox->blockSignals( true );
  mPageSpinBox->blockSignals( true );

  QPointF pos = mItem->pagePos();

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperLeft )
  {
    mUpperLeftCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperMiddle )
  {
    mUpperMiddleCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperRight )
  {
    mUpperRightCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::MiddleLeft )
  {
    mMiddleLeftCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::Middle )
  {
    mMiddleCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::MiddleRight )
  {
    mMiddleRightCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerLeft )
  {
    mLowerLeftCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerMiddle )
  {
    mLowerMiddleCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerRight )
  {
    mLowerRightCheckBox->setChecked( true );
    if ( !mFreezeXPosSpin )
      mXPosSpin->setValue( pos.x() + mItem->rect().width() );
    if ( !mFreezeYPosSpin )
      mYPosSpin->setValue( pos.y() + mItem->rect().height() );
  }

  if ( !mFreezeWidthSpin )
    mWidthSpin->setValue( mItem->rect().width() );
  if ( !mFreezeHeightSpin )
    mHeightSpin->setValue( mItem->rect().height() );
  if ( !mFreezePageSpin )
    mPageSpinBox->setValue( mItem->page() );

  mXPosSpin->blockSignals( false );
  mYPosSpin->blockSignals( false );
  mWidthSpin->blockSignals( false );
  mHeightSpin->blockSignals( false );
  mUpperLeftCheckBox->blockSignals( false );
  mUpperMiddleCheckBox->blockSignals( false );
  mUpperRightCheckBox->blockSignals( false );
  mMiddleLeftCheckBox->blockSignals( false );
  mMiddleCheckBox->blockSignals( false );
  mMiddleRightCheckBox->blockSignals( false );
  mLowerLeftCheckBox->blockSignals( false );
  mLowerMiddleCheckBox->blockSignals( false );
  mLowerRightCheckBox->blockSignals( false );
  mPageSpinBox->blockSignals( false );
}

void QgsComposerItemWidget::setValuesForGuiNonPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  mOutlineWidthSpinBox->blockSignals( true );
  mFrameGroupBox->blockSignals( true );
  mBackgroundGroupBox->blockSignals( true );
  mItemIdLineEdit->blockSignals( true );
  mBlendModeCombo->blockSignals( true );
  mTransparencySlider->blockSignals( true );
  mTransparencySpnBx->blockSignals( true );
  mFrameColorButton->blockSignals( true );
  mFrameJoinStyleCombo->blockSignals( true );
  mBackgroundColorButton->blockSignals( true );
  mItemRotationSpinBox->blockSignals( true );
  mExcludeFromPrintsCheckBox->blockSignals( true );

  mBackgroundColorButton->setColor( mItem->backgroundColor() );
  mFrameColorButton->setColor( mItem->frameOutlineColor() );
  mOutlineWidthSpinBox->setValue( mItem->frameOutlineWidth() );
  mFrameJoinStyleCombo->setPenJoinStyle( mItem->frameJoinStyle() );
  mItemIdLineEdit->setText( mItem->id() );
  mFrameGroupBox->setChecked( mItem->hasFrame() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );
  mBlendModeCombo->setBlendMode( mItem->blendMode() );
  mTransparencySlider->setValue( mItem->transparency() );
  mTransparencySpnBx->setValue( mItem->transparency() );
  mItemRotationSpinBox->setValue( mItem->itemRotation( QgsComposerObject::OriginalValue ) );
  mExcludeFromPrintsCheckBox->setChecked( mItem->excludeFromExports( QgsComposerObject::OriginalValue ) );

  mBackgroundColorButton->blockSignals( false );
  mFrameColorButton->blockSignals( false );
  mFrameJoinStyleCombo->blockSignals( false );
  mOutlineWidthSpinBox->blockSignals( false );
  mFrameGroupBox->blockSignals( false );
  mBackgroundGroupBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mBlendModeCombo->blockSignals( false );
  mTransparencySlider->blockSignals( false );
  mTransparencySpnBx->blockSignals( false );
  mItemRotationSpinBox->blockSignals( false );
  mExcludeFromPrintsCheckBox->blockSignals( false );
}

void QgsComposerItemWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  //block signals from data defined buttons
  mXPositionDDBtn->blockSignals( true );
  mYPositionDDBtn->blockSignals( true );
  mWidthDDBtn->blockSignals( true );
  mHeightDDBtn->blockSignals( true );
  mItemRotationDDBtn->blockSignals( true );
  mTransparencyDDBtn->blockSignals( true );
  mBlendModeDDBtn->blockSignals( true );
  mExcludePrintsDDBtn->blockSignals( true );

  //initialise buttons to use atlas coverage layer
  mXPositionDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::PositionX ),
                         QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mYPositionDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::PositionY ),
                         QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mWidthDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::ItemWidth ),
                     QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mHeightDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::ItemHeight ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mItemRotationDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::ItemRotation ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::double180RotDesc() );
  mTransparencyDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::Transparency ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intTranspDesc() );
  mBlendModeDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::BlendMode ),
                         QgsDataDefinedButton::String, QgsDataDefinedButton::blendModesDesc() );
  mExcludePrintsDDBtn->init( vl, mItem->dataDefinedProperty( QgsComposerObject::ExcludeFromExports ),
                             QgsDataDefinedButton::String, QgsDataDefinedButton::boolDesc() );

  //unblock signals from data defined buttons
  mXPositionDDBtn->blockSignals( false );
  mYPositionDDBtn->blockSignals( false );
  mWidthDDBtn->blockSignals( false );
  mHeightDDBtn->blockSignals( false );
  mItemRotationDDBtn->blockSignals( false );
  mTransparencyDDBtn->blockSignals( false );
  mBlendModeDDBtn->blockSignals( false );
  mExcludePrintsDDBtn->blockSignals( false );
}

QgsComposerObject::DataDefinedProperty QgsComposerItemWidget::ddPropertyForWidget( QgsDataDefinedButton* widget )
{
  if ( widget == mXPositionDDBtn )
  {
    return QgsComposerObject::PositionX;
  }
  else if ( widget == mYPositionDDBtn )
  {
    return QgsComposerObject::PositionY;
  }
  else if ( widget == mWidthDDBtn )
  {
    return QgsComposerObject::ItemWidth;
  }
  else if ( widget == mHeightDDBtn )
  {
    return QgsComposerObject::ItemHeight;
  }
  else if ( widget == mItemRotationDDBtn )
  {
    return QgsComposerObject::ItemRotation;
  }
  else if ( widget == mTransparencyDDBtn )
  {
    return QgsComposerObject::Transparency;
  }
  else if ( widget == mBlendModeDDBtn )
  {
    return QgsComposerObject::BlendMode;
  }
  else if ( widget == mExcludePrintsDDBtn )
  {
    return QgsComposerObject::ExcludeFromExports;
  }

  return QgsComposerObject::NoProperty;
}

void QgsComposerItemWidget::setValuesForGuiElements()
{
  if ( !mItem )
  {
    return;
  }

  mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
  mBackgroundColorButton->setAllowAlpha( true );
  mBackgroundColorButton->setContext( "composer" );
  mFrameColorButton->setColorDialogTitle( tr( "Select frame color" ) );
  mFrameColorButton->setAllowAlpha( true );
  mFrameColorButton->setContext( "composer" );

  setValuesForGuiPositionElements();
  setValuesForGuiNonPositionElements();
  populateDataDefinedButtons();
}

void QgsComposerItemWidget::on_mBlendModeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item blend mode changed" ) );
    mItem->setBlendMode( mBlendModeCombo->blendMode() );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::on_mTransparencySpnBx_valueChanged( int value )
{
  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value );
  mTransparencySlider->blockSignals( false );
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item transparency changed" ), QgsComposerMergeCommand::ItemTransparency );
    mItem->setTransparency( value );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::on_mItemIdLineEdit_editingFinished()
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item id changed" ), QgsComposerMergeCommand::ComposerLabelSetId );
    mItem->setId( mItemIdLineEdit->text() );
    mItemIdLineEdit->setText( mItem->id() );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::on_mPageSpinBox_valueChanged( int )
{
  mFreezePageSpin = true;
  changeItemPosition();
  mFreezePageSpin = false;
}

void QgsComposerItemWidget::on_mXPosSpin_valueChanged( double )
{
  mFreezeXPosSpin = true;
  changeItemPosition();
  mFreezeXPosSpin = false;
}

void QgsComposerItemWidget::on_mYPosSpin_valueChanged( double )
{
  mFreezeYPosSpin = true;
  changeItemPosition();
  mFreezeYPosSpin = false;
}

void QgsComposerItemWidget::on_mWidthSpin_valueChanged( double )
{
  mFreezeWidthSpin = true;
  changeItemPosition();
  mFreezeWidthSpin = false;
}

void QgsComposerItemWidget::on_mHeightSpin_valueChanged( double )
{
  mFreezeHeightSpin = true;
  changeItemPosition();
  mFreezeHeightSpin = false;
}

void QgsComposerItemWidget::on_mUpperLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x(), mItem->pos().y(), QgsComposerItem::UpperLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mUpperMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y(), QgsComposerItem::UpperMiddle );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mUpperRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y(), QgsComposerItem::UpperRight );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mMiddleLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x(),
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::MiddleLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::Middle );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mMiddleRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::MiddleRight );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mLowerLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x(),
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mLowerMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerMiddle );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mLowerRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerRight );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::on_mItemRotationSpinBox_valueChanged( double val )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item rotation changed" ), QgsComposerMergeCommand::ItemRotation );
    mItem->setItemRotation( val, true );
    mItem->update();
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::on_mExcludeFromPrintsCheckBox_toggled( bool checked )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Exclude from exports changed" ) );
    mItem->setExcludeFromExports( checked );
    mItem->endCommand();
  }
}
