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
  connect( mSearchWidgetToolButton, &QgsSearchWidgetToolButton::activeFlagsChanged,
           this, &QgsAttributeFormWidget::searchWidgetFlagsChanged );
  l->addWidget( mSearchWidgetToolButton, 0 );


  mStack = new QStackedWidget;
  mStack->addWidget( mEditPage );
  mStack->addWidget( mSearchPage );

  l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  setLayout( l );
  l->addWidget( mStack );

  if ( !mWidget || !mForm )
    return;

  mEditPage->layout()->addWidget( mWidget->widget() );

  // Respect size policy of embedded widget
  setSizePolicy( mWidget->widget()->sizePolicy() );

  updateWidgets();
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

void QgsAttributeFormWidget::updateWidgets()
{
  switch ( mMode )
  {
    case DefaultMode:
    case MultiEditMode:
      mStack->setCurrentWidget( mEditPage );
      break;

    case SearchMode:
    case AggregateSearchMode:
    {
      mStack->setCurrentWidget( mSearchPage );
      break;
    }
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
