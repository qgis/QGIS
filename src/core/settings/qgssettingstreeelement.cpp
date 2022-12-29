/***************************************************************************
  qgssettingstreeelement.cpp
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

#include "qgssettingstreeelement.h"
#include "qgssettingsentryimpl.h"
#include "qgsexception.h"

#include <QDir>


QgsSettingsTreeElement::~QgsSettingsTreeElement()
{
  if ( mType != Type::Root )
    mParent->unregisterChildElement( this );

  qDeleteAll( mChildrenElements );
  qDeleteAll( mChildrenSettings );
}

QgsSettingsTreeElement *QgsSettingsTreeElement::createRootElement()
{
  QgsSettingsTreeElement *te = new QgsSettingsTreeElement();
  te->mType = Type::Root;
  te->mKey = QString();
  te->mCompleteKey = QStringLiteral( "/" );
  return te;
}

QgsSettingsTreeElement *QgsSettingsTreeElement::createChildElement( const QString &key )
{
  if ( childElement( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child tree element with key '%2'." ).arg( this->key(), key ) );
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  QgsSettingsTreeElement *te = new QgsSettingsTreeElement();
  te->mType = Type::Normal;
  te->init( this, key );
  registerChildElement( te );
  return te;
}

QgsSettingsTreeNamedListElement *QgsSettingsTreeElement::createNamedListElement( const QString &key, const QgsSettingsTreeElement::Options &options )
{
  if ( childElement( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child tree element with key '%2'." ).arg( this->key(), key ) );
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  QgsSettingsTreeNamedListElement *te = new QgsSettingsTreeNamedListElement();
  te->mType = Type::NamedList;
  te->init( this, key );
  te->initNamedList( options );
  registerChildElement( te );
  return te;
}

QgsSettingsTreeElement *QgsSettingsTreeElement::childElement( const QString &key )
{
  QList<QgsSettingsTreeElement *>::iterator it = mChildrenElements.begin();
  for ( ; it != mChildrenElements.end(); ++it )
  {
    if ( ( *it )->key() == key )
      return *it;
  }
  return nullptr;
}

QgsSettingsEntryBase *QgsSettingsTreeElement::childSetting( const QString &key )
{
  const QString testCompleteKey = QStringLiteral( "%1%2" ).arg( mCompleteKey, key );
  QList<QgsSettingsEntryBase *>::iterator it = mChildrenSettings.begin();
  for ( ; it != mChildrenSettings.end(); ++it )
  {
    if ( ( *it )->key() == testCompleteKey )
      return *it;
  }
  return nullptr;
}

void QgsSettingsTreeElement::registerChildSetting( QgsSettingsEntryBase *setting, const QString &key )
{
  if ( childElement( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child tree element with key '%2'." ).arg( this->key(), key ) );
  if ( childSetting( key ) )
    throw QgsSettingsException( QObject::tr( "Settings tree element '%1' already holds a child setting with key '%2'." ).arg( this->key(), key ) );

  mChildrenSettings.append( setting );
}


void QgsSettingsTreeElement::registerChildElement( QgsSettingsTreeElement *element )
{
  mChildrenElements.append( element );
}

void QgsSettingsTreeElement::unregisterChildSetting( QgsSettingsEntryBase *setting, bool deleteSettingValues, const QStringList &parentsNamedItems )
{
  if ( deleteSettingValues )
    setting->remove( parentsNamedItems );

  mChildrenSettings.removeAll( setting );
}

void QgsSettingsTreeElement::unregisterChildElement( QgsSettingsTreeElement *element )
{
  mChildrenElements.removeAll( element );
}

void QgsSettingsTreeElement::init( QgsSettingsTreeElement *parent, const QString &key )
{
  mParent = parent;
  mKey = key;
  mCompleteKey = QDir::cleanPath( QStringLiteral( "%1/%2" ).arg( parent->completeKey(), key ) ) + '/';
}


void QgsSettingsTreeNamedListElement::initNamedList( const QgsSettingsTreeElement::Options &options )
{
  mOptions = options;
  if ( options.testFlag( Option::NamedListSelectedItemSetting ) )
  {
    // this must be done before completing the key
    mSelectedItemSetting = new QgsSettingsEntryString( QStringLiteral( "%1/selected" ).arg( mCompleteKey ), nullptr );
  }

  mNamedElementsCount = mParent->namedElementsCount() + 1;
  mCompleteKey.append( QStringLiteral( "items/%%1/" ).arg( mNamedElementsCount ) );
}

QgsSettingsTreeNamedListElement::~QgsSettingsTreeNamedListElement()
{
  if ( mSelectedItemSetting )
    delete mSelectedItemSetting;
}

const QStringList QgsSettingsTreeNamedListElement::items( const QStringList &parentItems ) const
{
  if ( namedElementsCount() - 1 != parentItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentItems.count(),  namedElementsCount() ) );

  QgsSettings settings;
  switch ( parentItems.count() )
  {
    case 0:
      settings.beginGroup( completeKey() );
      break;
    case 1:
      settings.beginGroup( completeKey().arg( parentItems[0] ) );
      break;
    case 2:
      settings.beginGroup( completeKey().arg( parentItems[0], parentItems[1] ) );
      break;
    case 3:
      settings.beginGroup( completeKey().arg( parentItems[0], parentItems[1], parentItems[2] ) );
      break;
    case 4:
      settings.beginGroup( completeKey().arg( parentItems[0], parentItems[1], parentItems[2], parentItems[3] ) );
      break;
    case 5:
      settings.beginGroup( completeKey().arg( parentItems[0], parentItems[1], parentItems[2], parentItems[3], parentItems[4] ) );
      break;
    default:
      throw QgsSettingsException( QObject::tr( "Current implementation of QgsSettingsTreeNamedListElement::items doesn't handle more than 5 parent named items" ) );
      break;
  }

  return settings.childGroups();
}

void QgsSettingsTreeNamedListElement::setSelectedItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(),  namedElementsCount() ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The  named list element has no option to set the current selected entry." ) );

  mSelectedItemSetting->setValue( item, parentsNamedItems );
}

QString QgsSettingsTreeNamedListElement::selectedItem( const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(),  namedElementsCount() ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The  named list element has no option to set the current selected entry." ) );

  return mSelectedItemSetting->value( parentsNamedItems );
}

void QgsSettingsTreeNamedListElement::deleteItem( const QString &item, const QStringList &parentsNamedItems )
{
  if ( namedElementsCount() - 1 != parentsNamedItems.count() )
    throw QgsSettingsException( QObject::tr( "The number of given parent named items (%1) doesn't match with the number of named items in the key (%2)." ).arg( parentsNamedItems.count(),  namedElementsCount() ) );
  if ( !mOptions.testFlag( Option::NamedListSelectedItemSetting ) )
    throw QgsSettingsException( QObject::tr( "The  named list element has no option to set the current selected entry." ) );

  QString key = completeKey().arg( item );
  QgsSettings().remove( key );
}


