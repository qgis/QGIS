/***************************************************************************
    qgsattributeformrelationeditorwidget.cpp
     --------------------------------------
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

#include "qgsattributeformrelationeditorwidget.h"
#include "qgsrelationaggregatesearchwidgetwrapper.h"
#include "qgsattributeform.h"

#include "qgsrelationwidgetwrapper.h"

QgsAttributeFormRelationEditorWidget::QgsAttributeFormRelationEditorWidget( QgsRelationWidgetWrapper *wrapper, QgsAttributeForm *form )
  : QgsAttributeFormWidget( wrapper, form )
  , mWrapper( wrapper )
{
  setSearchWidgetToolButtonVisible( false );
}

void QgsAttributeFormRelationEditorWidget::createSearchWidgetWrappers( const QgsAttributeEditorContext &context )
{
  if ( !context.parentContext() )
  {
    mSearchWidget = new QgsRelationAggregateSearchWidgetWrapper( layer(), mWrapper, form() );
    mSearchWidget->setContext( context );

    setSearchWidgetWrapper( mSearchWidget );
  }
}

QString QgsAttributeFormRelationEditorWidget::currentFilterExpression() const
{
  QString filterExpression;
  if ( mSearchWidget )
    filterExpression = mSearchWidget->expression();

  return filterExpression;
}

void QgsAttributeFormRelationEditorWidget::setMultiEditFeatureIds( const QgsFeatureIds &fids )
{
  mWrapper->setMultiEditFeatureIds( fids );
}
