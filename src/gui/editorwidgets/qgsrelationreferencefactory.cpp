/***************************************************************************
    qgsrelationreferencefactory.cpp
     --------------------------------------
    Date                 : 29.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproject.h"
#include "qgsrelationreferencefactory.h"

#include "qgsrelation.h"
#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsrelationreferenceconfigdlg.h"
#include "qgsrelationreferencesearchwidgetwrapper.h"
#include "qgsrelationreferencewidget.h"

QgsRelationReferenceFactory::QgsRelationReferenceFactory( const QString &name, QgsMapCanvas *canvas, QgsMessageBar *messageBar )
  : QgsEditorWidgetFactory( name )
  , mCanvas( canvas )
  , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper *QgsRelationReferenceFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsRelationReferenceWidgetWrapper( vl, fieldIdx, editor, mCanvas, mMessageBar, parent );
}

QgsSearchWidgetWrapper *QgsRelationReferenceFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsRelationReferenceSearchWidgetWrapper( vl, fieldIdx, mCanvas, parent );
}

QgsEditorConfigWidget *QgsRelationReferenceFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsRelationReferenceConfigDlg( vl, fieldIdx, parent );
}

QHash<const char *, int> QgsRelationReferenceFactory::supportedWidgetTypes()
{
  QHash<const char *, int> map = QHash<const char *, int>();
  map.insert( QgsRelationReferenceWidget::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsRelationReferenceFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  int normalRelationsCount = 0;
  const QList<QgsRelation> relations = vl->referencingRelations( fieldIdx );
  for ( const QgsRelation &rel : relations )
  {
    switch ( rel.type() )
    {
      case Qgis::RelationshipType::Normal:
        normalRelationsCount++;
        break;

      case Qgis::RelationshipType::Generated:
        break;
    }
  }
  // generated relations should not be used for relation reference widget
  return normalRelationsCount > 0 ? 21 /*A bit stronger than the range widget*/ : 5;
}
