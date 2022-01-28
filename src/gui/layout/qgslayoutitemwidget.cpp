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
#include "qgslayoutundostack.h"
#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"
#include "qgsexpressioncontextutils.h"
#include "qgslayoutframe.h"
#include "qgssymbolbutton.h"
#include "qgsfontbutton.h"
#include "qgslayoutdesignerinterface.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutmultiframe.h"
#include <QButtonGroup>

//
// QgsLayoutConfigObject
//

QgsLayoutConfigObject::QgsLayoutConfigObject( QWidget *parent, QgsLayoutObject *layoutObject )
  : QObject( parent )
  , mLayoutObject( layoutObject )
{
  if ( mLayoutObject->layout() )
  {
    connect( &mLayoutObject->layout()->reportContext(), &QgsLayoutReportContext::layerChanged,
             this, [ = ] { updateDataDefinedButtons(); } );
  }
  if ( layoutAtlas() )
  {
    connect( layoutAtlas(), &QgsLayoutAtlas::toggled, this, &QgsLayoutConfigObject::updateDataDefinedButtons );
  }
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

  const bool propertyAssociatesWithMultiFrame = QgsLayoutObject::propertyAssociatesWithParentMultiframe( key );

  //set the data defined property and refresh the item
  if ( propertyAssociatesWithMultiFrame )
  {
    if ( QgsLayoutFrame *frame = dynamic_cast< QgsLayoutFrame * >( mLayoutObject.data() ) )
    {
      if ( QgsLayoutMultiFrame *multiFrame = frame->multiFrame() )
      {
        multiFrame->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
        multiFrame->refresh();
      }
    }
    else if ( QgsLayoutMultiFrame *multiFrame = dynamic_cast< QgsLayoutMultiFrame * >( mLayoutObject.data() ) )
    {
      multiFrame->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
      multiFrame->refresh();
    }
  }
  else if ( mLayoutObject )
  {
    mLayoutObject->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
    mLayoutObject->refresh();
  }
}

void QgsLayoutConfigObject::updateDataDefinedButtons()
{
  const QList< QgsPropertyOverrideButton * > buttons = findChildren< QgsPropertyOverrideButton * >();
  for ( QgsPropertyOverrideButton *button : buttons )
  {
    button->setVectorLayer( coverageLayer() );
  }
}

void QgsLayoutConfigObject::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key )
{
  button->blockSignals( true );
  button->init( key, mLayoutObject->dataDefinedProperties(), QgsLayoutObject::propertyDefinitions(), coverageLayer() );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLayoutConfigObject::updateDataDefinedProperty, Qt::UniqueConnection );
  button->registerExpressionContextGenerator( mLayoutObject );
  button->blockSignals( false );
}

void QgsLayoutConfigObject::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 || !mLayoutObject )
    return;

  const QgsLayoutObject::DataDefinedProperty key = static_cast< QgsLayoutObject::DataDefinedProperty >( button->propertyKey() );
  const bool propertyAssociatesWithMultiFrame = QgsLayoutObject::propertyAssociatesWithParentMultiframe( key );

  //set the data defined property
  if ( propertyAssociatesWithMultiFrame )
  {
    if ( QgsLayoutFrame *frame = dynamic_cast< QgsLayoutFrame * >( mLayoutObject.data() ) )
    {
      if ( QgsLayoutMultiFrame *multiFrame = frame->multiFrame() )
      {
        whileBlocking( button )->setToProperty( multiFrame->dataDefinedProperties().property( key ) );
      }
    }
    else if ( QgsLayoutMultiFrame *multiFrame = dynamic_cast< QgsLayoutMultiFrame * >( mLayoutObject.data() ) )
    {
      whileBlocking( button )->setToProperty( multiFrame->dataDefinedProperties().property( key ) );
    }
  }
  else if ( mLayoutObject )
  {
    whileBlocking( button )->setToProperty( mLayoutObject->dataDefinedProperties().property( key ) );
  }

  // In case the button was initialized to a different config object, we need to reconnect to it here (see https://github.com/qgis/QGIS/issues/26582 )
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLayoutConfigObject::updateDataDefinedProperty, Qt::UniqueConnection );
  button->registerExpressionContextGenerator( mLayoutObject );
}

