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

#include "qgsrelationreferencefactory.h"

#include "qgsproject.h"
#include "qgsrelation.h"
#include "qgsrelationreferenceconfigdlg.h"
#include "qgsrelationreferencesearchwidgetwrapper.h"
#include "qgsrelationreferencewidget.h"
#include "qgsrelationreferencewidgetwrapper.h"

QgsRelationReferenceFactory::QgsRelationReferenceFactory( const QString &name, QgsMapCanvas *canvas, QgsMessageBar *messageBar, const QIcon &icon )
  : QgsEditorWidgetFactory( name, icon )
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
  bool isFirstFieldInRelation = false;
  const QList<QgsRelation> relations = vl->referencingRelations( fieldIdx );
  for ( const QgsRelation &rel : relations )
  {
    switch ( rel.type() )
    {
      case Qgis::RelationshipType::Normal:
      {
        // For composite foreign keys, only the first referencing field should
        // get a RelationReference widget. The other fields are managed by the
        // widget internally and should not show a separate RelationReference
        // widget (see https://github.com/qgis/QGIS/issues/32048).
        const QList<QgsRelation::FieldPair> fieldPairs = rel.fieldPairs();
        if ( !fieldPairs.isEmpty() )
        {
          const int firstReferencingFieldIdx = vl->fields().lookupField( fieldPairs.at( 0 ).referencingField() );
          if ( firstReferencingFieldIdx == fieldIdx )
          {
            isFirstFieldInRelation = true;
          }
        }
        break;
      }

      case Qgis::RelationshipType::Generated:
        break;
    }
  }
  // generated relations should not be used for relation reference widget
  // For composite foreign keys, only the first field gets the RelationReference widget
  return isFirstFieldInRelation ? 21 /*A bit stronger than the range widget*/ : 5;
}
