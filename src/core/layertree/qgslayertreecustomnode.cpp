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

#include "qgslayertreeutils.h"

#include "moc_qgslayertreecustomnode.cpp"

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
  if ( element.tagName() != "layer-tree-custom-node"_L1 )
    return nullptr;

  const QString nodeId = element.attribute( u"id"_s );
  const QString name =  element.attribute( u"name"_s );
  bool checked = QgsLayerTreeUtils::checkStateFromXml( element.attribute( u"checked"_s ) ) != Qt::Unchecked;

  QgsLayerTreeCustomNode *customNode = new QgsLayerTreeCustomNode( nodeId, name, checked );
  customNode->readCommonXml( element );

  return customNode;
}

void QgsLayerTreeCustomNode::writeXml( QDomElement &parentElement, const QgsReadWriteContext & )
{
  QDomDocument doc = parentElement.ownerDocument();
  QDomElement elem = doc.createElement( u"layer-tree-custom-node"_s );
  elem.setAttribute( u"id"_s, mId );
  elem.setAttribute( u"name"_s, mName );
  elem.setAttribute( u"checked"_s, mChecked ? u"Qt::Checked"_s : u"Qt::Unchecked"_s );

  writeCommonXml( elem );

  parentElement.appendChild( elem );
}

QString QgsLayerTreeCustomNode::dump() const
{
  return u"CUSTOM NODE: %1 checked=%2 id=%3\n"_s.arg( mName ).arg( mChecked ).arg( mId );
}

QgsLayerTreeCustomNode *QgsLayerTreeCustomNode::clone() const
{
  return new QgsLayerTreeCustomNode( *this );
}

void QgsLayerTreeCustomNode::resolveReferences( const QgsProject *, bool )
{
}
