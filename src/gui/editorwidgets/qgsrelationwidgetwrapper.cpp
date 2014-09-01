/***************************************************************************
    qgsrelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationwidgetwrapper.h"

#include "qgsrelationeditorwidget.h"
#include "qgsattributeeditorcontext.h"

#include <QWidget>

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor, QWidget* parent )
    : QgsWidgetWrapper( vl, editor, parent )
    , mRelation( relation )
    , mWidget( NULL )
{
}

QWidget* QgsRelationWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsRelationEditorWidget( parent );
}

void QgsRelationWidgetWrapper::setFeature( const QgsFeature& feature )
{
  if ( mWidget )
    mWidget->setRelationFeature( mRelation, feature );
}

void QgsRelationWidgetWrapper::initWidget( QWidget* editor )
{
  QgsRelationEditorWidget* w = dynamic_cast<QgsRelationEditorWidget*>( editor );

  // if the editor cannot be cast to relation editor, insert a new one
  if ( !w )
  {
    w = new QgsRelationEditorWidget( editor );
  }

  QgsAttributeEditorContext myContext( QgsAttributeEditorContext( context(), mRelation, QgsAttributeEditorContext::EmbedMultiple ) );

  w->setEditorContext( myContext );

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext* ctx = &context();
  do
  {
    if ( ctx->relation().name() == mRelation.name() && ctx->relationMode() == QgsAttributeEditorContext::EmbedMultiple )
    {
      w->setSaveCollapsedState( false );
      w->setCollapsed( true );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  mWidget = w;
}
