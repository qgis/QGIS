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
#include "qgspointxy.h"
#include "qgspropertyoverridebutton.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>
#include <QPen>


//QgsComposerItemBaseWidget

QgsComposerConfigObject::QgsComposerConfigObject( QWidget *parent, QgsComposerObject *composerObject )
  : QObject( parent )
  , mComposerObject( composerObject )
{
  connect( atlasComposition(), &QgsAtlasComposition::coverageLayerChanged,
           this, [ = ] { updateDataDefinedButtons(); } );
  connect( atlasComposition(), &QgsAtlasComposition::toggled, this, &QgsComposerConfigObject::updateDataDefinedButtons );
}

void QgsComposerConfigObject::updateDataDefinedProperty()
{
  //match data defined button to item's data defined property
  QgsPropertyOverrideButton *ddButton = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  if ( !ddButton )
  {
    return;
  }
  QgsComposerObject::DataDefinedProperty key = QgsComposerObject::NoProperty;

  if ( ddButton->propertyKey() >= 0 )
    key = static_cast< QgsComposerObject::DataDefinedProperty >( ddButton->propertyKey() );

  if ( key == QgsComposerObject::NoProperty )
  {
    return;
  }

  //set the data defined property and refresh the item
  mComposerObject->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
  mComposerObject->refreshDataDefinedProperty( key );
}

void QgsComposerConfigObject::updateDataDefinedButtons()
{
  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    button->setVectorLayer( atlasCoverageLayer() );
  }
}

void QgsComposerConfigObject::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsComposerObject::DataDefinedProperty key )
{
  button->blockSignals( true );
  button->init( key, mComposerObject->dataDefinedProperties(), QgsComposerObject::propertyDefinitions(), atlasCoverageLayer() );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsComposerConfigObject::updateDataDefinedProperty );
  button->registerExpressionContextGenerator( mComposerObject );
  button->blockSignals( false );
}

void QgsComposerConfigObject::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 )
    return;

  QgsComposerObject::DataDefinedProperty key = static_cast< QgsComposerObject::DataDefinedProperty >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mComposerObject->dataDefinedProperties().property( key ) );
}

QgsAtlasComposition *QgsComposerConfigObject::atlasComposition() const
{
  if ( !mComposerObject )
  {
    return nullptr;
  }

  QgsComposition *composition = mComposerObject->composition();

  if ( !composition )
  {
    return nullptr;
  }

  return &composition->atlasComposition();
}

QgsVectorLayer *QgsComposerConfigObject::atlasCoverageLayer() const
{
  QgsAtlasComposition *atlasMap = atlasComposition();

  if ( atlasMap && atlasMap->enabled() )
  {
    return atlasMap->coverageLayer();
  }

  return nullptr;
}


//QgsComposerItemWidget

void QgsComposerItemWidget::updateVariables()
{
  QgsExpressionContext context = mItem->createExpressionContext();
  mVariableEditor->setContext( &context );
  int editableIndex = context.indexOfScope( tr( "Composer Item" ) );
  if ( editableIndex >= 0 )
    mVariableEditor->setEditableScopeIndex( editableIndex );
}

