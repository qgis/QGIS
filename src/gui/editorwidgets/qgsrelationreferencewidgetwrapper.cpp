/***************************************************************************
    qgsrelationreferencewidget.cpp
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


QgsRelationReferenceWidgetWrapper::QgsRelationReferenceWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QgsAttributeEditorContext context, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mEditorContext( context )
{
}

QWidget* QgsRelationReferenceWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsRelationReferenceWidget( parent );
}

void QgsRelationReferenceWidgetWrapper::initWidget( QWidget* editor )
{
  QgsRelationReferenceWidget* w = dynamic_cast<QgsRelationReferenceWidget*>( editor );
  if ( !w )
    return;

  mWidget = w;

  mWidget->setEditorContext( mEditorContext );

  mWidget->displayEmbedForm( config( "ShowForm", true ).toBool() );
  connect( mWidget, SIGNAL( showForm() ), this, SLOT( openForm() ) );

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config( "Relation" ).toString() );
  mWidget->setRelation( relation, config( "AllowNULL" ).toBool() );

  connect( mWidget, SIGNAL( relatedFeatureChanged( QgsFeatureId ) ), this, SLOT( relatedFeatureChanged( QgsFeatureId ) ) );
}

QVariant QgsRelationReferenceWidgetWrapper::value()
{
  if ( !mWidget )
    return QVariant();

  return mWidget->relatedFeature();
}

void QgsRelationReferenceWidgetWrapper::setValue( const QVariant& value )
{
  if ( !mWidget )
    return;

  mWidget->setRelatedFeature( value );
}

void QgsRelationReferenceWidgetWrapper::setEnabled( bool enabled )
{
  if ( !mWidget )
    return;

  mWidget->setRelationEditable( enabled );
}

void QgsRelationReferenceWidgetWrapper::relatedFeatureChanged( QgsFeatureId featureId )
{
  emit valueChanged( featureId );
}
