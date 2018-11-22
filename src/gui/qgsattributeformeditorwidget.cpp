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
#include "qgssearchwidgettoolbutton.h"
#include "qgseditorwidgetwrapper.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetregistry.h"
#include "qgsaggregatetoolbutton.h"
#include "qgsgui.h"

#include <QLayout>
#include <QLabel>
#include <QStackedWidget>

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType, QgsAttributeForm *form )
  : QgsAttributeFormWidget( editorWidget, form )
  , mWidgetType( widgetType )
  , mWidget( editorWidget )
  , mForm( form )
  , mMultiEditButton( new QgsMultiEditToolButton() )
  , mBlockValueUpdate( false )
  , mIsMixed( false )
  , mIsChanged( false )
{
  mConstraintResultLabel = new QLabel( this );
  mConstraintResultLabel->setObjectName( QStringLiteral( "ConstraintStatus" ) );
  mConstraintResultLabel->setSizePolicy( QSizePolicy::Fixed, mConstraintResultLabel->sizePolicy().verticalPolicy() );

  mMultiEditButton->setField( mWidget->field() );
  mAggregateButton = new QgsAggregateToolButton();
  mAggregateButton->setType( editorWidget->field().type() );
  connect( mAggregateButton, &QgsAggregateToolButton::aggregateChanged, this, &QgsAttributeFormEditorWidget::onAggregateChanged );

  if ( mWidget->widget() )
  {
    mWidget->widget()->setObjectName( mWidget->field().name() );
  }

  connect( mWidget, static_cast<void ( QgsEditorWidgetWrapper::* )( const QVariant &value )>( &QgsEditorWidgetWrapper::valueChanged ), this, &QgsAttributeFormEditorWidget::editorWidgetChanged );

  connect( mMultiEditButton, &QgsMultiEditToolButton::resetFieldValueTriggered, this, &QgsAttributeFormEditorWidget::resetValue );
  connect( mMultiEditButton, &QgsMultiEditToolButton::setFieldValueTriggered, this, &QgsAttributeFormEditorWidget::setFieldTriggered );

  mMultiEditButton->setField( mWidget->field() );

  updateWidgets();
}

QgsAttributeFormEditorWidget::~QgsAttributeFormEditorWidget()
{
  //there's a chance these widgets are not currently added to the layout, so have no parent set
  delete mMultiEditButton;
}

void QgsAttributeFormEditorWidget::createSearchWidgetWrappers( const QgsAttributeEditorContext &context )
{
  Q_ASSERT( !mWidgetType.isEmpty() );
  const QVariantMap config = mWidget->config();
  const int fieldIdx = mWidget->fieldIdx();

  QgsSearchWidgetWrapper *sww = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config,
                                searchWidgetFrame(), context );
  setSearchWidgetWrapper( sww );
  searchWidgetFrame()->layout()->addWidget( mAggregateButton );
  if ( sww->supportedFlags() & QgsSearchWidgetWrapper::Between ||
       sww->supportedFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // create secondary widget for between type searches
    QgsSearchWidgetWrapper *sww2 = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config,
                                   searchWidgetFrame(), context );
    addAdditionalSearchWidgetWrapper( sww2 );
  }
}

void QgsAttributeFormEditorWidget::setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result )
{
  switch ( result )
  {
    case QgsEditorWidgetWrapper::ConstraintResultFailHard:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#FF9800\">%1</font>" ).arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? QStringLiteral( "<b>%1</b>: %2" ).arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultFailSoft:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#FFC107\">%1</font>" ).arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? QStringLiteral( "<b>%1</b>: %2" ).arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultPass:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#259B24\">%1</font>" ).arg( QChar( 0x2714 ) ) );
      mConstraintResultLabel->setToolTip( QString() );
      break;
  }
}

void QgsAttributeFormEditorWidget::setConstraintResultVisible( bool editable )
{
  mConstraintResultLabel->setHidden( !editable );
}

void QgsAttributeFormEditorWidget::setIsMixed( bool mixed )
{
  if ( mWidget && mixed )
    mWidget->showIndeterminateState();
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



void QgsAttributeFormEditorWidget::initialize( const QVariant &initialValue, bool mixedValues )
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



void QgsAttributeFormEditorWidget::editorWidgetChanged( const QVariant &value )
{
  if ( mBlockValueUpdate )
    return;

  mIsChanged = true;

  switch ( mode() )
  {
    case DefaultMode:
    case SearchMode:
    case AggregateSearchMode:
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

  switch ( mode() )
  {
    case DefaultMode:
    case SearchMode:
    case AggregateSearchMode:
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

void QgsAttributeFormEditorWidget::onAggregateChanged()
{
  for ( QgsSearchWidgetWrapper *searchWidget : searchWidgetWrappers() )
    searchWidget->setAggregate( mAggregateButton->aggregate() );
}

void QgsAttributeFormEditorWidget::updateWidgets()
{
  //first update the tool buttons
  bool hasMultiEditButton = ( editPage()->layout()->indexOf( mMultiEditButton ) >= 0 );
  bool fieldReadOnly = layer()->editFormConfig().readOnly( mWidget->fieldIdx() );

  if ( hasMultiEditButton )
  {
    if ( mode() != MultiEditMode || fieldReadOnly )
    {
      editPage()->layout()->removeWidget( mMultiEditButton );
      mMultiEditButton->setParent( nullptr );
    }
  }
  else
  {
    if ( mode() == MultiEditMode && !fieldReadOnly )
    {
      editPage()->layout()->addWidget( mMultiEditButton );
    }
  }

  switch ( mode() )
  {
    case DefaultMode:
    case MultiEditMode:
    {
      stack()->setCurrentWidget( editPage() );

      editPage()->layout()->addWidget( mConstraintResultLabel );

      break;
    }

    case AggregateSearchMode:
    {
      mAggregateButton->setVisible( true );
      stack()->setCurrentWidget( searchPage() );
      break;
    }

    case SearchMode:
    {
      mAggregateButton->setVisible( false );
      stack()->setCurrentWidget( searchPage() );
      break;
    }
  }
}