QgsComposerItemWidget::QgsComposerItemWidget( QWidget *parent, QgsComposerItem *item )
  : QWidget( parent )
  , mItem( item )
  , mConfigObject( new QgsComposerConfigObject( this, item ) )
  , mFreezeXPosSpin( false )
  , mFreezeYPosSpin( false )
  , mFreezeWidthSpin( false )
  , mFreezeHeightSpin( false )
  , mFreezePageSpin( false )
{

  setupUi( this );
  connect( mFrameColorButton, &QgsColorButton::colorChanged, this, &QgsComposerItemWidget::mFrameColorButton_colorChanged );
  connect( mBackgroundColorButton, &QgsColorButton::clicked, this, &QgsComposerItemWidget::mBackgroundColorButton_clicked );
  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, &QgsComposerItemWidget::mBackgroundColorButton_colorChanged );
  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mStrokeWidthSpinBox_valueChanged );
  connect( mFrameGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerItemWidget::mFrameGroupBox_toggled );
  connect( mFrameJoinStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerItemWidget::mFrameJoinStyleCombo_currentIndexChanged );
  connect( mBackgroundGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerItemWidget::mBackgroundGroupBox_toggled );
  connect( mItemIdLineEdit, &QLineEdit::editingFinished, this, &QgsComposerItemWidget::mItemIdLineEdit_editingFinished );
  connect( mPageSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerItemWidget::mPageSpinBox_valueChanged );
  connect( mXPosSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mXPosSpin_valueChanged );
  connect( mYPosSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mYPosSpin_valueChanged );
  connect( mWidthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mWidthSpin_valueChanged );
  connect( mHeightSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mHeightSpin_valueChanged );
  connect( mUpperLeftCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mUpperLeftCheckBox_stateChanged );
  connect( mUpperMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mUpperMiddleCheckBox_stateChanged );
  connect( mUpperRightCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mUpperRightCheckBox_stateChanged );
  connect( mMiddleLeftCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mMiddleLeftCheckBox_stateChanged );
  connect( mMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mMiddleCheckBox_stateChanged );
  connect( mMiddleRightCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mMiddleRightCheckBox_stateChanged );
  connect( mLowerLeftCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mLowerLeftCheckBox_stateChanged );
  connect( mLowerMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mLowerMiddleCheckBox_stateChanged );
  connect( mLowerRightCheckBox, &QCheckBox::stateChanged, this, &QgsComposerItemWidget::mLowerRightCheckBox_stateChanged );
  connect( mBlendModeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerItemWidget::mBlendModeCombo_currentIndexChanged );
  connect( mItemRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerItemWidget::mItemRotationSpinBox_valueChanged );
  connect( mExcludeFromPrintsCheckBox, &QCheckBox::toggled, this, &QgsComposerItemWidget::mExcludeFromPrintsCheckBox_toggled );

  mItemRotationSpinBox->setClearValue( 0 );

  //make button exclusive
  QButtonGroup *buttonGroup = new QButtonGroup( this );
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

  initializeDataDefinedButtons();

  setValuesForGuiElements();
  connect( mItem->composition(), &QgsComposition::paperSizeChanged, this, &QgsComposerItemWidget::setValuesForGuiPositionElements );
  connect( mItem, &QgsComposerItem::sizeChanged, this, &QgsComposerItemWidget::setValuesForGuiPositionElements );
  connect( mItem, &QgsComposerObject::itemChanged, this, &QgsComposerItemWidget::setValuesForGuiNonPositionElements );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsComposerItemWidget::opacityChanged );

  updateVariables();
  connect( mVariableEditor, &QgsVariableEditorWidget::scopeChanged, this, &QgsComposerItemWidget::variablesChanged );
  // listen out for variable edits
  connect( QgsApplication::instance(), &QgsApplication::customVariablesChanged, this, &QgsComposerItemWidget::updateVariables );
  connect( QgsProject::instance(), &QgsProject::customVariablesChanged, this, &QgsComposerItemWidget::updateVariables );

  if ( mItem->composition() )
    connect( mItem->composition(), &QgsComposition::variablesChanged, this, &QgsComposerItemWidget::updateVariables );
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

void QgsComposerItemWidget::mFrameColorButton_colorChanged( const QColor &newFrameColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->beginCommand( tr( "Frame color changed" ), QgsComposerMergeCommand::ItemStrokeColor );
  mItem->setFrameStrokeColor( newFrameColor );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::mBackgroundColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }
}

