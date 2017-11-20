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
  Q_UNUSED( context )
  mSearchWidget = new QgsRelationAggregateSearchWidgetWrapper( layer(), mWrapper, form() );

  setSearchWidgetWrapper( mSearchWidget );
}

QString QgsAttributeFormRelationEditorWidget::currentFilterExpression() const
{
  return mSearchWidget->expression();
}
