/***************************************************************************
                       qgslayertreecustomnode.cpp
                       --------------------------
    begin                : July 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreecustomnode.h"
#include "moc_qgslayertreecustomnode.cpp"
#include "qgslayertreeutils.h"


QgsLayerTreeCustomNode::QgsLayerTreeCustomNode( const QString &nodeId, const QString &nodeName, bool checked )
  : QgsLayerTreeNode( NodeCustom, checked )
  , mId( nodeId )
  , mName( nodeName.isEmpty() ? nodeId : nodeName )
{
}

QgsLayerTreeCustomNode::QgsLayerTreeCustomNode( const QgsLayerTreeCustomNode &other )
  : QgsLayerTreeNode( other )
  , mId( other.mId )
  , mName( other.mName )
{
}

QString QgsLayerTreeCustomNode::name() const
{
  return mName;
}

void QgsLayerTreeCustomNode::setName( const QString &name )
{
  if ( mName == name )
    return;

  mName = name;
  emit nameChanged( this, name );
}

QgsLayerTreeCustomNode *QgsLayerTreeCustomNode::readXml( const QDomElement &element, const QgsReadWriteContext & ) // cppcheck-suppress duplInheritedMember
{
  if ( element.tagName() != QLatin1String( "layer-tree-custom-node" ) )
    return nullptr;

  const QString nodeId = element.attribute( QStringLiteral( "id" ) );
  const QString name =  element.attribute( QStringLiteral( "name" ) );
  bool checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( QStringLiteral( "checked" ) ) ) != Qt::Unchecked;
  bool isExpanded = ( element.attribute( QStringLiteral( "expanded" ), QStringLiteral( "1" ) ) == QLatin1String( "1" ) );

  QgsLayerTreeCustomNode *customNode = new QgsLayerTreeCustomNode( nodeId, name, checked );
  customNode->setExpanded( isExpanded );

  customNode->readCommonXml( element );

  return customNode;
}

void QgsLayerTreeCustomNode::writeXml( QDomElement &parentElement, const QgsReadWriteContext & )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( QStringLiteral( "layer-tree-custom-node" ) );
  elem.setAttribute( QStringLiteral( "id" ), mId );
  elem.setAttribute( QStringLiteral( "name" ), mName );
  elem.setAttribute( QStringLiteral( "expanded" ), mExpanded ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "checked" ), mChecked ? QStringLiteral( "Qt::Checked" ) : QStringLiteral( "Qt::Unchecked" ) );

  writeCommonXml( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeCustomNode::dump() const
{
  return QStringLiteral( "CUSTOM NODE: %1 checked=%2 expanded=%3 id=%4\n" ).arg( mName ).arg( mChecked ).arg( mExpanded ).arg( mId );
}

QgsLayerTreeCustomNode *QgsLayerTreeCustomNode::clone() const
{
  return new QgsLayerTreeCustomNode( *this );
}

void QgsLayerTreeCustomNode::resolveReferences( const QgsProject *, bool )
{
}