void QgsComposerItemWidget::mBackgroundColorButton_colorChanged( const QColor &newBackgroundColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->beginCommand( tr( "Background color changed" ), QgsComposerMergeCommand::ItemBackgroundColor );
  mItem->setBackgroundColor( newBackgroundColor );

  //if the item is a composer map, we need to regenerate the map image
  //because it usually is cached
  if ( QgsComposerMap *cm = qobject_cast<QgsComposerMap *>( mItem ) )
  {
    cm->invalidateCache();
  }
  else
  {
    mItem->updateItem();
  }
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

void QgsComposerItemWidget::variablesChanged()
{
  QgsExpressionContextUtils::setComposerItemVariables( mItem, mVariableEditor->variablesInActiveScope() );
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

void QgsComposerItemWidget::mStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item stroke width" ), QgsComposerMergeCommand::ItemStrokeWidth );
  mItem->setFrameStrokeWidth( d );
  mItem->endCommand();
}

void QgsComposerItemWidget::mFrameJoinStyleCombo_currentIndexChanged( int index )
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

void QgsComposerItemWidget::mFrameGroupBox_toggled( bool state )
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

void QgsComposerItemWidget::mBackgroundGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item background toggled" ) );
  mItem->setBackgroundEnabled( state );

  //if the item is a composer map, we need to regenerate the map image
  //because it usually is cached
  if ( QgsComposerMap *cm = qobject_cast<QgsComposerMap *>( mItem ) )
  {
    cm->invalidateCache();
  }
  else
  {
    mItem->updateItem();
  }
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

  mStrokeWidthSpinBox->blockSignals( true );
  mFrameGroupBox->blockSignals( true );
  mBackgroundGroupBox->blockSignals( true );
  mItemIdLineEdit->blockSignals( true );
  mBlendModeCombo->blockSignals( true );
  mOpacityWidget->blockSignals( true );
  mFrameColorButton->blockSignals( true );
  mFrameJoinStyleCombo->blockSignals( true );
  mBackgroundColorButton->blockSignals( true );
  mItemRotationSpinBox->blockSignals( true );
  mExcludeFromPrintsCheckBox->blockSignals( true );

  mBackgroundColorButton->setColor( mItem->backgroundColor() );
  mFrameColorButton->setColor( mItem->frameStrokeColor() );
  mStrokeWidthSpinBox->setValue( mItem->frameStrokeWidth() );
  mFrameJoinStyleCombo->setPenJoinStyle( mItem->frameJoinStyle() );
  mItemIdLineEdit->setText( mItem->id() );
  mFrameGroupBox->setChecked( mItem->hasFrame() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );
  mBlendModeCombo->setBlendMode( mItem->blendMode() );
  mOpacityWidget->setOpacity( mItem->itemOpacity() );
  mItemRotationSpinBox->setValue( mItem->itemRotation( QgsComposerObject::OriginalValue ) );
  mExcludeFromPrintsCheckBox->setChecked( mItem->excludeFromExports( QgsComposerObject::OriginalValue ) );

  mBackgroundColorButton->blockSignals( false );
  mFrameColorButton->blockSignals( false );
  mFrameJoinStyleCombo->blockSignals( false );
  mStrokeWidthSpinBox->blockSignals( false );
  mFrameGroupBox->blockSignals( false );
  mBackgroundGroupBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mBlendModeCombo->blockSignals( false );
  mOpacityWidget->blockSignals( false );
  mItemRotationSpinBox->blockSignals( false );
  mExcludeFromPrintsCheckBox->blockSignals( false );
}

void QgsComposerItemWidget::initializeDataDefinedButtons()
{
  mConfigObject->initializeDataDefinedButton( mXPositionDDBtn, QgsComposerObject::PositionX );
  mConfigObject->initializeDataDefinedButton( mYPositionDDBtn, QgsComposerObject::PositionY );
  mConfigObject->initializeDataDefinedButton( mWidthDDBtn, QgsComposerObject::ItemWidth );
  mConfigObject->initializeDataDefinedButton( mHeightDDBtn, QgsComposerObject::ItemHeight );
  mConfigObject->initializeDataDefinedButton( mItemRotationDDBtn, QgsComposerObject::ItemRotation );
  mConfigObject->initializeDataDefinedButton( mOpacityDDBtn, QgsComposerObject::Opacity );
  mConfigObject->initializeDataDefinedButton( mBlendModeDDBtn, QgsComposerObject::BlendMode );
  mConfigObject->initializeDataDefinedButton( mExcludePrintsDDBtn, QgsComposerObject::ExcludeFromExports );
  mConfigObject->initializeDataDefinedButton( mItemFrameColorDDBtn, QgsComposerObject::FrameColor );
  mConfigObject->initializeDataDefinedButton( mItemBackgroundColorDDBtn, QgsComposerObject::BackgroundColor );
}

