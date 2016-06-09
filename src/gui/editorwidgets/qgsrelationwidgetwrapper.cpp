/***************************************************************************
    qgsrelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include "qgsproject.h"

#include <QWidget>

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor, QWidget* parent )
    : QgsWidgetWrapper( vl, editor, parent )
    , mRelation( relation )
    , mWidget( nullptr )
{
}

QWidget* QgsRelationWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsRelationEditorWidget( parent );
}

void QgsRelationWidgetWrapper::setFeature( const QgsFeature& feature )
{
  if ( mWidget && mRelation.isValid() )
    mWidget->setFeature( feature );
}

void QgsRelationWidgetWrapper::setVisible( bool visible )
{
  if ( mWidget )
    mWidget->setVisible( visible );
}

void QgsRelationWidgetWrapper::initWidget( QWidget* editor )
{
  QgsRelationEditorWidget* w = dynamic_cast<QgsRelationEditorWidget*>( editor );

  // if the editor cannot be cast to relation editor, insert a new one
  if ( !w )
  {
    w = new QgsRelationEditorWidget( editor );
    w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    if ( ! editor->layout() )
    {
      editor->setLayout( new QGridLayout() );
    }
    editor->layout()->addWidget( w );
  }

  QgsAttributeEditorContext myContext( QgsAttributeEditorContext( context(), mRelation, QgsAttributeEditorContext::Multiple, QgsAttributeEditorContext::Embed ) );

  w->setEditorContext( myContext );

  QgsRelation nmrel = QgsProject::instance()->relationManager()->relation( config( "nm-rel" ).toString() );

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext* ctx = &context();
  do
  {
    if (( ctx->relation().name() == mRelation.name() && ctx->formMode() == QgsAttributeEditorContext::Embed )
        || ( nmrel.isValid() && ctx->relation().name() == nmrel.name() ) )
    {
      w->setVisible( false );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );


  w->setRelations( mRelation, nmrel );

  mWidget = w;
}

bool QgsRelationWidgetWrapper::valid() const
{
  return mWidget;
}
