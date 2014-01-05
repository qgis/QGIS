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

#include "qgsrelationeditor.h"

#include <QWidget>

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor, QWidget* parent )
  :  QgsWidgetWrapper( vl, editor, parent )
  , mRelation( relation )
  , mRelationWidget( 0 )
{
}


QWidget* QgsRelationWidgetWrapper::createWidget( QWidget* parent )
{
  return new QWidget( parent );
}

void QgsRelationWidgetWrapper::setFeature( const QgsFeature& feature )
{
  delete ( mRelationWidget );
  mRelationWidget = QgsRelationEditorWidget::createRelationEditor( mRelation, feature, context(), widget() );
  widget()->layout()->addWidget( mRelationWidget );
}

void QgsRelationWidgetWrapper::initWidget(QWidget* editor)
{
  if ( !editor->layout() )
    editor->setLayout( new QGridLayout( editor ) );
}
