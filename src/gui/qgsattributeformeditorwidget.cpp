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
#include "qgsgui.h"
#include <QLayout>
#include <QLabel>
#include <QStackedWidget>

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget,
    QgsAttributeForm *form )
  : QWidget( form )
  , mWidget( editorWidget )
  , mForm( form )
  , mMode( DefaultMode )
  , mMultiEditButton( new QgsMultiEditToolButton() )
  , mBlockValueUpdate( false )
  , mIsMixed( false )
  , mIsChanged( false )
{
  mConstraintResultLabel = new QLabel( this );
  mConstraintResultLabel->setObjectName( QStringLiteral( "ConstraintStatus" ) );
  mConstraintResultLabel->setSizePolicy( QSizePolicy::Fixed, mConstraintResultLabel->sizePolicy().verticalPolicy() );

  mEditPage = new QWidget();
  QHBoxLayout *l = new QHBoxLayout();
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
  connect( mSearchWidgetToolButton, &QgsSearchWidgetToolButton::activeFlagsChanged,
           this, &QgsAttributeFormEditorWidget::searchWidgetFlagsChanged );
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

void QgsAttributeFormEditorWidget::createSearchWidgetWrappers( const QString &widgetId, int fieldIdx, const QVariantMap &config,  const QgsAttributeEditorContext &context )
{
  QgsSearchWidgetWrapper *sww = QgsGui::editorWidgetRegistry()->createSearchWidget( widgetId, layer(), fieldIdx, config,
                                mSearchFrame, context );
  setSearchWidgetWrapper( sww );
  if ( sww->supportedFlags() & QgsSearchWidgetWrapper::Between ||
       sww->supportedFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // create secondary widget for between type searches
    QgsSearchWidgetWrapper *sww2 = QgsGui::editorWidgetRegistry()->createSearchWidget( widgetId, layer(), fieldIdx, config,
                                   mSearchFrame, context );
    mSearchWidgets << sww2;
    mSearchFrame->layout()->addWidget( sww2->widget() );
    sww2->widget()->hide();
  }
}

void QgsAttributeFormEditorWidget::setSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper )
{
  mSearchWidgets.clear();
  mSearchWidgets << wrapper;
  mSearchFrame->layout()->addWidget( wrapper->widget() );
  mSearchWidgetToolButton->setAvailableFlags( wrapper->supportedFlags() );
  mSearchWidgetToolButton->setActiveFlags( QgsSearchWidgetWrapper::FilterFlags() );
  mSearchWidgetToolButton->setDefaultFlags( wrapper->defaultFlags() );
  connect( wrapper, &QgsSearchWidgetWrapper::valueChanged, mSearchWidgetToolButton, &QgsSearchWidgetToolButton::setActive );
  connect( wrapper, &QgsSearchWidgetWrapper::valueCleared, mSearchWidgetToolButton, &QgsSearchWidgetToolButton::setInactive );
}

QWidget *QgsAttributeFormEditorWidget::searchWidgetFrame()
{
  return mSearchFrame;
}

QList< QgsSearchWidgetWrapper * > QgsAttributeFormEditorWidget::searchWidgetWrappers()
{
  return mSearchWidgets;
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

void QgsAttributeFormEditorWidget::setMode( QgsAttributeFormEditorWidget::Mode mode )
{
  mMode = mode;
  updateWidgets();
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

void QgsAttributeFormEditorWidget::resetSearch()
{
  mSearchWidgetToolButton->setInactive();
  Q_FOREACH ( QgsSearchWidgetWrapper *widget, mSearchWidgets )
  {
    widget->clearWidget();
  }
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
    return QStringLiteral( "%1 AND %2" ).arg( filter1, filter2 );
  }
  else if ( mSearchWidgetToolButton->activeFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // special case: Is Not Between search
    QString filter1 = mSearchWidgets.at( 0 )->createExpression( QgsSearchWidgetWrapper::LessThan );
    QString filter2 = mSearchWidgets.at( 1 )->createExpression( QgsSearchWidgetWrapper::GreaterThan );
    return QStringLiteral( "%1 OR %2" ).arg( filter1, filter2 );
  }

  return mSearchWidgets.at( 0 )->createExpression( mSearchWidgetToolButton->activeFlags() );
}

void QgsAttributeFormEditorWidget::editorWidgetChanged( const QVariant &value )
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
  Q_FOREACH ( QgsSearchWidgetWrapper *widget, mSearchWidgets )
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

QgsSearchWidgetToolButton *QgsAttributeFormEditorWidget::searchWidgetToolButton()
{
  return mSearchWidgetToolButton;
}

QgsVectorLayer *QgsAttributeFormEditorWidget::layer()
{
  return mForm ? mForm->layer() : nullptr;
}

void QgsAttributeFormEditorWidget::updateWidgets()
{
  //first update the tool buttons
  bool hasMultiEditButton = ( mEditPage->layout()->indexOf( mMultiEditButton ) >= 0 );
  bool fieldReadOnly = layer()->editFormConfig().readOnly( mWidget->fieldIdx() );

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

      mEditPage->layout()->addWidget( mConstraintResultLabel );

      break;
    }

    case SearchMode:
    {
      mStack->setCurrentWidget( mSearchPage );
      break;
    }
  }
}
