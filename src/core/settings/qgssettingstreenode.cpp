/***************************************************************************
  qgssettingstreenode.cpp
  --------------------------------------
  Date                 : December 2022
  Copyright            : (C) 2022 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingstreenode.h"
#include "moc_qgssettingstreenode.cpp"
#include "qgssettingsentryimpl.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgssettingsproxy.h"

#include <QDir>

QgsSettingsTreeNode::~QgsSettingsTreeNode()
{
  if ( mType != Qgis::SettingsTreeNodeType::Root )
    mParent->unregisterChildNode( this );

  // do not use qDeleteAll
  // the destructor of QgsSettingsTreeNode and QgsSettingsEntry
  // will call unregister on the parent (see above)
  // and will modify the containers at the same time
  const auto nodes = mChildrenNodes;
  for ( const auto *node : nodes )
    delete node;
  const auto settings = mChildrenSettings;
  for ( const auto *setting : settings )
    delete setting;
}

QgsSettingsTreeNode *QgsSettingsTreeNode::createRootNode()
{
  QgsSettingsTreeNode *te = new QgsSettingsTreeNode();
  te->mType = Qgis::SettingsTreeNodeType::Root;
  te->mKey = QString();
  te->mCompleteKey = QStringLiteral( "/" );
  return te;
}

QgsSettingsTreeNode *QgsSettingsTreeNode::createChildNode( const QString &key )
{
  QgsSettingsTreeNode *te = childNode( key );
  if ( te )
    return te;
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree node '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  te = new QgsSettingsTreeNode();
  te->mType = Qgis::SettingsTreeNodeType::Standard;
  te->init( this, key );
  registerChildNode( te );
  return te;
}

QgsSettingsTreeNamedListNode *QgsSettingsTreeNode::createNamedListNode( const QString &key, const Qgis::SettingsTreeNodeOptions &options )
{
  QgsSettingsTreeNode *nte = childNode( key );
  if ( nte )
  {
    if ( nte->type() == Qgis::SettingsTreeNodeType::NamedList )
      return dynamic_cast<QgsSettingsTreeNamedListNode *>( nte );
    else
      throw QgsSettingsException( QObject::tr( "Settings tree node '%1' already holds a child node with key '%2', but it is not a named list.." ).arg( this->key(), key ) );
  }
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree node '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  QgsSettingsTreeNamedListNode *te = new QgsSettingsTreeNamedListNode();
  te->mType = Qgis::SettingsTreeNodeType::NamedList;
  te->init( this, key );
  te->initNamedList( options );
  registerChildNode( te );
  return te;
}


QgsSettingsTreeNode *QgsSettingsTreeNode::childNode( const QString &key ) const
{
  QList<QgsSettingsTreeNode *>::const_iterator it = mChildrenNodes.constBegin();
  for ( ; it != mChildrenNodes.constEnd(); ++it )
  {
    if ( ( *it )->key() == key )
      return *it;
  }
  return nullptr;
}

const QgsSettingsEntryBase *QgsSettingsTreeNode::childSetting( const QString &key ) const
{
  const QString testCompleteKey = QStringLiteral( "%1%2" ).arg( mCompleteKey, key );
  QList<const QgsSettingsEntryBase *>::const_iterator it = mChildrenSettings.constBegin();
  for ( ; it != mChildrenSettings.constEnd(); ++it )
  {
    if ( ( *it )->definitionKey() == testCompleteKey )
      return *it;
  }
  return nullptr;
}

void QgsSettingsTreeNode::registerChildSetting( const QgsSettingsEntryBase *setting, const QString &key )
{
  if ( childNode( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree node '%1' already holds a child tree node with key '%2'." ).arg( this->key(), key ) );
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree node '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  mChildrenSettings.append( setting );
}


void QgsSettingsTreeNode::registerChildNode( QgsSettingsTreeNode *node )
{
  mChildrenNodes.append( node );
}

void QgsSettingsTreeNode::unregisterChildSetting( const QgsSettingsEntryBase *setting, bool deleteSettingValues, const QStringList &parentsNamedItems )
{
  if ( deleteSettingValues )
    setting->remove( parentsNamedItems );

  mChildrenSettings.removeAll( setting );
}

void QgsSettingsTreeNode::unregisterChildNode( QgsSettingsTreeNode *node )
{
  mChildrenNodes.removeAll( node );
}

void QgsSettingsTreeNode::init( QgsSettingsTreeNode *parent, const QString &key )
{
  mParent = parent;
  mKey = key;
  mCompleteKey = QDir::cleanPath( QStringLiteral( "%1/%2" ).arg( parent->completeKey(), key ) ) + '/';
}


void QgsSettingsTreeNamedListNode::initNamedList( const Qgis::SettingsTreeNodeOptions &options )
{
  mOptions = options;
  if ( options.testFlag( Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting ) )
  {
    // this must be done before completing the key
    mSelectedItemSetting = new QgsSettingsEntryString( QStringLiteral( "%1/selected" ).arg( mCompleteKey ), nullptr );
  }

  mNamedNodesCount = mParent->namedNodesCount() + 1;
  mItemsCompleteKey = QStringLiteral( "%1items/" ).arg( mCompleteKey );
  mCompleteKey.append( QStringLiteral( "items/%%1/" ).arg( mNamedNodesCount ) );
}

QgsSettingsTreeNamedListNode::~QgsSettingsTreeNamedListNode()
{
  delete mSelectedItemSetting;
}


QStringList QgsSettingsTreeNamedListNode::items( const QStringList &parentsNamedItems ) const
{
  return items( Qgis::SettingsOrigin::Any, parentsNamedItems );
}

QStringList QgsSettingsTreeNamedListNode::items( Qgis::SettingsOrigin origin, const QStringList &parentsNamedItems ) const
{
  if ( namedNodesCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the node '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedNodesCount() ) ) );


  const QString completeKeyParam = completeKeyWithNamedItems( mItemsCompleteKey, parentsNamedItems );
  auto settings = QgsSettings::get();
  settings->beginGroup( completeKeyParam );
  const QStringList res = settings->childGroups( origin );
  settings->endGroup();
  return res;
}

void QgsSettingsTreeNamedListNode::setSelectedItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedNodesCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the node '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedNodesCount() ) ) );
  if ( !mOptions.testFlag( Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The named list node '%1' has no option to set the current selected entry." ).arg( mCompleteKey ) );

  mSelectedItemSetting->setValue( item, parentsNamedItems );
}

QString QgsSettingsTreeNamedListNode::selectedItem( const QStringList &parentsNamedItems )
{
  if ( namedNodesCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the node '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedNodesCount() ) ) );
  if ( !mOptions.testFlag( Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The named list node '%1' has no option to set the current selected entry." ).arg( mCompleteKey ) );

  return mSelectedItemSetting->value( parentsNamedItems );
}

void QgsSettingsTreeNamedListNode::deleteItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedNodesCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(), namedNodesCount() ) );

  QStringList args = parentsNamedItems;
  args << item;
  QString key = completeKeyWithNamedItems( mCompleteKey, args );
  QgsSettings::get()->remove( key );
}

void QgsSettingsTreeNamedListNode::deleteAllItems( const QStringList &parentsNamedItems )
{
  if ( namedNodesCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(), namedNodesCount() ) );

  const QStringList children = items( parentsNamedItems );
  auto settings = QgsSettings::get();
  for ( const QString &child : children )
  {
    QStringList args = parentsNamedItems;
    args << child;
    QString key = completeKeyWithNamedItems( mCompleteKey, args );
    settings->remove( key );
  }
}

QString QgsSettingsTreeNamedListNode::completeKeyWithNamedItems( const QString &key, const QStringList &namedItems ) const
{
  switch ( namedItems.count() )
  {
    case 0:
      return key;
    case 1:
      return key.arg( namedItems[0] );
    case 2:
      return key.arg( namedItems[0], namedItems[1] );
    case 3:
      return key.arg( namedItems[0], namedItems[1], namedItems[2] );
    case 4:
      return key.arg( namedItems[0], namedItems[1], namedItems[2], namedItems[3] );
    case 5:
      return key.arg( namedItems[0], namedItems[1], namedItems[2], namedItems[3], namedItems[4] );
    default:
      throw QgsSettingsException( QObject::tr( "Current implementation of QgsSettingsTreeNamedListNode::items doesn't handle more than 5 parent named items" ) );
      break;
  }
}
