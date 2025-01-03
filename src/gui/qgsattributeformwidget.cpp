/***************************************************************************
    qgsattributeformwidget.cpp
    ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeformwidget.h"
#include "moc_qgsattributeformwidget.cpp"
#include <QHBoxLayout>
#include <QStackedWidget>

#include "qgsattributeform.h"
#include "qgssearchwidgettoolbutton.h"

QgsAttributeFormWidget::QgsAttributeFormWidget( QgsWidgetWrapper *widget, QgsAttributeForm *form )
  : QWidget( form )
  , mForm( form )
  , mWidget( widget )
{
  mEditPage = new QWidget();
  QHBoxLayout *l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  mEditPage->setLayout( l );

  l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  mSearchFrame = new QWidget();
  mSearchFrame->setLayout( l );

  mSearchPage = new QWidget();
  l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  mSearchPage->setLayout( l );
  l->addWidget( mSearchFrame, 1 );
  mSearchWidgetToolButton = new QgsSearchWidgetToolButton();
  mSearchWidgetToolButton->setObjectName( QStringLiteral( "SearchWidgetToolButton" ) );
  connect( mSearchWidgetToolButton, &QgsSearchWidgetToolButton::activeFlagsChanged, this, &QgsAttributeFormWidget::searchWidgetFlagsChanged );
  l->addWidget( mSearchWidgetToolButton, 0 );

  mStack = new QStackedWidget();
  // IMPORTANT!
  // We do NOT add pages to mStack here, as QStackedWidgets will always inherit the minimum size
  // of their largest page. This can cause attribute form sizes to needlessly blow out in certain modes,
  // eg when the form is in the "Add feature" mode we do NOT need the extra horizontal space requirements
  // that the search widgets enfore. Doing so forces all editor widgets in all modes to have a very wide
  // minimum width, preventing attribute forms from being shrunk to reasonable sizes without horizontal
  // scroll bars appearing.
  // Instead, the pages are added and removed from the stack whenever the visible page is changed (in updateWidgets()).
  // This ensures that the stack, and this widget too, only inherit the size requirements of the actual visible
  // page.

  l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  setLayout( l );
  l->addWidget( mStack );

  if ( !mWidget || !mForm )
    return;

  mEditPage->layout()->addWidget( mWidget->widget() );

  // Respect size policy of embedded widget
  setSizePolicy( mWidget->widget()->sizePolicy() );

  setVisiblePageForMode( mMode );
}

QgsAttributeFormWidget::~QgsAttributeFormWidget()
{
  // depending on the current page in the stacked widget, these pages NOT
  // be parented to the stacked widget or this widget. Clean them up manually to avoid leaks.
  delete mEditPage;
  mEditPage = nullptr;
  delete mSearchPage;
  mSearchPage = nullptr;
}

void QgsAttributeFormWidget::setMode( QgsAttributeFormWidget::Mode mode )
{
  mMode = mode;
  updateWidgets();
}

QgsAttributeForm *QgsAttributeFormWidget::form() const
{
  return mForm;
}

QWidget *QgsAttributeFormWidget::searchWidgetFrame()
{
  return mSearchFrame;
}

void QgsAttributeFormWidget::setSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper ) SIP_SKIP
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

void QgsAttributeFormWidget::addAdditionalSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper )
{
  mSearchWidgets << wrapper;

  mSearchFrame->layout()->addWidget( wrapper->widget() );
  wrapper->widget()->hide();
}

QList<QgsSearchWidgetWrapper *> QgsAttributeFormWidget::searchWidgetWrappers() SIP_SKIP
{
  return mSearchWidgets;
}

QString QgsAttributeFormWidget::currentFilterExpression() const
{
  if ( mSearchWidgets.isEmpty() )
    return QString();

  if ( !mSearchWidgetToolButton->isActive() )
    return QString();

  if ( mSearchWidgetToolButton->activeFlags() & QgsSearchWidgetWrapper::Between )
  {
    // special case: Between search
    const QString filter1 = mSearchWidgets.at( 0 )->createExpression( QgsSearchWidgetWrapper::GreaterThanOrEqualTo );
    const QString filter2 = mSearchWidgets.at( 1 )->createExpression( QgsSearchWidgetWrapper::LessThanOrEqualTo );
    return QStringLiteral( "%1 AND %2" ).arg( filter1, filter2 );
  }
  else if ( mSearchWidgetToolButton->activeFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // special case: Is Not Between search
    const QString filter1 = mSearchWidgets.at( 0 )->createExpression( QgsSearchWidgetWrapper::LessThan );
    const QString filter2 = mSearchWidgets.at( 1 )->createExpression( QgsSearchWidgetWrapper::GreaterThan );
    return QStringLiteral( "%1 OR %2" ).arg( filter1, filter2 );
  }

  return mSearchWidgets.at( 0 )->createExpression( mSearchWidgetToolButton->activeFlags() );
}

void QgsAttributeFormWidget::resetSearch()
{
  mSearchWidgetToolButton->setInactive();
  const auto constMSearchWidgets = mSearchWidgets;
  for ( QgsSearchWidgetWrapper *widget : constMSearchWidgets )
  {
    widget->clearWidget();
  }
}

QgsVectorLayer *QgsAttributeFormWidget::layer()
{
  return mWidget->layer();
}

void QgsAttributeFormWidget::searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags )
{
  const auto constMSearchWidgets = mSearchWidgets;
  for ( QgsSearchWidgetWrapper *widget : constMSearchWidgets )
  {
    widget->setEnabled( !( flags & QgsSearchWidgetWrapper::IsNull ) && !( flags & QgsSearchWidgetWrapper::IsNotNull ) );
    if ( !mSearchWidgetToolButton->isActive() )
    {
      widget->clearWidget();
    }
  }

  if ( mSearchWidgets.count() >= 2 )
  {
    mSearchWidgets.at( 1 )->widget()->setVisible( flags & QgsSearchWidgetWrapper::Between || flags & QgsSearchWidgetWrapper::IsNotBetween );
  }
}

void QgsAttributeFormWidget::updateWidgets()
{
  setVisiblePageForMode( mMode );
}

void QgsAttributeFormWidget::setVisiblePageForMode( Mode mode )
{
  QWidget *currentVisibleWidget = mStack->currentWidget();

  QWidget *newVisibleWidget = nullptr;
  switch ( mode )
  {
    case DefaultMode:
    case MultiEditMode:
      newVisibleWidget = mEditPage;
      break;

    case SearchMode:
    case AggregateSearchMode:
    {
      newVisibleWidget = mSearchPage;
      break;
    }
  }

  if ( newVisibleWidget != currentVisibleWidget )
  {
    if ( currentVisibleWidget )
    {
      // as per Qt docs, this does NOT delete the page, it just removes it from the stack
      mStack->removeWidget( currentVisibleWidget );
    }

    mStack->addWidget( newVisibleWidget );
    mStack->setCurrentWidget( newVisibleWidget );
  }
}

bool QgsAttributeFormWidget::searchWidgetToolButtonVisible() const
{
  return mSearchWidgetToolButton->isVisible();
}

void QgsAttributeFormWidget::setSearchWidgetToolButtonVisible( bool searchWidgetToolButtonVisible )
{
  mSearchWidgetToolButton->setVisible( searchWidgetToolButtonVisible );
}

QWidget *QgsAttributeFormWidget::searchPage() const
{
  return mSearchPage;
}

QStackedWidget *QgsAttributeFormWidget::stack() const
{
  return mStack;
}

QWidget *QgsAttributeFormWidget::editPage() const
{
  return mEditPage;
}
