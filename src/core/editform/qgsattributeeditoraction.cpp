/***************************************************************************
  qgsattributeeditoraction.cpp - QgsAttributeEditorAction

 ---------------------
 begin                : 14.8.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
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
  , mUuid( action.id() )
{}

QgsAttributeEditorAction::QgsAttributeEditorAction( const QUuid &uuid, QgsAttributeEditorElement *parent )
  : QgsAttributeEditorAction( QgsAction(), parent )
{
  mUuid = uuid;
}

QgsAttributeEditorElement *QgsAttributeEditorAction::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorAction *element = new QgsAttributeEditorAction( mAction, parent );
  element->mUuid = mUuid;
  return element;
}

const QgsAction &QgsAttributeEditorAction::action( const QgsVectorLayer *layer ) const
{
  // Lazy loading
  if ( ! mAction.isValid() && ! mUuid.isNull() && layer )
  {
    mAction = layer->actions()->action( mUuid );
  }
  return mAction;
}

void QgsAttributeEditorAction::setAction( const QgsAction &newAction )
{
  mUuid = newAction.id();
  mAction = newAction;
}

QString QgsAttributeEditorAction::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorAction" );
}

void QgsAttributeEditorAction::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "ActionUUID" ), mUuid.toString() );
}

void QgsAttributeEditorAction::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
  mUuid = QUuid( element.attribute( QStringLiteral( "ActionUUID" ) ) );
}
