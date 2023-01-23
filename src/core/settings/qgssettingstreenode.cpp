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
#include "qgssettingsentryimpl.h"
#include "qgsexception.h"

#include <QDir>


QgsSettingsTreeNode::~QgsSettingsTreeNode()
{
  if ( mType != Type::Root )
    mParent->unregisterChildElement( this );

  qDeleteAll( mChildrenElements );
  qDeleteAll( mChildrenSettings );
}

QgsSettingsTreeNode *QgsSettingsTreeNode::createRootElement()
{
  QgsSettingsTreeNode *te = new QgsSettingsTreeNode();
  te->mType = Type::Root;
  te->mKey = QString();
  te->mCompleteKey = QStringLiteral( "/" );
  return te;
}

QgsSettingsTreeNode *QgsSettingsTreeNode::createChildElement( const QString &key )
{
  QgsSettingsTreeNode *te = childElement( key );
  if ( te )
    return te;
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  te = new QgsSettingsTreeNode();
  te->mType = Type::Standard;
  te->init( this, key );
  registerChildElement( te );
  return te;
}

QgsSettingsTreeNamedListNode *QgsSettingsTreeNode::createNamedListElement( const QString &key, const QgsSettingsTreeNode::Options &options )
{
  QgsSettingsTreeNode *nte = childElement( key );
  if ( nte )
  {
    if ( nte->type() == Type::NamedList )
      return dynamic_cast<QgsSettingsTreeNamedListNode *>( nte );
    else
      throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child element with key '%2', but it is not a named list.." ).arg( this->key(), key ) );
  }
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  QgsSettingsTreeNamedListNode *te = new QgsSettingsTreeNamedListNode();
  te->mType = Type::NamedList;
  te->init( this, key );
  te->initNamedList( options );
  registerChildElement( te );
  return te;
}


QgsSettingsTreeNode *QgsSettingsTreeNode::childElement( const QString &key )
{
  QList<QgsSettingsTreeNode *>::const_iterator it = mChildrenElements.constBegin();
  for ( ; it != mChildrenElements.constEnd(); ++it )
  {
    if ( ( *it )->key() == key )
      return *it;
  }
  return nullptr;
}

const QgsSettingsEntryBase *QgsSettingsTreeNode::childSetting( const QString &key )
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
  if ( childElement( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child tree element with key '%2'." ).arg( this->key(), key ) );
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  mChildrenSettings.append( setting );
}


void QgsSettingsTreeNode::registerChildElement( QgsSettingsTreeNode *element )
{
  mChildrenElements.append( element );
}

void QgsSettingsTreeNode::unregisterChildSetting( const QgsSettingsEntryBase *setting, bool deleteSettingValues, const QStringList &parentsNamedItems )
{
  if ( deleteSettingValues )
    setting->remove( parentsNamedItems );

  mChildrenSettings.removeAll( setting );
}

void QgsSettingsTreeNode::unregisterChildElement( QgsSettingsTreeNode *element )
{
  mChildrenElements.removeAll( element );
}

void QgsSettingsTreeNode::init( QgsSettingsTreeNode *parent, const QString &key )
{
  mParent = parent;
  mKey = key;
  mCompleteKey = QDir::cleanPath( QStringLiteral( "%1/%2" ).arg( parent->completeKey(), key ) ) + '/';
}


void QgsSettingsTreeNamedListNode::initNamedList( const QgsSettingsTreeNode::Options &options )
{
  mOptions = options;
  if ( options.testFlag( Option::NamedListSelectedItemSetting ) )
  {
    // this must be done before completing the key
    mSelectedItemSetting = new QgsSettingsEntryString( QStringLiteral( "%1/selected" ).arg( mCompleteKey ), nullptr );
  }

  mNamedElementsCount = mParent->namedElementsCount() + 1;
  mItemsCompleteKey = QStringLiteral( "%1items/" ).arg( mCompleteKey );
  mCompleteKey.append( QStringLiteral( "items/%%1/" ).arg( mNamedElementsCount ) );
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
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the element '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedElementsCount() ) ) );


  const QString completeKeyParam = completeKeyWithNamedItems( mItemsCompleteKey, parentsNamedItems );
  QgsSettings settings;
  settings.beginGroup( completeKeyParam );
  return settings.childGroups( origin );
}

void QgsSettingsTreeNamedListNode::setSelectedItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the element '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedElementsCount() ) ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The named list element '%1' has no option to set the current selected entry." ).arg( mCompleteKey ) );

  mSelectedItemSetting->setValue( item, parentsNamedItems );
}

QString QgsSettingsTreeNamedListNode::selectedItem( const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) for the element '%2' doesn't match with the number of named items in the key (%3)." ).arg( QString::number( parentsNamedItems.count() ), mCompleteKey, QString::number( namedElementsCount() ) ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The named list element '%1' has no option to set the current selected entry." ).arg( mCompleteKey ) );

  return mSelectedItemSetting->value( parentsNamedItems );
}

void QgsSettingsTreeNamedListNode::deleteItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(), namedElementsCount() ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The  named list element has no option to set the current selected entry." ) );

  QStringList args = parentsNamedItems;
  args << item;
  QString key = completeKeyWithNamedItems( mCompleteKey, args );
  QgsSettings().remove( key );
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
