/***************************************************************************
  qgsattributeeditorelement.cpp - QgsAttributeEditorElement

 ---------------------
 begin                : 18.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditorelement.h"
#include "qgsrelationmanager.h"

QDomElement QgsAttributeEditorContainer::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorContainer" );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "columnCount", mColumnCount );
  elem.setAttribute( "groupBox", mIsGroupBox ? 1 : 0 );

  Q_FOREACH ( QgsAttributeEditorElement* child, mChildren )
  {
    elem.appendChild( child->toDomElement( doc ) );
  }
  return elem;
}

void QgsAttributeEditorContainer::addChildElement( QgsAttributeEditorElement *widget )
{
  mChildren.append( widget );
}

void QgsAttributeEditorContainer::setName( const QString& name )
{
  mName = name;
}

QList<QgsAttributeEditorElement*> QgsAttributeEditorContainer::findElements( QgsAttributeEditorElement::AttributeEditorType type ) const
{
  QList<QgsAttributeEditorElement*> results;

  Q_FOREACH ( QgsAttributeEditorElement* elem, mChildren )
  {
    if ( elem->type() == type )
    {
      results.append( elem );
    }

    if ( elem->type() == AeTypeContainer )
    {
      QgsAttributeEditorContainer* cont = dynamic_cast<QgsAttributeEditorContainer*>( elem );
      if ( cont )
        results += cont->findElements( type );
    }
  }

  return results;
}

void QgsAttributeEditorContainer::clear()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

QDomElement QgsAttributeEditorField::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorField" );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "index", mIdx );
  return elem;
}

QgsAttributeEditorElement* QgsAttributeEditorField::clone( QgsAttributeEditorElement* parent ) const
{
  QgsAttributeEditorField* element = new QgsAttributeEditorField( name(), mIdx, parent );

  return element;
}

QDomElement QgsAttributeEditorRelation::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorRelation" );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "relation", mRelation.id() );
  return elem;
}

bool QgsAttributeEditorRelation::init( QgsRelationManager* relationManager )
{
  mRelation = relationManager->relation( mRelationId );
  return mRelation.isValid();
}

QgsAttributeEditorElement* QgsAttributeEditorRelation::clone( QgsAttributeEditorElement* parent ) const
{
  QgsAttributeEditorRelation* element = new QgsAttributeEditorRelation( name(), mRelationId, parent );
  element->mRelation = mRelation;

  return element;
}
