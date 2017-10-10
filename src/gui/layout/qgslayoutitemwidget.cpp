/***************************************************************************
                             qgslayoutitemwidget.cpp
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgslayout.h"
#include "qgsproject.h"

//
// QgsLayoutConfigObject
//

QgsLayoutConfigObject::QgsLayoutConfigObject( QWidget *parent, QgsLayoutObject *layoutObject )
  : QObject( parent )
  , mLayoutObject( layoutObject )
{
#if 0 //TODO
  connect( atlasComposition(), &QgsAtlasComposition::coverageLayerChanged,
           this, [ = ] { updateDataDefinedButtons(); } );
  connect( atlasComposition(), &QgsAtlasComposition::toggled, this, &QgsComposerConfigObject::updateDataDefinedButtons );
#endif
}

void QgsLayoutConfigObject::updateDataDefinedProperty()
{
  //match data defined button to item's data defined property
  QgsPropertyOverrideButton *ddButton = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  if ( !ddButton )
  {
    return;
  }
  QgsLayoutObject::DataDefinedProperty key = QgsLayoutObject::NoProperty;

  if ( ddButton->propertyKey() >= 0 )
    key = static_cast< QgsLayoutObject::DataDefinedProperty >( ddButton->propertyKey() );

  if ( key == QgsLayoutObject::NoProperty )
  {
    return;
  }

  //set the data defined property and refresh the item
  if ( mLayoutObject )
  {
    mLayoutObject->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
    mLayoutObject->refresh();
  }
}

void QgsLayoutConfigObject::updateDataDefinedButtons()
{
#if 0 //TODO
  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    button->setVectorLayer( atlasCoverageLayer() );
  }
#endif
}

void QgsLayoutConfigObject::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key )
{
  button->blockSignals( true );
  button->init( key, mLayoutObject->dataDefinedProperties(), QgsLayoutObject::propertyDefinitions(), coverageLayer() );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLayoutConfigObject::updateDataDefinedProperty );
  button->registerExpressionContextGenerator( mLayoutObject );
  button->blockSignals( false );
}

void QgsLayoutConfigObject::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 || !mLayoutObject )
    return;

  QgsLayoutObject::DataDefinedProperty key = static_cast< QgsLayoutObject::DataDefinedProperty >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mLayoutObject->dataDefinedProperties().property( key ) );
}

#if 0 // TODO
QgsAtlasComposition *QgsLayoutConfigObject::atlasComposition() const
{
  if ( !mLayoutObject )
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
#endif

QgsVectorLayer *QgsLayoutConfigObject::coverageLayer() const
{
  if ( !mLayoutObject )
    return nullptr;

  QgsLayout *layout = mLayoutObject->layout();
  if ( !layout )
    return nullptr;

  return layout->context().layer();
}


//
// QgsLayoutItemBaseWidget
//

QgsLayoutItemBaseWidget::QgsLayoutItemBaseWidget( QWidget *parent, QgsLayoutObject *layoutObject )
  : QgsPanelWidget( parent )
  , mConfigObject( new QgsLayoutConfigObject( this, layoutObject ) )
{

}

void QgsLayoutItemBaseWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty property )
{
  mConfigObject->initializeDataDefinedButton( button, property );
}

void QgsLayoutItemBaseWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  mConfigObject->updateDataDefinedButton( button );
}

QgsVectorLayer *QgsLayoutItemBaseWidget::coverageLayer() const
{
  return mConfigObject->coverageLayer();
}

#if 0 //TODO
QgsAtlasComposition *QgsLayoutItemBaseWidget::atlasComposition() const
{
  return mConfigObject->atlasComposition();
}
#endif


//


//QgsLayoutItemPropertiesWidget

void QgsLayoutItemPropertiesWidget::updateVariables()
{
  QgsExpressionContext context = mItem->createExpressionContext();
  mVariableEditor->setContext( &context );
  int editableIndex = context.indexOfScope( tr( "Composer Item" ) );
  if ( editableIndex >= 0 )
    mVariableEditor->setEditableScopeIndex( editableIndex );
}

QgsLayoutItemPropertiesWidget::QgsLayoutItemPropertiesWidget( QWidget *parent, QgsLayoutItem *item )
  : QWidget( parent )
  , mItem( item )
  , mConfigObject( new QgsLayoutConfigObject( this, item ) )
  , mFreezeXPosSpin( false )
  , mFreezeYPosSpin( false )
  , mFreezeWidthSpin( false )
  , mFreezeHeightSpin( false )
  , mFreezePageSpin( false )
{

  setupUi( this );
  connect( mFrameColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutItemPropertiesWidget::mFrameColorButton_colorChanged );
  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutItemPropertiesWidget::mBackgroundColorButton_colorChanged );
  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mStrokeWidthSpinBox_valueChanged );
  connect( mStrokeUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutItemPropertiesWidget::strokeUnitChanged );
  connect( mFrameGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutItemPropertiesWidget::mFrameGroupBox_toggled );
  connect( mFrameJoinStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutItemPropertiesWidget::mFrameJoinStyleCombo_currentIndexChanged );
  connect( mBackgroundGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutItemPropertiesWidget::mBackgroundGroupBox_toggled );
  connect( mItemIdLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutItemPropertiesWidget::mItemIdLineEdit_editingFinished );
  connect( mPageSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mPageSpinBox_valueChanged );
  connect( mXPosSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mXPosSpin_valueChanged );
  connect( mYPosSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mYPosSpin_valueChanged );
  connect( mWidthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mWidthSpin_valueChanged );
  connect( mHeightSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mHeightSpin_valueChanged );
  connect( mUpperLeftCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mUpperLeftCheckBox_stateChanged );
  connect( mUpperMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mUpperMiddleCheckBox_stateChanged );
  connect( mUpperRightCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mUpperRightCheckBox_stateChanged );
  connect( mMiddleLeftCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mMiddleLeftCheckBox_stateChanged );
  connect( mMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mMiddleCheckBox_stateChanged );
  connect( mMiddleRightCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mMiddleRightCheckBox_stateChanged );
  connect( mLowerLeftCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mLowerLeftCheckBox_stateChanged );
  connect( mLowerMiddleCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mLowerMiddleCheckBox_stateChanged );
  connect( mLowerRightCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutItemPropertiesWidget::mLowerRightCheckBox_stateChanged );
  connect( mBlendModeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutItemPropertiesWidget::mBlendModeCombo_currentIndexChanged );
  connect( mItemRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mItemRotationSpinBox_valueChanged );
  connect( mExcludeFromPrintsCheckBox, &QCheckBox::toggled, this, &QgsLayoutItemPropertiesWidget::mExcludeFromPrintsCheckBox_toggled );

  mItemRotationSpinBox->setClearValue( 0 );
  mStrokeUnitsComboBox->linkToWidget( mStrokeWidthSpinBox );
  mStrokeUnitsComboBox->setConverter( &mItem->layout()->context().measurementConverter() );


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

#if 0 //TODO
  connect( mItem->composition(), &QgsComposition::paperSizeChanged, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements );
  connect( mItem, &QgsComposerItem::sizeChanged, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements );
#endif

  connect( mItem, &QgsLayoutObject::changed, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiNonPositionElements );

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsLayoutItemPropertiesWidget::opacityChanged );

  updateVariables();
  connect( mVariableEditor, &QgsVariableEditorWidget::scopeChanged, this, &QgsLayoutItemPropertiesWidget::variablesChanged );
  // listen out for variable edits
  connect( QgsApplication::instance(), &QgsApplication::customVariablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
  connect( mItem->layout()->project(), &QgsProject::customVariablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );

  if ( mItem->layout() )
    connect( mItem->layout(), &QgsLayout::variablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
}

void QgsLayoutItemPropertiesWidget::showBackgroundGroup( bool showGroup )
{
  mBackgroundGroupBox->setVisible( showGroup );
}

void QgsLayoutItemPropertiesWidget::showFrameGroup( bool showGroup )
{
  mFrameGroupBox->setVisible( showGroup );
}

//slots

void QgsLayoutItemPropertiesWidget::mFrameColorButton_colorChanged( const QColor &newFrameColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Frame Color" ), QgsLayoutItem::UndoStrokeColor );
  mItem->setFrameStrokeColor( newFrameColor );
  mItem->layout()->undoStack()->endCommand();
  mItem->update();
}

void QgsLayoutItemPropertiesWidget::mBackgroundColorButton_colorChanged( const QColor &newBackgroundColor )
{
  if ( !mItem )
  {
    return;
  }
  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Background Color" ), QgsLayoutItem::UndoBackgroundColor );
  mItem->setBackgroundColor( newBackgroundColor );

#if 0 //TODO
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
#endif
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::changeItemPosition()
{
  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Move Item" ) );

  double x = mXPosSpin->value();
  double y = mYPosSpin->value();
  double width = mWidthSpin->value();
  double height = mHeightSpin->value();

#if 0 //TODO
  mItem->setItemPosition( x, y, width, height, positionMode(), false, mPageSpinBox->value() );
#endif
  mItem->update();
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::variablesChanged()
{
#if 0 //TODO
  QgsExpressionContextUtils::setComposerItemVariables( mItem, mVariableEditor->variablesInActiveScope() );
#endif
}

QgsLayoutItem::ReferencePoint QgsLayoutItemPropertiesWidget::positionMode() const
{
  if ( mUpperLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperLeft;
  }
  else if ( mUpperMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperMiddle;
  }
  else if ( mUpperRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperRight;
  }
  else if ( mMiddleLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::MiddleLeft;
  }
  else if ( mMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::Middle;
  }
  else if ( mMiddleRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::MiddleRight;
  }
  else if ( mLowerLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerLeft;
  }
  else if ( mLowerMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerMiddle;
  }
  else if ( mLowerRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerRight;
  }
  return QgsLayoutItem::UpperLeft;
}

void QgsLayoutItemPropertiesWidget::mStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mItem )
  {
    return;
  }

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Frame Stroke Width" ), QgsLayoutItem::UndoStrokeWidth );
  mItem->setFrameStrokeWidth( QgsLayoutMeasurement( d, mStrokeUnitsComboBox->unit() ) );
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::strokeUnitChanged( QgsUnitTypes::LayoutUnit unit )
{
  if ( !mItem )
  {
    return;
  }

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Frame Stroke Width" ), QgsLayoutItem::UndoStrokeWidth );
  mItem->setFrameStrokeWidth( QgsLayoutMeasurement( mStrokeWidthSpinBox->value(), unit ) );
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::mFrameJoinStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mItem )
  {
    return;
  }

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Frame Join Style" ) );
  mItem->setFrameJoinStyle( mFrameJoinStyleCombo->penJoinStyle() );
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::mFrameGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->layout()->undoStack()->beginCommand( mItem, state ? tr( "Enable Frame" ) : tr( "Disable Frame" ) );
  mItem->setFrameEnabled( state );
  mItem->update();
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::mBackgroundGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->layout()->undoStack()->beginCommand( mItem, state ? tr( "Enable Background" ) : tr( "Disable Background" ) );
  mItem->setBackgroundEnabled( state );

#if 0 //TODO
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
#endif
  mItem->layout()->undoStack()->endCommand();
}


void QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements()
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

  QPointF pos; //TODO = mItem->pagePos();

  switch ( mItem->referencePoint() )
  {
    case QgsLayoutItem::UpperLeft:
    {
      mUpperLeftCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() );
      break;
    }

    case QgsLayoutItem::UpperMiddle:
    {
      mUpperMiddleCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() );
      break;
    }

    case QgsLayoutItem::UpperRight:
    {
      mUpperRightCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() );
      break;
    }

    case QgsLayoutItem::MiddleLeft:
    {
      mMiddleLeftCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
      break;
    }

    case QgsLayoutItem::Middle:
    {
      mMiddleCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
      break;
    }

    case QgsLayoutItem::MiddleRight:
    {
      mMiddleRightCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() / 2.0 );
      break;
    }

    case QgsLayoutItem::LowerLeft:
    {
      mLowerLeftCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() );
      break;
    }

    case QgsLayoutItem::LowerMiddle:
    {
      mLowerMiddleCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() / 2.0 );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() );
      break;
    }

    case QgsLayoutItem::LowerRight:
    {
      mLowerRightCheckBox->setChecked( true );
      if ( !mFreezeXPosSpin )
        mXPosSpin->setValue( pos.x() + mItem->rect().width() );
      if ( !mFreezeYPosSpin )
        mYPosSpin->setValue( pos.y() + mItem->rect().height() );
      break;
    }
  }

  if ( !mFreezeWidthSpin )
    mWidthSpin->setValue( mItem->rect().width() );
  if ( !mFreezeHeightSpin )
    mHeightSpin->setValue( mItem->rect().height() );
#if 0 //TODO
  if ( !mFreezePageSpin )
    mPageSpinBox->setValue( mItem->page() );
#endif

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

void QgsLayoutItemPropertiesWidget::setValuesForGuiNonPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  mStrokeWidthSpinBox->blockSignals( true );
  mStrokeUnitsComboBox->blockSignals( true );
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
  mStrokeUnitsComboBox->setUnit( mItem->frameStrokeWidth().units() );
  mStrokeWidthSpinBox->setValue( mItem->frameStrokeWidth().length() );
  mFrameJoinStyleCombo->setPenJoinStyle( mItem->frameJoinStyle() );
  mItemIdLineEdit->setText( mItem->id() );
  mFrameGroupBox->setChecked( mItem->hasFrame() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );
#if 0//TODO
  mBlendModeCombo->setBlendMode( mItem->blendMode() );
  mOpacityWidget->setOpacity( mItem->itemOpacity() );
  mItemRotationSpinBox->setValue( mItem->itemRotation( QgsComposerObject::OriginalValue ) );
  mExcludeFromPrintsCheckBox->setChecked( mItem->excludeFromExports( QgsComposerObject::OriginalValue ) );
#endif

  mBackgroundColorButton->blockSignals( false );
  mFrameColorButton->blockSignals( false );
  mFrameJoinStyleCombo->blockSignals( false );
  mStrokeWidthSpinBox->blockSignals( false );
  mStrokeUnitsComboBox->blockSignals( false );
  mFrameGroupBox->blockSignals( false );
  mBackgroundGroupBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mBlendModeCombo->blockSignals( false );
  mOpacityWidget->blockSignals( false );
  mItemRotationSpinBox->blockSignals( false );
  mExcludeFromPrintsCheckBox->blockSignals( false );
}

void QgsLayoutItemPropertiesWidget::initializeDataDefinedButtons()
{
  mConfigObject->initializeDataDefinedButton( mXPositionDDBtn, QgsLayoutObject::PositionX );
  mConfigObject->initializeDataDefinedButton( mYPositionDDBtn, QgsLayoutObject::PositionY );
  mConfigObject->initializeDataDefinedButton( mWidthDDBtn, QgsLayoutObject::ItemWidth );
  mConfigObject->initializeDataDefinedButton( mHeightDDBtn, QgsLayoutObject::ItemHeight );
  mConfigObject->initializeDataDefinedButton( mItemRotationDDBtn, QgsLayoutObject::ItemRotation );
  mConfigObject->initializeDataDefinedButton( mOpacityDDBtn, QgsLayoutObject::Opacity );
  mConfigObject->initializeDataDefinedButton( mBlendModeDDBtn, QgsLayoutObject::BlendMode );
  mConfigObject->initializeDataDefinedButton( mExcludePrintsDDBtn, QgsLayoutObject::ExcludeFromExports );
  mConfigObject->initializeDataDefinedButton( mItemFrameColorDDBtn, QgsLayoutObject::FrameColor );
  mConfigObject->initializeDataDefinedButton( mItemBackgroundColorDDBtn, QgsLayoutObject::BackgroundColor );
}

void QgsLayoutItemPropertiesWidget::populateDataDefinedButtons()
{
  const QList< QgsPropertyOverrideButton * > buttons = findChildren< QgsPropertyOverrideButton * >();
  for ( QgsPropertyOverrideButton *button : buttons )
  {
    mConfigObject->updateDataDefinedButton( button );
  }
}

void QgsLayoutItemPropertiesWidget::setValuesForGuiElements()
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

void QgsLayoutItemPropertiesWidget::mBlendModeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Blend Mode" ) );
#if 0 //TODO
    mItem->setBlendMode( mBlendModeCombo->blendMode() );
#endif
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::opacityChanged( double value )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Opacity" ), QgsLayoutItem::UndoOpacity );
#if 0 //TODO
    mItem->setItemOpacity( value );
#endif
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::mItemIdLineEdit_editingFinished()
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Item ID" ), QgsLayoutItem::UndoSetId );
    mItem->setId( mItemIdLineEdit->text() );
    mItemIdLineEdit->setText( mItem->id() );
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::mPageSpinBox_valueChanged( int )
{
  mFreezePageSpin = true;
  changeItemPosition();
  mFreezePageSpin = false;
}

void QgsLayoutItemPropertiesWidget::mXPosSpin_valueChanged( double )
{
  mFreezeXPosSpin = true;
  changeItemPosition();
  mFreezeXPosSpin = false;
}

void QgsLayoutItemPropertiesWidget::mYPosSpin_valueChanged( double )
{
  mFreezeYPosSpin = true;
  changeItemPosition();
  mFreezeYPosSpin = false;
}

void QgsLayoutItemPropertiesWidget::mWidthSpin_valueChanged( double )
{
  mFreezeWidthSpin = true;
  changeItemPosition();
  mFreezeWidthSpin = false;
}

void QgsLayoutItemPropertiesWidget::mHeightSpin_valueChanged( double )
{
  mFreezeHeightSpin = true;
  changeItemPosition();
  mFreezeHeightSpin = false;
}

void QgsLayoutItemPropertiesWidget::mUpperLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 // TODO
    mItem->setItemPosition( mItem->pos().x(), mItem->pos().y(), QgsComposerItem::UpperLeft );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mUpperMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y(), QgsComposerItem::UpperMiddle );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mUpperRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y(), QgsComposerItem::UpperRight );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x(),
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::MiddleLeft );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::Middle );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y() + mItem->rect().height() / 2.0, QgsComposerItem::MiddleRight );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerLeftCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x(),
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerLeft );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerMiddleCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width() / 2.0,
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerMiddle );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerRightCheckBox_stateChanged( int state )
{
  if ( state != Qt::Checked )
    return;
  if ( mItem )
  {
#if 0 //TODO
    mItem->setItemPosition( mItem->pos().x() + mItem->rect().width(),
                            mItem->pos().y() + mItem->rect().height(), QgsComposerItem::LowerRight );
#endif
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mItemRotationSpinBox_valueChanged( double val )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Rotate" ), QgsLayoutItem::UndoRotation );
#if 0 //TODO
    mItem->setItemRotation( val, true );
#endif
    mItem->update();
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::mExcludeFromPrintsCheckBox_toggled( bool checked )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, checked ? tr( "Exclude from Exports" ) : tr( "Include in Exports" ) );
#if 0 //TODO
    mItem->setExcludeFromExports( checked );
#endif
    mItem->layout()->undoStack()->endCommand();
  }
}
