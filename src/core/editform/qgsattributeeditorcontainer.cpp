/***************************************************************************
  qgsattributeeditorcontainer.cpp - QgsAttributeEditorContainer

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorcontainer.h"


QgsAttributeEditorContainer::~QgsAttributeEditorContainer()
{
  qDeleteAll( mChildren );
}

void QgsAttributeEditorContainer::addChildElement( QgsAttributeEditorElement *widget )
{
  mChildren.append( widget );
}

void QgsAttributeEditorContainer::setName( const QString &name )
{
  mName = name;
}

QgsOptionalExpression QgsAttributeEditorContainer::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributeEditorContainer::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  if ( visibilityExpression == mVisibilityExpression )
    return;

  mVisibilityExpression = visibilityExpression;
}

QColor QgsAttributeEditorContainer::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributeEditorContainer::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}

QList<QgsAttributeEditorElement *> QgsAttributeEditorContainer::findElements( QgsAttributeEditorElement::AttributeEditorType type ) const
{
  QList<QgsAttributeEditorElement *> results;

  const auto constMChildren = mChildren;
  for ( QgsAttributeEditorElement *elem : constMChildren )
  {
    if ( elem->type() == type )
    {
      results.append( elem );
    }

    if ( elem->type() == AeTypeContainer )
    {
      QgsAttributeEditorContainer *cont = dynamic_cast<QgsAttributeEditorContainer *>( elem );
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

int QgsAttributeEditorContainer::columnCount() const
{
  return mColumnCount;
}

void QgsAttributeEditorContainer::setColumnCount( int columnCount )
{
  mColumnCount = columnCount;
}

QgsAttributeEditorElement *QgsAttributeEditorContainer::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorContainer *element = new QgsAttributeEditorContainer( name(), parent );

  const auto childElements = children();

  for ( QgsAttributeEditorElement *child : childElements )
  {
    element->addChildElement( child->clone( element ) );
  }
  element->mIsGroupBox = mIsGroupBox;
  element->mColumnCount = mColumnCount;
  element->mVisibilityExpression = mVisibilityExpression;

  return element;
}

void QgsAttributeEditorContainer::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "columnCount" ), mColumnCount );
  elem.setAttribute( QStringLiteral( "groupBox" ), mIsGroupBox ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpressionEnabled" ), mVisibilityExpression.enabled() ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpression" ), mVisibilityExpression->expression() );
  if ( mBackgroundColor.isValid() )
    elem.setAttribute( QStringLiteral( "backgroundColor" ), mBackgroundColor.name( ) );
  const auto constMChildren = mChildren;
  for ( QgsAttributeEditorElement *child : constMChildren )
  {
    QDomDocument doc = elem.ownerDocument();
    elem.appendChild( child->toDomElement( doc ) );
  }
}

void QgsAttributeEditorContainer::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  mBackgroundColor = element.attribute( QStringLiteral( "backgroundColor" ), QString() );
  bool ok;
  int cc = element.attribute( QStringLiteral( "columnCount" ) ).toInt( &ok );
  if ( !ok )
    cc = 0;
  setColumnCount( cc );

  bool isGroupBox = element.attribute( QStringLiteral( "groupBox" ) ).toInt( &ok );
  if ( ok )
    setIsGroupBox( isGroupBox );
  else
    setIsGroupBox( mParent );

  bool visibilityExpressionEnabled = element.attribute( QStringLiteral( "visibilityExpressionEnabled" ) ).toInt( &ok );
  QgsOptionalExpression visibilityExpression;
  if ( ok )
  {
    visibilityExpression.setEnabled( visibilityExpressionEnabled );
    visibilityExpression.setData( QgsExpression( element.attribute( QStringLiteral( "visibilityExpression" ) ) ) );
  }
  setVisibilityExpression( visibilityExpression );

  QDomNodeList childNodeList = element.childNodes();

  for ( int i = 0; i < childNodeList.size(); i++ )
  {
    QDomElement childElem = childNodeList.at( i ).toElement();

    QgsAttributeEditorElement *myElem = create( childElem, layerId, fields, context, this );
    if ( myElem )
      addChildElement( myElem );
  }
}

QString QgsAttributeEditorContainer::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorContainer" );
}