void QgsComposerItemWidget::populateDataDefinedButtons()
{
  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    mConfigObject->updateDataDefinedButton( button );
  }
}

void QgsComposerItemWidget::setValuesForGuiElements()
{
  if ( !mItem )
  {
    return;
  }

  mBackgroundColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setContext( QStringLiteral( "composer" ) );
  mFrameColorButton->setColorDialogTitle( tr( "Select Frame Color" ) );
  mFrameColorButton->setAllowOpacity( true );
  mFrameColorButton->setContext( QStringLiteral( "composer" ) );

  setValuesForGuiPositionElements();
  setValuesForGuiNonPositionElements();
  populateDataDefinedButtons();
}

void QgsComposerItemWidget::mBlendModeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item blend mode changed" ) );
    mItem->setBlendMode( mBlendModeCombo->blendMode() );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::opacityChanged( double value )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item opacity changed" ), QgsComposerMergeCommand::ItemOpacity );
    mItem->setItemOpacity( value );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::mItemIdLineEdit_editingFinished()
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item id changed" ), QgsComposerMergeCommand::ComposerLabelSetId );
    mItem->setId( mItemIdLineEdit->text() );
    mItemIdLineEdit->setText( mItem->id() );
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::mPageSpinBox_valueChanged( int )
{
  mFreezePageSpin = true;
  changeItemPosition();
  mFreezePageSpin = false;
}

void QgsComposerItemWidget::mXPosSpin_valueChanged( double )
{
  mFreezeXPosSpin = true;
  changeItemPosition();
  mFreezeXPosSpin = false;
}

void QgsComposerItemWidget::mYPosSpin_valueChanged( double )
{
  mFreezeYPosSpin = true;
  changeItemPosition();
  mFreezeYPosSpin = false;
}

void QgsComposerItemWidget::mWidthSpin_valueChanged( double )
{
  mFreezeWidthSpin = true;
  changeItemPosition();
  mFreezeWidthSpin = false;
}

void QgsComposerItemWidget::mHeightSpin_valueChanged( double )
{
  mFreezeHeightSpin = true;
  changeItemPosition();
  mFreezeHeightSpin = false;
}

void QgsComposerItemWidget::mUpperLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
    mItem->setItemPosition( mItem->pos().x(), mItem->pos().y(), QgsComposerItem::UpperLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsComposerItemWidget::mUpperMiddleCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mUpperRightCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mMiddleLeftCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mMiddleCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mMiddleRightCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mLowerLeftCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mLowerMiddleCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mLowerRightCheckBox_stateChanged( int state )
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

void QgsComposerItemWidget::mItemRotationSpinBox_valueChanged( double val )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item rotation changed" ), QgsComposerMergeCommand::ItemRotation );
    mItem->setItemRotation( val, true );
    mItem->update();
    mItem->endCommand();
  }
}

void QgsComposerItemWidget::mExcludeFromPrintsCheckBox_toggled( bool checked )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Exclude from exports changed" ) );
    mItem->setExcludeFromExports( checked );
    mItem->endCommand();
  }
}

QgsComposerItemBaseWidget::QgsComposerItemBaseWidget( QWidget *parent, QgsComposerObject *composerObject )
  : QgsPanelWidget( parent )
  , mConfigObject( new QgsComposerConfigObject( this, composerObject ) )
{

}

void QgsComposerItemBaseWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsComposerObject::DataDefinedProperty property )
{
  mConfigObject->initializeDataDefinedButton( button, property );
}

void QgsComposerItemBaseWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  mConfigObject->updateDataDefinedButton( button );
}

QgsVectorLayer *QgsComposerItemBaseWidget::atlasCoverageLayer() const
{
  return mConfigObject->atlasCoverageLayer();
}

QgsAtlasComposition *QgsComposerItemBaseWidget::atlasComposition() const
{
  return mConfigObject->atlasComposition();
}
