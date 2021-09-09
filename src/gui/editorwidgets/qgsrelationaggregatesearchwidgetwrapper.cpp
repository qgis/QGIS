/***************************************************************************
     qgsrelationaggregatesearchwidget.cpp
     -----------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsrelationaggregatesearchwidgetwrapper.h"
#include "qgsattributeform.h"
#include "qgsrelationwidgetwrapper.h"
#include "qgslogger.h"
#include "qgscollapsiblegroupbox.h"

#include <QLabel>
#include <QGridLayout>

QgsRelationAggregateSearchWidgetWrapper::QgsRelationAggregateSearchWidgetWrapper( QgsVectorLayer *vl, QgsRelationWidgetWrapper *wrapper, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, -1, parent )
  , mWrapper( wrapper )
{
  setContext( mWrapper->context() );
}

QString QgsRelationAggregateSearchWidgetWrapper::expression() const
{
  QString aggregateFilter;

  if ( mAttributeForm )
    aggregateFilter = mAttributeForm->aggregateFilter();

  if ( aggregateFilter.isEmpty() )
    return QStringLiteral( "TRUE" );
  else
    return aggregateFilter;
}

bool QgsRelationAggregateSearchWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsRelationAggregateSearchWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *widget;
  const QgsRelation relation = mWrapper->relation();

  QgsCollapsibleGroupBox *groupBox = new QgsCollapsibleGroupBox( relation.name() );

  if ( !relation.isValid() )
  {
    widget = new QLabel( tr( "Relation not valid" ) );
  }
  else
  {
    mContainerWidget = new QWidget( parent );
    widget = mContainerWidget;
    widget->installEventFilter( this );
  }

  groupBox->setLayout( new QGridLayout() );
  groupBox->layout()->addWidget( widget );

  return groupBox;
}

bool QgsRelationAggregateSearchWidgetWrapper::applyDirectly()
{
  return true;
}

void QgsRelationAggregateSearchWidgetWrapper::setExpression( const QString &value )
{
  Q_UNUSED( value )
  QgsDebugMsg( QStringLiteral( "Not supported" ) );
}

bool QgsRelationAggregateSearchWidgetWrapper::eventFilter( QObject *watched, QEvent *event )
{
  const bool rv = QgsSearchWidgetWrapper::eventFilter( watched, event );
  if ( event->type() == QEvent::Show && !mAttributeForm )
  {
    const QgsAttributeEditorContext subContext = QgsAttributeEditorContext( context(), mWrapper->relation(), QgsAttributeEditorContext::Multiple, QgsAttributeEditorContext::Embed );
    mAttributeForm = new QgsAttributeForm( mWrapper->relation().referencingLayer(), QgsFeature(), subContext, mContainerWidget );
    mAttributeForm->setMode( QgsAttributeEditorContext::AggregateSearchMode );
    QGridLayout *glayout = new QGridLayout();
    mContainerWidget->setLayout( glayout );
    glayout->setContentsMargins( 0, 0, 0, 0 );
    glayout->addWidget( mAttributeForm );
  }
  return rv;
}
