/***************************************************************************
  qgsattributeeditoraction.cpp - QgsAttributeEditorAction

 ---------------------
 begin                : 14.8.2021
 copyright            : (C) 2021 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditoraction.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsactionmanager.h"

QgsAttributeEditorAction::QgsAttributeEditorAction( const QgsAction &action, QgsAttributeEditorElement *parent )
  : QgsAttributeEditorElement( AeTypeAction, action.id().toString(), parent )
  , mAction( action )
{}

QgsAttributeEditorAction::QgsAttributeEditorAction( const QUuid &uuid, const QString &layerId, QgsAttributeEditorElement *parent )
  : QgsAttributeEditorAction( QgsAction(), parent )
{
  mUuid = uuid;
  mLayerId = layerId;
}

QgsAttributeEditorElement *QgsAttributeEditorAction::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorAction *element = new QgsAttributeEditorAction( mAction, parent );
  element->mLayerId = mLayerId;
  element->mUuid = mUuid;
  return element;
}

const QgsAction &QgsAttributeEditorAction::action() const
{
  // Lazy loading
  if ( ! mAction.isValid() && ! mUuid.isNull() && ! mLayerId.isEmpty() )
  {
    if ( const QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( mLayerId ) ); layer )
    {
      mAction = layer->actions()->action( mUuid );
    }
  }
  return mAction;
}

void QgsAttributeEditorAction::setAction( const QgsAction &newAction )
{
  mUuid = newAction.id();
  mAction = newAction;
}

const QString QgsAttributeEditorAction::actionUUID() const
{
  return action().isValid() ? action().id().toString( ) : QString( );
}

const QString QgsAttributeEditorAction::actionTitle() const
{
  if ( action().isValid() )
  {
    return action().shortTitle().isEmpty() ? action().name() : action().shortTitle();
  }
  else
  {
    return QString();
  }
}

QString QgsAttributeEditorAction::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorAction" );
}

void QgsAttributeEditorAction::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "ActionUUID" ), mAction.id().toString() );
}

void QgsAttributeEditorAction::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( context )
  Q_UNUSED( fields )
  mUuid = element.attribute( QStringLiteral( "ActionUUID" ) );
  mLayerId = layerId;
}
