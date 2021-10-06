/***************************************************************************
    qgsdevtoolsmodelnode.cpp
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdevtoolsmodelnode.h"
#include "qgis.h"
#include "qgsjsonutils.h"
#include <QUrlQuery>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QAction>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <nlohmann/json.hpp>

//
// QgsDevToolsModelNode
//

QgsDevToolsModelNode::QgsDevToolsModelNode() = default;
QgsDevToolsModelNode::~QgsDevToolsModelNode() = default;

QVariant QgsDevToolsModelNode::toVariant() const
{
  return QVariant();
}

QList<QAction *> QgsDevToolsModelNode::actions( QObject * )
{
  return QList< QAction * >();
}


//
// QgsDevToolsModelGroup
//

QgsDevToolsModelGroup::QgsDevToolsModelGroup( const QString &title )
  : mGroupTitle( title )
{
}

void QgsDevToolsModelGroup::addChild( std::unique_ptr<QgsDevToolsModelNode> child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.emplace_back( std::move( child ) );
}

int QgsDevToolsModelGroup::indexOf( QgsDevToolsModelNode *child ) const
{
  Q_ASSERT( child->mParent == this );
  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsDevToolsModelNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsDevToolsModelNode *QgsDevToolsModelGroup::childAt( int index )
{
  Q_ASSERT( static_cast< std::size_t >( index ) < mChildren.size() );
  return mChildren[ index ].get();
}

void QgsDevToolsModelGroup::clear()
{
  mChildren.clear();
}

QVariant QgsDevToolsModelGroup::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
      return mGroupTitle;

    default:
      break;
  }
  return QVariant();
}

QVariant QgsDevToolsModelGroup::toVariant() const
{
  QVariantMap res;
  for ( const std::unique_ptr< QgsDevToolsModelNode > &child : mChildren )
  {
    if ( const QgsDevToolsModelValueNode *valueNode = dynamic_cast< const QgsDevToolsModelValueNode *>( child.get() ) )
    {
      res.insert( valueNode->key(), valueNode->value() );
    }
  }
  return res;
}


//
// QgsDevToolsModelValueNode
//
QgsDevToolsModelValueNode::QgsDevToolsModelValueNode( const QString &key, const QString &value, const QColor &color )
  : mKey( key )
  , mValue( value )
  , mColor( color )
{

}

QVariant QgsDevToolsModelValueNode::data( int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      return QStringLiteral( "%1: %2" ).arg( mKey.leftJustified( 30, ' ' ), mValue );
    }

    case Qt::ForegroundRole:
    {
      if ( mColor.isValid() )
        return QBrush( mColor );
      break;
    }
    default:
      break;
  }
  return QVariant();
}

QList<QAction *> QgsDevToolsModelValueNode::actions( QObject *parent )
{
  QList< QAction * > res;

  QAction *copyAction = new QAction( QObject::tr( "Copy" ), parent );
  QObject::connect( copyAction, &QAction::triggered, copyAction, [ = ]
  {
    QApplication::clipboard()->setText( QStringLiteral( "%1: %2" ).arg( mKey, mValue ) );
  } );

  res << copyAction;

  return res;
}

//
// QgsDevToolsModelGroup
//

void QgsDevToolsModelGroup::addKeyValueNode( const QString &key, const QString &value, const QColor &color )
{
  addChild( std::make_unique< QgsDevToolsModelValueNode >( key, value, color ) );
}

