/***************************************************************************
    qgsrelationreferencewidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"


QgsRelationReferenceWidgetWrapper::QgsRelationReferenceWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QgsMapCanvas* canvas, QgsMessageBar* messageBar, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mWidget( NULL )
    , mCanvas( canvas )
    , mMessageBar( messageBar )
{
}

QWidget* QgsRelationReferenceWidgetWrapper::createWidget( QWidget* parent )
{
  QgsRelationReferenceWidget* w = new QgsRelationReferenceWidget( parent );
  w->setSizePolicy( w->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding );
  return w;
}

void QgsRelationReferenceWidgetWrapper::initWidget( QWidget* editor )
{
  QgsRelationReferenceWidget* w = dynamic_cast<QgsRelationReferenceWidget*>( editor );
  if ( !w )
  {
    w = new QgsRelationReferenceWidget( editor );
  }

  mWidget = w;

  mWidget->setEditorContext( context(), mCanvas, mMessageBar );

  bool showForm = config( "ShowForm", true ).toBool();
  bool mapIdent = config( "MapIdentification", false ).toBool();
  bool readOnlyWidget = config( "ReadOnly", false ).toBool();
  bool orderByValue = config( "OrderByValue", false ).toBool();

  mWidget->setEmbedForm( showForm );
  mWidget->setReadOnlySelector( readOnlyWidget );
  mWidget->setAllowMapIdentification( mapIdent );
  mWidget->setOrderByValue( orderByValue );

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config( "Relation" ).toString() );

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext* ctx = &context();
  do
  {
    if ( ctx->relation().name() == relation.name() )
    {
      mWidget->setEmbedForm( false );
      mWidget->setReadOnlySelector( false );
      mWidget->setAllowMapIdentification( false );
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  mWidget->setRelation( relation, config( "AllowNULL" ).toBool() );

  connect( mWidget, SIGNAL( foreignKeyChanged( QVariant ) ), this,  SLOT( foreignKeyChanged( QVariant ) ) );
}

QVariant QgsRelationReferenceWidgetWrapper::value()
{
  if ( !mWidget )
    return QVariant( field().type() );

  QVariant v = mWidget->foreignKey();

  if ( v.isNull() )
  {
    return QVariant( field().type() );
  }
  else
  {
    return v;
  }
}

void QgsRelationReferenceWidgetWrapper::setValue( const QVariant& value )
{
  if ( !mWidget )
    return;

  mWidget->setForeignKey( value );
}

void QgsRelationReferenceWidgetWrapper::setEnabled( bool enabled )
{
  if ( !mWidget )
    return;

  mWidget->setRelationEditable( enabled );
}

void QgsRelationReferenceWidgetWrapper::foreignKeyChanged( QVariant value )
{
  if ( !value.isValid() || value.isNull() )
  {
    value = QVariant( field().type() );
  }
  emit valueChanged( value );
}
