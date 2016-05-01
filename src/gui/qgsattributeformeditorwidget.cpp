/***************************************************************************
    qgsattributeformeditorwidget.cpp
     -------------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeformeditorwidget.h"
#include "qgsattributeform.h"
#include "qgsmultiedittoolbutton.h"
#include "qgseditorwidgetwrapper.h"
#include <QLayout>
#include <QLabel>

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper* editorWidget, QgsAttributeForm* form )
    : QWidget( form )
    , mWidget( editorWidget )
    , mForm( form )
    , mMode( DefaultMode )
    , mMultiEditButton( new QgsMultiEditToolButton() )
    , mBlockValueUpdate( false )
    , mIsMixed( false )
    , mIsChanged( false )
{
  if ( !mWidget || !mForm )
    return;

  QLayout* l = new QHBoxLayout();
  l->setMargin( 0 );
  l->setContentsMargins( 0, 0, 0, 0 );
  l->addWidget( mWidget->widget() );

  if ( mWidget->widget() )
  {
    mWidget->widget()->setObjectName( mWidget->field().name() );
  }
  connect( mWidget, SIGNAL( valueChanged( const QVariant& ) ), this, SLOT( editorWidgetChanged( const QVariant & ) ) );
  connect( mMultiEditButton, SIGNAL( resetFieldValueTriggered() ), this, SLOT( resetValue() ) );
  connect( mMultiEditButton, SIGNAL( setFieldValueTriggered() ), this, SLOT( setFieldTriggered() ) );

  mMultiEditButton->setField( mWidget->field() );

  setLayout( l );
  updateWidgets();
}

QgsAttributeFormEditorWidget::~QgsAttributeFormEditorWidget()
{
  //there's a chance these widgets are not currently added to the layout, so have no parent set
  delete mMultiEditButton;
}

void QgsAttributeFormEditorWidget::setMode( QgsAttributeFormEditorWidget::Mode mode )
{
  mMode = mode;
  updateWidgets();
}

void QgsAttributeFormEditorWidget::setIsMixed( bool mixed )
{
  if ( mWidget && mixed )
    mWidget->showIndeterminateState( );
  mMultiEditButton->setIsMixed( mixed );
  mIsMixed = mixed;
}

void QgsAttributeFormEditorWidget::changesCommitted()
{
  if ( mWidget )
    mPreviousValue = mWidget->value();

  setIsMixed( false );
  mMultiEditButton->changesCommitted();
  mIsChanged = false;
}

void QgsAttributeFormEditorWidget::initialize( const QVariant& initialValue, bool mixedValues )
{
  if ( mWidget )
  {
    mBlockValueUpdate = true;
    mWidget->setValue( initialValue );
    mBlockValueUpdate = false;
  }
  mPreviousValue = initialValue;
  setIsMixed( mixedValues );
  mMultiEditButton->setIsChanged( false );
  mIsChanged = false;
}

QVariant QgsAttributeFormEditorWidget::currentValue() const
{
  return mWidget->value();
}

void QgsAttributeFormEditorWidget::editorWidgetChanged( const QVariant& value )
{
  if ( mBlockValueUpdate )
    return;

  mIsChanged = true;

  switch ( mMode )
  {
    case DefaultMode:
      break;
    case MultiEditMode:
      mMultiEditButton->setIsChanged( true );
  }

  emit valueChanged( value );
}

void QgsAttributeFormEditorWidget::resetValue()
{
  mIsChanged = false;
  mBlockValueUpdate = true;
  if ( mWidget )
    mWidget->setValue( mPreviousValue );
  mBlockValueUpdate = false;

  switch ( mMode )
  {
    case DefaultMode:
      break;
    case MultiEditMode:
    {
      mMultiEditButton->setIsChanged( false );
      if ( mWidget && mIsMixed )
        mWidget->showIndeterminateState();
      break;
    }
  }
}

void QgsAttributeFormEditorWidget::setFieldTriggered()
{
  mIsChanged = true;
}

QgsVectorLayer* QgsAttributeFormEditorWidget::layer()
{
  return mForm ? mForm->layer() : nullptr;
}

void QgsAttributeFormEditorWidget::updateWidgets()
{
  bool hasMultiEditButton = ( layout()->indexOf( mMultiEditButton ) >= 0 );
  bool fieldReadOnly = layer()->editFormConfig()->readOnly( mWidget->fieldIdx() );

  if ( hasMultiEditButton )
  {
    if ( mMode != MultiEditMode || fieldReadOnly )
    {
      layout()->removeWidget( mMultiEditButton );
      mMultiEditButton->setParent( nullptr );
    }
  }
  else
  {
    if ( mMode == MultiEditMode && !fieldReadOnly )
    {
      layout()->addWidget( mMultiEditButton );
    }
  }
}
