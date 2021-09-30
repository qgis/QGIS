/***************************************************************************
    QgsDatabaseQueryLoggernode.cpp
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsqueryloggernode.h"
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
// QgsDatabaseQueryLoggerNode
//

QgsDatabaseQueryLoggerNode::QgsDatabaseQueryLoggerNode() = default;
QgsDatabaseQueryLoggerNode::~QgsDatabaseQueryLoggerNode() = default;


//
// QgsDatabaseQueryLoggerGroup
//

QgsDatabaseQueryLoggerGroup::QgsDatabaseQueryLoggerGroup( const QString &title )
  : mGroupTitle( title )
{
}

void QgsDatabaseQueryLoggerGroup::addChild( std::unique_ptr<QgsDatabaseQueryLoggerNode> child )
{
  if ( !child )
    return;

  Q_ASSERT( !child->mParent );
  child->mParent = this;

  mChildren.emplace_back( std::move( child ) );
}

int QgsDatabaseQueryLoggerGroup::indexOf( QgsDatabaseQueryLoggerNode *child ) const
{
  Q_ASSERT( child->mParent == this );
  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsDatabaseQueryLoggerNode> &p )
  {
    return p.get() == child;
  } );
  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

QgsDatabaseQueryLoggerNode *QgsDatabaseQueryLoggerGroup::childAt( int index )
{
  Q_ASSERT( static_cast< std::size_t >( index ) < mChildren.size() );
  return mChildren[ index ].get();
}

void QgsDatabaseQueryLoggerGroup::clear()
{
  mChildren.clear();
}

QVariant QgsDatabaseQueryLoggerGroup::data( int role ) const
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

QVariant QgsDatabaseQueryLoggerGroup::toVariant() const
{
  QVariantMap res;
  for ( const std::unique_ptr< QgsDatabaseQueryLoggerNode > &child : mChildren )
  {
    if ( const QgsDatabaseQueryLoggerValueNode *valueNode = dynamic_cast< const QgsDatabaseQueryLoggerValueNode *>( child.get() ) )
    {
      res.insert( valueNode->key(), valueNode->value() );
    }
  }
  return res;
}

//
// QgsDatabaseQueryLoggerRootNode
//

QgsDatabaseQueryLoggerRootNode::QgsDatabaseQueryLoggerRootNode()
  : QgsDatabaseQueryLoggerGroup( QString() )
{

}

QVariant QgsDatabaseQueryLoggerRootNode::data( int ) const
{
  return QVariant();
}

void QgsDatabaseQueryLoggerRootNode::removeRow( int row )
{
  mChildren.erase( mChildren.begin() + row );
}

QVariant QgsDatabaseQueryLoggerRootNode::toVariant() const
{
  QVariantList res;
  for ( const std::unique_ptr< QgsDatabaseQueryLoggerNode > &child : mChildren )
    res << child->toVariant();
  return res;
}


//
// QgsDatabaseQueryLoggerValueNode
//
QgsDatabaseQueryLoggerValueNode::QgsDatabaseQueryLoggerValueNode( const QString &key, const QString &value, const QColor &color )
  : mKey( key )
  , mValue( value )
  , mColor( color )
{

}

QVariant QgsDatabaseQueryLoggerValueNode::data( int role ) const
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

QList<QAction *> QgsDatabaseQueryLoggerValueNode::actions( QObject *parent )
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
// QgsDatabaseQueryLoggerGroup
//

void QgsDatabaseQueryLoggerGroup::addKeyValueNode( const QString &key, const QString &value, const QColor &color )
{
  addChild( std::make_unique< QgsDatabaseQueryLoggerValueNode >( key, value, color ) );
}


QList<QAction *> QgsDatabaseQueryLoggerNode::actions( QObject * )
{
  return QList< QAction * >();
}

QVariant QgsDatabaseQueryLoggerNode::toVariant() const
{
  return QVariant();
}
