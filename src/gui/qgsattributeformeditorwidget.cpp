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
#include "qgseditorwidgetconfig.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetregistry.h"
#include <QLayout>
#include <QLabel>
#include <QStackedWidget>

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper* editorWidget,
    QgsAttributeForm* form )
    : QWidget( form )
    , mWidget( editorWidget )
    , mForm( form )
    , mMode( DefaultMode )
    , mMultiEditButton( new QgsMultiEditToolButton() )
    , mBlockValueUpdate( false )
    , mIsMixed( false )
    , mIsChanged( false )
{
  mEditPage = new QWidget();
  QHBoxLayout* l = new QHBoxLayout();
  l->setMargin( 0 );
  l->setContentsMargins( 0, 0, 0, 0 );
  mEditPage->setLayout( l );

  l = new QHBoxLayout();
  l->setMargin( 0 );
  l->setContentsMargins( 0, 0, 0, 0 );
  mSearchFrame = new QWidget();
  mSearchFrame->setLayout( l );

  mSearchPage = new QWidget();
  l = new QHBoxLayout();
  l->setMargin( 0 );
  l->setContentsMargins( 0, 0, 0, 0 );
  mSearchPage->setLayout( l );
  l->addWidget( mSearchFrame, 1 );
  mSearchWidgetToolButton = new QgsSearchWidgetToolButton();
  connect( mSearchWidgetToolButton, SIGNAL( activeFlagsChanged( QgsSearchWidgetWrapper::FilterFlags ) ),
           this, SLOT( searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags ) ) );
  l->addWidget( mSearchWidgetToolButton, 0 );


  mStack = new QStackedWidget;
  mStack->addWidget( mEditPage );
  mStack->addWidget( mSearchPage );

  l = new QHBoxLayout();
  l->setMargin( 0 );
  l->setContentsMargins( 0, 0, 0, 0 );
  setLayout( l );
  l->addWidget( mStack );

  if ( !mWidget || !mForm )
    return;

  mEditPage->layout()->addWidget( mWidget->widget() );

  if ( mWidget->widget() )
  {
    mWidget->widget()->setObjectName( mWidget->field().name() );
  }
  connect( mWidget, SIGNAL( valueChanged( const QVariant& ) ), this, SLOT( editorWidgetChanged( const QVariant & ) ) );
  connect( mMultiEditButton, SIGNAL( resetFieldValueTriggered() ), this, SLOT( resetValue() ) );
  connect( mMultiEditButton, SIGNAL( setFieldValueTriggered() ), this, SLOT( setFieldTriggered() ) );

  mMultiEditButton->setField( mWidget->field() );

  updateWidgets();
}

QgsAttributeFormEditorWidget::~QgsAttributeFormEditorWidget()
{
  //there's a chance these widgets are not currently added to the layout, so have no parent set
  delete mMultiEditButton;
}

void QgsAttributeFormEditorWidget::createSearchWidgetWrappers( const QString& widgetId, int fieldIdx, const QgsEditorWidgetConfig& config,  const QgsAttributeEditorContext& context )
{
  QgsSearchWidgetWrapper* sww = QgsEditorWidgetRegistry::instance()->createSearchWidget( widgetId, layer(), fieldIdx, config,
                                mSearchFrame, context );
  setSearchWidgetWrapper( sww );
  if ( sww->supportedFlags() & QgsSearchWidgetWrapper::Between ||
       sww->supportedFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // create secondary widget for between type searches
    QgsSearchWidgetWrapper* sww2 = QgsEditorWidgetRegistry::instance()->createSearchWidget( widgetId, layer(), fieldIdx, config,
                                   mSearchFrame, context );
    mSearchWidgets << sww2;
    mSearchFrame->layout()->addWidget( sww2->widget() );
    sww2->widget()->hide();
  }
}

void QgsAttributeFormEditorWidget::setSearchWidgetWrapper( QgsSearchWidgetWrapper* wrapper )
{
  mSearchWidgets.clear();
  mSearchWidgets << wrapper;
  mSearchFrame->layout()->addWidget( wrapper->widget() );
  mSearchWidgetToolButton->setAvailableFlags( wrapper->supportedFlags() );
  mSearchWidgetToolButton->setActiveFlags( QgsSearchWidgetWrapper::FilterFlags() );
  mSearchWidgetToolButton->setDefaultFlags( wrapper->defaultFlags() );
  connect( wrapper, SIGNAL( valueChanged() ), mSearchWidgetToolButton, SLOT( searchWidgetValueChanged() ) );
  connect( wrapper, SIGNAL( valueCleared() ), mSearchWidgetToolButton, SLOT( setInactive() ) );
}

