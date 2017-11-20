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
  QgsRelation relation = mWrapper->relation();

  QgsCollapsibleGroupBox *groupBox = new QgsCollapsibleGroupBox( relation.name() );

  if ( !relation.isValid() )
  {
    widget = new QLabel( tr( "Relation not valid" ) );
  }
  else
  {
    QgsAttributeEditorContext subContext = QgsAttributeEditorContext( context(), mWrapper->relation(), QgsAttributeEditorContext::Multiple, QgsAttributeEditorContext::Embed );
    mAttributeForm = new QgsAttributeForm( mWrapper->relation().referencingLayer(), QgsFeature(), subContext, parent );
    mAttributeForm->setMode( QgsAttributeForm::AggregateSearchMode );
    widget = mAttributeForm;
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
  QgsDebugMsg( "Not supported" );
}