QgsLayoutAtlas *QgsLayoutConfigObject::layoutAtlas() const
{
  if ( !mLayoutObject )
  {
    return nullptr;
  }

  QgsPrintLayout *printLayout = qobject_cast< QgsPrintLayout * >( mLayoutObject->layout() );

  if ( !printLayout )
  {
    return nullptr;
  }

  return printLayout->atlas();
}

void QgsLayoutConfigObject::setObject( QgsLayoutObject *object )
{
  mLayoutObject = object;
}

QgsVectorLayer *QgsLayoutConfigObject::coverageLayer() const
{
  if ( !mLayoutObject )
    return nullptr;

  QgsLayout *layout = mLayoutObject->layout();
  if ( !layout )
    return nullptr;

  return layout->reportContext().layer();
}


//
// QgsLayoutItemBaseWidget
//

QgsLayoutItemBaseWidget::QgsLayoutItemBaseWidget( QWidget *parent, QgsLayoutObject *layoutObject )
  : QgsPanelWidget( parent )
  , mConfigObject( new QgsLayoutConfigObject( this, layoutObject ) )
  , mObject( layoutObject )
{

}

QgsLayoutObject *QgsLayoutItemBaseWidget::layoutObject()
{
  return mObject;
}

bool QgsLayoutItemBaseWidget::setItem( QgsLayoutItem *item )
{
  QgsLayoutObject *oldObject = mObject;
  QgsLayoutConfigObject *oldConfigObject = mConfigObject;
  // have to set new mObject/mConfigObject here, because setNewItem methods require access to them
  mObject = item;
  mConfigObject = new QgsLayoutConfigObject( this, mObject );
  if ( setNewItem( item ) )
  {
    oldConfigObject->deleteLater();
    return true;
  }
  else
  {
    // revert object change since it was unsuccessful
    mObject = oldObject;
    mConfigObject->deleteLater();
    mConfigObject = oldConfigObject;
    return false;
  }
}

void QgsLayoutItemBaseWidget::setReportTypeString( const QString & )
{
}

void QgsLayoutItemBaseWidget::setDesignerInterface( QgsLayoutDesignerInterface *iface )
{
  const auto symbolButtonWidgets = findChildren<QgsSymbolButton *>();
  for ( QgsSymbolButton *symbolWidget : symbolButtonWidgets )
  {
    symbolWidget->setMessageBar( iface->messageBar() );
  }
  const auto fontButtonWidgets = findChildren<QgsFontButton *>();
  for ( QgsFontButton *fontButton : fontButtonWidgets )
  {
    fontButton->setMessageBar( iface->messageBar() );
  }
}

void QgsLayoutItemBaseWidget::setMasterLayout( QgsMasterLayoutInterface * )
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

bool QgsLayoutItemBaseWidget::setNewItem( QgsLayoutItem * )
{
  return false;
}

QgsLayoutAtlas *QgsLayoutItemBaseWidget::layoutAtlas() const
{
  return mConfigObject->layoutAtlas();
}

//


//QgsLayoutItemPropertiesWidget

void QgsLayoutItemPropertiesWidget::updateVariables()
{
  if ( !mItem )
    return;

  mBlockVariableUpdates = true;
  QgsExpressionContext context = mItem->createExpressionContext();
  mVariableEditor->setContext( &context );

  // here, we prefer to make the multiframe's scope the editable one. That's because most expressions are evaluated
  // on the multiframe subclass level, not on a frame-by-frame basis. Ideally both would be editable, but for now let's go
  // with the most useful one.
  const int multiFrameScopeIndex = context.indexOfScope( tr( "Multiframe Item" ) );
  const int itemScopeIndex = context.indexOfScope( tr( "Layout Item" ) );
  if ( multiFrameScopeIndex >= 0 )
    mVariableEditor->setEditableScopeIndex( multiFrameScopeIndex );
  else if ( itemScopeIndex >= 0 )
    mVariableEditor->setEditableScopeIndex( itemScopeIndex );
  mBlockVariableUpdates = false;
}