QWidget*QgsAttributeFormEditorWidget::searchWidgetFrame()
{
  return mSearchFrame;
}

QList< QgsSearchWidgetWrapper* > QgsAttributeFormEditorWidget::searchWidgetWrappers()
{
  return mSearchWidgets;
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

void QgsAttributeFormEditorWidget::resetSearch()
{
  mSearchWidgetToolButton->setInactive();
  Q_FOREACH ( QgsSearchWidgetWrapper* widget, mSearchWidgets )
  {
    widget->clearWidget();
  }
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

QString QgsAttributeFormEditorWidget::currentFilterExpression() const
{
  if ( mSearchWidgets.isEmpty() )
    return QString();

  if ( !mSearchWidgetToolButton->isActive() )
    return QString();

  if ( mSearchWidgetToolButton->activeFlags() & QgsSearchWidgetWrapper::Between )
  {
    // special case: Between search
    QString filter1 = mSearchWidgets.at( 0 )->createExpression( QgsSearchWidgetWrapper::GreaterThanOrEqualTo );
    QString filter2 = mSearchWidgets.at( 1 )->createExpression( QgsSearchWidgetWrapper::LessThanOrEqualTo );
    return QString( "%1 AND %2" ).arg( filter1, filter2 );
  }
  else if ( mSearchWidgetToolButton->activeFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // special case: Is Not Between search
    QString filter1 = mSearchWidgets.at( 0 )->createExpression( QgsSearchWidgetWrapper::LessThan );
    QString filter2 = mSearchWidgets.at( 1 )->createExpression( QgsSearchWidgetWrapper::GreaterThan );
    return QString( "%1 OR %2" ).arg( filter1, filter2 );
  }

  return mSearchWidgets.at( 0 )->createExpression( mSearchWidgetToolButton->activeFlags() );
}

void QgsAttributeFormEditorWidget::editorWidgetChanged( const QVariant& value )
{
  if ( mBlockValueUpdate )
    return;

  mIsChanged = true;

  switch ( mMode )
  {
    case DefaultMode:
    case SearchMode:
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
    case SearchMode:
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

void QgsAttributeFormEditorWidget::searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags )
{
  Q_FOREACH ( QgsSearchWidgetWrapper* widget, mSearchWidgets )
  {
    widget->setEnabled( !( flags & QgsSearchWidgetWrapper::IsNull )
                        && !( flags & QgsSearchWidgetWrapper::IsNotNull ) );
    if ( !mSearchWidgetToolButton->isActive() )
    {
      widget->clearWidget();
    }
  }

  if ( mSearchWidgets.count() >= 2 )
  {
    mSearchWidgets.at( 1 )->widget()->setVisible( flags & QgsSearchWidgetWrapper::Between ||
        flags & QgsSearchWidgetWrapper::IsNotBetween );
  }
}

QgsSearchWidgetToolButton* QgsAttributeFormEditorWidget::searchWidgetToolButton()
{
  return mSearchWidgetToolButton;
}

QgsVectorLayer* QgsAttributeFormEditorWidget::layer()
{
  return mForm ? mForm->layer() : nullptr;
}

void QgsAttributeFormEditorWidget::updateWidgets()
{
  //first update the tool buttons
  bool hasMultiEditButton = ( mEditPage->layout()->indexOf( mMultiEditButton ) >= 0 );
  bool fieldReadOnly = layer()->editFormConfig()->readOnly( mWidget->fieldIdx() );

  if ( hasMultiEditButton )
  {
    if ( mMode != MultiEditMode || fieldReadOnly )
    {
      mEditPage->layout()->removeWidget( mMultiEditButton );
      mMultiEditButton->setParent( nullptr );
    }
  }
  else
  {
    if ( mMode == MultiEditMode && !fieldReadOnly )
    {
      mEditPage->layout()->addWidget( mMultiEditButton );
    }
  }

  switch ( mMode )
  {
    case DefaultMode:
    case MultiEditMode:
    {
      mStack->setCurrentWidget( mEditPage );
      break;
    }

    case SearchMode:
    {
      mStack->setCurrentWidget( mSearchPage );
      break;
    }
  }
}