QgsLayoutItemPropertiesWidget::QgsLayoutItemPropertiesWidget( QWidget *parent, QgsLayoutItem *item )
  : QWidget( parent )
  , mConfigObject( new QgsLayoutConfigObject( this, item ) )
{
  setupUi( this );

  mVariableEditor->setMinimumHeight( mVariableEditor->fontMetrics().height() * 15 );

  mItemRotationSpinBox->setClearValue( 0 );
  mStrokeUnitsComboBox->linkToWidget( mStrokeWidthSpinBox );
  mStrokeUnitsComboBox->setConverter( &item->layout()->renderContext().measurementConverter() );

  mPosUnitsComboBox->linkToWidget( mXPosSpin );
  mPosUnitsComboBox->linkToWidget( mYPosSpin );
  mSizeUnitsComboBox->linkToWidget( mWidthSpin );
  mSizeUnitsComboBox->linkToWidget( mHeightSpin );

  mPosUnitsComboBox->setConverter( &item->layout()->renderContext().measurementConverter() );
  mSizeUnitsComboBox->setConverter( &item->layout()->renderContext().measurementConverter() );

  mPosLockAspectRatio->setWidthSpinBox( mXPosSpin );
  mPosLockAspectRatio->setHeightSpinBox( mYPosSpin );
  mSizeLockAspectRatio->setWidthSpinBox( mWidthSpin );
  mSizeLockAspectRatio->setHeightSpinBox( mHeightSpin );

  mItemFrameColorDDBtn->registerLinkedWidget( mFrameColorButton );
  mItemBackgroundColorDDBtn->registerLinkedWidget( mBackgroundColorButton );

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
  connect( mPosUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutItemPropertiesWidget::positionUnitsChanged );
  connect( mWidthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mWidthSpin_valueChanged );
  connect( mHeightSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mHeightSpin_valueChanged );
  connect( mSizeUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutItemPropertiesWidget::sizeUnitsChanged );
  connect( mUpperLeftRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mUpperLeftCheckBox_stateChanged );
  connect( mUpperMiddleRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mUpperMiddleCheckBox_stateChanged );
  connect( mUpperRightRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mUpperRightCheckBox_stateChanged );
  connect( mMiddleLeftRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mMiddleLeftCheckBox_stateChanged );
  connect( mMiddleRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mMiddleCheckBox_stateChanged );
  connect( mMiddleRightRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mMiddleRightCheckBox_stateChanged );
  connect( mLowerLeftRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mLowerLeftCheckBox_stateChanged );
  connect( mLowerMiddleRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mLowerMiddleCheckBox_stateChanged );
  connect( mLowerRightRadioButton, &QRadioButton::toggled, this, &QgsLayoutItemPropertiesWidget::mLowerRightCheckBox_stateChanged );
  connect( mBlendModeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutItemPropertiesWidget::mBlendModeCombo_currentIndexChanged );
  connect( mItemRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutItemPropertiesWidget::mItemRotationSpinBox_valueChanged );
  connect( mExcludeFromPrintsCheckBox, &QCheckBox::toggled, this, &QgsLayoutItemPropertiesWidget::mExcludeFromPrintsCheckBox_toggled );

  //make button exclusive
  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->addButton( mUpperLeftRadioButton );
  buttonGroup->addButton( mUpperMiddleRadioButton );
  buttonGroup->addButton( mUpperRightRadioButton );
  buttonGroup->addButton( mMiddleLeftRadioButton );
  buttonGroup->addButton( mMiddleRadioButton );
  buttonGroup->addButton( mMiddleRightRadioButton );
  buttonGroup->addButton( mLowerLeftRadioButton );
  buttonGroup->addButton( mLowerMiddleRadioButton );
  buttonGroup->addButton( mLowerRightRadioButton );
  buttonGroup->setExclusive( true );

  initializeDataDefinedButtons();

  setItem( item );

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsLayoutItemPropertiesWidget::opacityChanged );

  updateVariables();
  connect( mVariableEditor, &QgsVariableEditorWidget::scopeChanged, this, [ = ]
  {
    if ( !mBlockVariableUpdates )
      QgsLayoutItemPropertiesWidget::variablesChanged();
  } );
  // listen out for variable edits
  connect( QgsApplication::instance(), &QgsApplication::customVariablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
  connect( item->layout()->project(), &QgsProject::customVariablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
  connect( item->layout()->project(), &QgsProject::metadataChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );

  if ( item->layout() )
  {
    connect( item->layout(), &QgsLayout::variablesChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
    connect( &item->layout()->renderContext(), &QgsLayoutRenderContext::dpiChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
    connect( item->layout()->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutItemPropertiesWidget::updateVariables );
  }
}

void QgsLayoutItemPropertiesWidget::showBackgroundGroup( bool showGroup )
{
  mBackgroundGroupBox->setVisible( showGroup );
}

void QgsLayoutItemPropertiesWidget::showFrameGroup( bool showGroup )
{
  mFrameGroupBox->setVisible( showGroup );
}

void QgsLayoutItemPropertiesWidget::setItem( QgsLayoutItem *item )
{
  if ( mItem )
  {
    disconnect( mItem, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements );
    disconnect( mItem, &QgsLayoutObject::changed, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiNonPositionElements );
  }
  mItem = item;
  if ( mItem )
  {
    connect( mItem, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements );
    connect( mItem, &QgsLayoutObject::changed, this, &QgsLayoutItemPropertiesWidget::setValuesForGuiNonPositionElements );
  }

  mConfigObject->setObject( mItem );

  setValuesForGuiElements();
}

void QgsLayoutItemPropertiesWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( QgsPrintLayout *printLayout = dynamic_cast< QgsPrintLayout * >( masterLayout ) )
  {
    connect( printLayout, &QgsPrintLayout::nameChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
    connect( printLayout->atlas(), &QgsLayoutAtlas::coverageLayerChanged, this, &QgsLayoutItemPropertiesWidget::updateVariables );
  }
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
  mItem->layout()->undoStack()->endCommand();
  mItem->invalidateCache();
}

void QgsLayoutItemPropertiesWidget::changeItemPosition()
{
  if ( !mItem )
    return;

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Move Item" ), QgsLayoutItem::UndoIncrementalMove );

  const QgsLayoutPoint point( mXPosSpin->value(), mYPosSpin->value(), mPosUnitsComboBox->unit() );
  mItem->attemptMove( point, true, false, mPageSpinBox->value() - 1 );

  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::changeItemReference( QgsLayoutItem::ReferencePoint point )
{
  if ( !mItem )
    return;

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Item Reference" ) );
  mItem->setReferencePoint( point );
  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::changeItemSize()
{
  if ( !mItem )
    return;

  mItem->layout()->undoStack()->beginCommand( mItem, tr( "Resize Item" ), QgsLayoutItem::UndoIncrementalResize );

  const QgsLayoutSize size( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() );
  mItem->attemptResize( size );

  mItem->layout()->undoStack()->endCommand();
}

void QgsLayoutItemPropertiesWidget::variablesChanged()
{
  if ( !mItem )
    return;

  if ( QgsLayoutFrame *frame = qobject_cast< QgsLayoutFrame * >( mItem ) )
  {
    if ( QgsLayoutMultiFrame *mf = frame->multiFrame() )
    {
      QgsExpressionContextUtils::setLayoutMultiFrameVariables( mf, mVariableEditor->variablesInActiveScope() );
    }
  }
  else
  {
    QgsExpressionContextUtils::setLayoutItemVariables( mItem, mVariableEditor->variablesInActiveScope() );
  }
}

QgsLayoutItem::ReferencePoint QgsLayoutItemPropertiesWidget::positionMode() const
{
  if ( mUpperLeftRadioButton->isChecked() )
  {
    return QgsLayoutItem::UpperLeft;
  }
  else if ( mUpperMiddleRadioButton->isChecked() )
  {
    return QgsLayoutItem::UpperMiddle;
  }
  else if ( mUpperRightRadioButton->isChecked() )
  {
    return QgsLayoutItem::UpperRight;
  }
  else if ( mMiddleLeftRadioButton->isChecked() )
  {
    return QgsLayoutItem::MiddleLeft;
  }
  else if ( mMiddleRadioButton->isChecked() )
  {
    return QgsLayoutItem::Middle;
  }
  else if ( mMiddleRightRadioButton->isChecked() )
  {
    return QgsLayoutItem::MiddleRight;
  }
  else if ( mLowerLeftRadioButton->isChecked() )
  {
    return QgsLayoutItem::LowerLeft;
  }
  else if ( mLowerMiddleRadioButton->isChecked() )
  {
    return QgsLayoutItem::LowerMiddle;
  }
  else if ( mLowerRightRadioButton->isChecked() )
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
  Q_UNUSED( index )
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
  mItem->layout()->undoStack()->endCommand();
  mItem->invalidateCache();
}


void QgsLayoutItemPropertiesWidget::setValuesForGuiPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  auto block = [ = ]( bool blocked )
  {
    mXPosSpin->blockSignals( blocked );
    mYPosSpin->blockSignals( blocked );
    mPosUnitsComboBox->blockSignals( blocked );
    mWidthSpin->blockSignals( blocked );
    mHeightSpin->blockSignals( blocked );
    mSizeUnitsComboBox->blockSignals( blocked );
    mUpperLeftRadioButton->blockSignals( blocked );
    mUpperMiddleRadioButton->blockSignals( blocked );
    mUpperRightRadioButton->blockSignals( blocked );
    mMiddleLeftRadioButton->blockSignals( blocked );
    mMiddleRadioButton->blockSignals( blocked );
    mMiddleRightRadioButton->blockSignals( blocked );
    mLowerLeftRadioButton->blockSignals( blocked );
    mLowerMiddleRadioButton->blockSignals( blocked );
    mLowerRightRadioButton->blockSignals( blocked );
    mPageSpinBox->blockSignals( blocked );
  };
  block( true );

  const QgsLayoutPoint point = mItem->pagePositionWithUnits();

  if ( !mFreezeXPosSpin )
    mXPosSpin->setValue( point.x() );
  if ( !mFreezeYPosSpin )
    mYPosSpin->setValue( point.y() );
  mPosUnitsComboBox->setUnit( point.units() );

  switch ( mItem->referencePoint() )
  {
    case QgsLayoutItem::UpperLeft:
    {
      mUpperLeftRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::UpperMiddle:
    {
      mUpperMiddleRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::UpperRight:
    {
      mUpperRightRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::MiddleLeft:
    {
      mMiddleLeftRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::Middle:
    {
      mMiddleRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::MiddleRight:
    {
      mMiddleRightRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::LowerLeft:
    {
      mLowerLeftRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::LowerMiddle:
    {
      mLowerMiddleRadioButton->setChecked( true );
      break;
    }

    case QgsLayoutItem::LowerRight:
    {
      mLowerRightRadioButton->setChecked( true );
      break;
    }
  }

  const QgsLayoutSize size = mItem->sizeWithUnits();
  if ( !mFreezeWidthSpin )
    mWidthSpin->setValue( size.width() );
  if ( !mFreezeHeightSpin )
    mHeightSpin->setValue( size.height() );

  mSizeUnitsComboBox->setUnit( size.units() );

  mSizeLockAspectRatio->resetRatio();
  mPosLockAspectRatio->resetRatio();

  if ( !mFreezePageSpin )
    mPageSpinBox->setValue( mItem->page() + 1 );

  block( false );
}

void QgsLayoutItemPropertiesWidget::setValuesForGuiNonPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  auto block = [ = ]( bool blocked )
  {
    mStrokeWidthSpinBox->blockSignals( blocked );
    mStrokeUnitsComboBox->blockSignals( blocked );
    mFrameGroupBox->blockSignals( blocked );
    mBackgroundGroupBox->blockSignals( blocked );
    mItemIdLineEdit->blockSignals( blocked );
    mBlendModeCombo->blockSignals( blocked );
    mOpacityWidget->blockSignals( blocked );
    mFrameColorButton->blockSignals( blocked );
    mFrameJoinStyleCombo->blockSignals( blocked );
    mBackgroundColorButton->blockSignals( blocked );
    mItemRotationSpinBox->blockSignals( blocked );
    mExcludeFromPrintsCheckBox->blockSignals( blocked );
  };
  block( true );

  mBackgroundColorButton->setColor( mItem->backgroundColor() );
  mFrameColorButton->setColor( mItem->frameStrokeColor() );
  mStrokeUnitsComboBox->setUnit( mItem->frameStrokeWidth().units() );
  mStrokeWidthSpinBox->setValue( mItem->frameStrokeWidth().length() );
  mFrameJoinStyleCombo->setPenJoinStyle( mItem->frameJoinStyle() );
  mItemIdLineEdit->setText( mItem->id() );
  mFrameGroupBox->setChecked( mItem->frameEnabled() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );
  mBlendModeCombo->setBlendMode( mItem->blendMode() );
  mOpacityWidget->setOpacity( mItem->itemOpacity() );
  mItemRotationSpinBox->setValue( mItem->itemRotation() );
  mExcludeFromPrintsCheckBox->setChecked( mItem->excludeFromExports() );

  block( false );
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

  updateVariables();
}

void QgsLayoutItemPropertiesWidget::mBlendModeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Blend Mode" ) );
    mItem->setBlendMode( mBlendModeCombo->blendMode() );
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::opacityChanged( double value )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Change Opacity" ), QgsLayoutItem::UndoOpacity );
    mItem->setItemOpacity( value );
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

void QgsLayoutItemPropertiesWidget::positionUnitsChanged( QgsUnitTypes::LayoutUnit )
{
  changeItemPosition();
}

void QgsLayoutItemPropertiesWidget::mWidthSpin_valueChanged( double )
{
  mFreezeWidthSpin = true;
  changeItemSize();
  mFreezeWidthSpin = false;
}

void QgsLayoutItemPropertiesWidget::mHeightSpin_valueChanged( double )
{
  mFreezeHeightSpin = true;
  changeItemSize();
  mFreezeHeightSpin = false;
}

void QgsLayoutItemPropertiesWidget::sizeUnitsChanged( QgsUnitTypes::LayoutUnit )
{
  changeItemSize();
}

void QgsLayoutItemPropertiesWidget::mUpperLeftCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;

  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::UpperLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mUpperMiddleCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::UpperMiddle );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mUpperRightCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::UpperRight );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleLeftCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::MiddleLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::Middle );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mMiddleRightCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::MiddleRight );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerLeftCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::LowerLeft );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerMiddleCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::LowerMiddle );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mLowerRightCheckBox_stateChanged( bool state )
{
  if ( !state )
    return;
  if ( mItem )
  {
    changeItemReference( QgsLayoutItem::LowerRight );
  }
  setValuesForGuiPositionElements();
}

void QgsLayoutItemPropertiesWidget::mItemRotationSpinBox_valueChanged( double val )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, tr( "Rotate" ), QgsLayoutItem::UndoRotation );
    mItem->setItemRotation( val, true );
    mItem->update();
    mItem->layout()->undoStack()->endCommand();
  }
}

void QgsLayoutItemPropertiesWidget::mExcludeFromPrintsCheckBox_toggled( bool checked )
{
  if ( mItem )
  {
    mItem->layout()->undoStack()->beginCommand( mItem, checked ? tr( "Exclude from Exports" ) : tr( "Include in Exports" ) );
    mItem->setExcludeFromExports( checked );
    mItem->layout()->undoStack()->endCommand();
  }
}
