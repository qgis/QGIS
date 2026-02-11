/***************************************************************************
               qgscustomization.cpp  - Customization
                             -------------------
    begin                : 2011-04-01
                           2025-12-10 (heavily refactored)
    copyright            : (C) 2011 Radim Blazek
                           (C) 2025 Julien Cabieces
    email                : radim dot blazek at gmail dot com
                           julien dot cabieces at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscustomization.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsbrowserdockwidget.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsgui.h"
#include "qgslogger.h"
#include "qgsstatusbar.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStatusBar>
#include <QString>
#include <QToolButton>
#include <QWidgetAction>

using namespace Qt::StringLiterals;

#define CUSTOMIZATION_CURRENT_VERSION "1"
#define USER_MENU_PROPERTY "__usermenu__"
#define USER_TOOLBAR_PROPERTY "__usertoolbar__"

QgsCustomization::QgsItem::QgsItem( QgsCustomization::QgsItem *parent )
  : mParent( parent )
{
}

QgsCustomization::QgsItem::QgsItem( const QString &name, const QString &title, QgsItem *parent )
  : mName( name )
  , mTitle( title )
  , mParent( parent )
{}

QgsCustomization::QgsItem::~QgsItem() = default;

const QString &QgsCustomization::QgsItem::name() const
{
  return mName;
}

const QString &QgsCustomization::QgsItem::title() const
{
  return mTitle;
}

void QgsCustomization::QgsItem::setTitle( const QString &title )
{
  mTitle = title;
}

QgsCustomization::QgsItem *QgsCustomization::QgsItem::parent() const
{
  return mParent;
}

bool QgsCustomization::QgsItem::isVisible() const
{
  return mVisible;
}

void QgsCustomization::QgsItem::setVisible( bool isVisible )
{
  mVisible = isVisible;
}

void QgsCustomization::QgsItem::setIcon( const QIcon &icon )
{
  mIcon = icon;
}

QIcon QgsCustomization::QgsItem::icon() const
{
  return mIcon;
}

void QgsCustomization::QgsItem::addChild( std::unique_ptr<QgsItem> item )
{
  if ( mChildItems.contains( item->name() ) )
  {
    QgsDebugError( "Customization item alread exists" );
    return;
  }

  const QString name = item->name();
  mChildItemList.push_back( std::move( item ) );
  mChildItems[name] = mChildItemList.back().get();
}

QgsCustomization::QgsItem *QgsCustomization::QgsItem::getChild( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mChildItemList.size() ) )
    return nullptr;

  return mChildItemList.at( index ).get();
}

QgsCustomization::QgsItem *QgsCustomization::QgsItem::getChild( const QString &name ) const
{
  return mChildItems.value( name, nullptr );
}

const std::vector<std::unique_ptr<QgsCustomization::QgsItem>> &QgsCustomization::QgsItem::childItemList() const
{
  return mChildItemList;
}

QgsCustomization::QgsItem *QgsCustomization::QgsItem::lastChild() const
{
  return mChildItemList.empty() ? nullptr : mChildItemList.back().get();
}


int QgsCustomization::QgsItem::indexOf( QgsItem *item ) const
{
  const auto it = std::find_if( mChildItemList.cbegin(), mChildItemList.cend(), [item]( const std::unique_ptr<QgsItem> &currentItem ) {
    return currentItem.get() == item;
  } );

  if ( it != mChildItemList.cend() )
    return static_cast<int>( std::distance( mChildItemList.cbegin(), it ) );
  else
    return -1;
}

unsigned int QgsCustomization::QgsItem::childrenCount() const
{
  return mChildItemList.size();
}

void QgsCustomization::QgsItem::insertChild( int position, std::unique_ptr<QgsItem> item )
{
  if ( position < 0 && position >= static_cast<int>( mChildItemList.size() ) )
  {
    QgsDebugError( u"Insert item impossible, invalid position"_s );
    return;
  }

  mChildItemList.insert( std::next( mChildItemList.cbegin(), position ), std::move( item ) );
  QgsItem *pitem = mChildItemList.at( position ).get();
  mChildItems[pitem->name()] = pitem;
}

void QgsCustomization::QgsItem::deleteChild( int position )
{
  if ( position < 0 && position >= static_cast<int>( mChildItemList.size() ) )
  {
    QgsDebugError( u"Delete item impossible, invalid position"_s );
    return;
  }

  mChildItems.take( mChildItemList.at( position )->name() );
  mChildItemList.erase( std::next( mChildItemList.cbegin(), position ) );
}

void QgsCustomization::QgsItem::writeXml( QDomDocument &doc, QDomElement &parent ) const
{
  QDomElement itemElem = doc.createElement( xmlTag() );
  itemElem.setAttribute( u"name"_s, mName );
  itemElem.setAttribute( u"visible"_s, ( mVisible ? "true" : "false" ) );

  writeXmlItem( itemElem );

  for ( const std::unique_ptr<QgsItem> &childItem : mChildItemList )
  {
    childItem->writeXml( doc, itemElem );
  }

  parent.appendChild( itemElem );
}

QString QgsCustomization::QgsItem::readXml( const QDomElement &elem )
{
  mVisible = elem.attribute( u"visible"_s ) == "true"_L1;
  mName = elem.attribute( u"name"_s );
  if ( mName.isEmpty() )
  {
    return QObject::tr( "Invalid XML file : empty name for tag '%1'" ).arg( elem.tagName() );
  }

  readXmlItem( elem );

  for ( QDomElement childElem = elem.firstChildElement(); !childElem.isNull(); childElem = childElem.nextSiblingElement() )
  {
    std::unique_ptr<QgsItem> childItem = createChildItem( childElem );
    if ( !childItem )
    {
      return QObject::tr( "Invalid XML file : failed to create an item '%1(%2)' as a child of item '%3(%4)'" )
        .arg( childElem.tagName() )
        .arg( childElem.attribute( u"name"_s ) )
        .arg( xmlTag() )
        .arg( mName );
    }
    childItem->readXml( childElem );
    addChild( std::move( childItem ) );
  }

  return QString();
}

bool QgsCustomization::QgsItem::hasCapability( QgsCustomization::QgsItem::ItemCapability capability ) const
{
  return static_cast<int>( capabilities() ) & static_cast<int>( capability );
}

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsItem::createChildItem( const QDomElement & )
{
  return nullptr;
}

void QgsCustomization::QgsItem::copyItemAttributes( const QgsCustomization::QgsItem *other )
{
  mName = other->mName;
  mTitle = other->mTitle;
  mVisible = other->mVisible;
  mIcon = other->mIcon;
  for ( const std::unique_ptr<QgsCustomization::QgsItem> &otherChildItem : other->mChildItemList )
  {
    addChild( otherChildItem->clone( this ) );
  }
}

void QgsCustomization::QgsItem::writeXmlItem( QDomElement & ) const {
};

void QgsCustomization::QgsItem::readXmlItem( const QDomElement & ) {
};

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsItem::capabilities() const
{
  return ItemCapability::None;
}

////////////////

QgsCustomization::QgsActionItem::QgsActionItem( QgsCustomization::QgsItem *parent )
  : QgsCustomization::QgsItem( parent )
{
}


QgsCustomization::QgsActionItem::QgsActionItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsItem( name, title, parent )
{}

QString QgsCustomization::QgsActionItem::xmlTag() const
{
  return u"Action"_s;
};

void QgsCustomization::QgsActionItem::setQAction( QAction *qaction, qsizetype qActionIndex )
{
  mQAction = qaction;
  mQActionIndex = qActionIndex;
}

QAction *QgsCustomization::QgsActionItem::qAction() const
{
  return mQAction;
}

qsizetype QgsCustomization::QgsActionItem::qActionIndex() const
{
  return mQActionIndex;
}

QString QgsCustomization::QgsActionItem::path() const
{
  QString path = name();

  QgsItem const *currentItem = this;
  while ( ( currentItem = currentItem->parent() ) )
  {
    path.prepend( currentItem->name() + "/" );
  }

  return path;
}

std::unique_ptr<QgsCustomization::QgsActionItem> QgsCustomization::QgsActionItem::cloneActionItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsActionItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsActionItem::createChildItem( const QDomElement &childElem )
{
  // Action with a menu can have child action
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<QgsActionItem>( this );
  else
    return nullptr;
}

void QgsCustomization::QgsActionItem::copyItemAttributes( const QgsItem *other )
{
  QgsItem::copyItemAttributes( other );
  if ( const QgsActionItem *action = dynamic_cast<const QgsActionItem *>( other ) )
  {
    mQAction = action->mQAction;
    mQActionIndex = action->mQActionIndex;
  }
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsActionItem::capabilities() const
{
  return ItemCapability::Drag;
}

////////////////

QgsCustomization::QgsActionRefItem::QgsActionRefItem( QgsItem *parent )
  : QgsActionItem( parent ) {}

QgsCustomization::QgsActionRefItem::QgsActionRefItem( const QString &name, const QString &title, const QString &path, QgsItem *parent )
  : QgsActionItem( name, title, parent )
  , mPath( path ) {}

const QString &QgsCustomization::QgsActionRefItem::actionRefPath() const
{
  return mPath;
}

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsActionRefItem::clone( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsActionRefItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsActionRefItem::xmlTag() const
{
  return u"ActionRef"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsActionRefItem::createChildItem( const QDomElement & )
{
  return nullptr;
}

void QgsCustomization::QgsActionRefItem::readXmlItem( const QDomElement &elem )
{
  mPath = elem.attribute( u"path"_s );
};

void QgsCustomization::QgsActionRefItem::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"path"_s, mPath );
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsActionRefItem::capabilities() const
{
  return ItemCapability::Delete;
};

void QgsCustomization::QgsActionRefItem::copyItemAttributes( const QgsItem *other )
{
  QgsActionItem::copyItemAttributes( other );
  if ( const QgsActionRefItem *action = dynamic_cast<const QgsActionRefItem *>( other ) )
  {
    mPath = action->mPath;
  }
}

////////////////

QgsCustomization::QgsMenuItem::QgsMenuItem( QgsItem *parent )
  : QgsActionItem( parent )
{}
QgsCustomization::QgsMenuItem::QgsMenuItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsActionItem( name, title, parent )
{}

std::unique_ptr<QgsCustomization::QgsMenuItem> QgsCustomization::QgsMenuItem::cloneMenuItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsMenuItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsMenuItem::xmlTag() const
{
  return u"Menu"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsMenuItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<QgsCustomization::QgsActionItem>( this );
  else if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<QgsCustomization::QgsMenuItem>( this );
  else
    return nullptr;
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsMenuItem::capabilities() const
{
  return ItemCapability::None;
}

////////////////

QgsCustomization::QgsUserMenuItem::QgsUserMenuItem( QgsItem *parent )
  : QgsMenuItem( parent )
{}

QgsCustomization::QgsUserMenuItem::QgsUserMenuItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsMenuItem( name, title, parent )
{}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsUserMenuItem::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::AddActionRefChild )
    | static_cast<int>( ItemCapability::AddUserMenuChild )
    | static_cast<int>( ItemCapability::Rename )
    | static_cast<int>( ItemCapability::Delete )
  );
}

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsUserMenuItem::clone( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsUserMenuItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsUserMenuItem::xmlTag() const
{
  return u"UserMenu"_s;
}

void QgsCustomization::QgsUserMenuItem::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"title"_s, title() );
};

void QgsCustomization::QgsUserMenuItem::readXmlItem( const QDomElement &elem )
{
  setTitle( elem.attribute( u"title"_s ) );
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsUserMenuItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ActionRef"_L1 )
    return std::make_unique<QgsActionRefItem>( this );
  else if ( childElem.tagName() == "UserMenu"_L1 )
    return std::make_unique<QgsUserMenuItem>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::QgsToolBarItem::QgsToolBarItem( QgsItem *parent )
  : QgsItem( parent )
{}

QgsCustomization::QgsToolBarItem::QgsToolBarItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsItem( name, title, parent ) {}

void QgsCustomization::QgsToolBarItem::setWasVisible( const bool &wasVisible )
{
  mWasVisible = wasVisible;
}

bool QgsCustomization::QgsToolBarItem::wasVisible() const
{
  return mWasVisible;
}

std::unique_ptr<QgsCustomization::QgsToolBarItem> QgsCustomization::QgsToolBarItem::cloneToolBarItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsToolBarItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsToolBarItem::xmlTag() const
{
  return u"ToolBar"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsToolBarItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<QgsActionItem>( this );
  if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<QgsMenuItem>( this );
  else
    return nullptr;
}

void QgsCustomization::QgsToolBarItem::copyItemAttributes( const QgsItem *other )
{
  QgsItem::copyItemAttributes( other );
  if ( const QgsToolBarItem *tb = dynamic_cast<const QgsToolBarItem *>( other ) )
  {
    mWasVisible = tb->mWasVisible;
  }
}

////////////////

QgsCustomization::QgsUserToolBarItem::QgsUserToolBarItem( QgsItem *parent )
  : QgsToolBarItem( parent )
{}

QgsCustomization::QgsUserToolBarItem::QgsUserToolBarItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsToolBarItem( name, title, parent )
{}

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsUserToolBarItem::clone( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsUserToolBarItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsUserToolBarItem::xmlTag() const
{
  return u"UserToolBar"_s;
}

void QgsCustomization::QgsUserToolBarItem::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"title"_s, title() );
};

void QgsCustomization::QgsUserToolBarItem::readXmlItem( const QDomElement &elem )
{
  setTitle( elem.attribute( u"title"_s ) );
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsUserToolBarItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ActionRef"_L1 )
    return std::make_unique<QgsActionRefItem>( this );
  else
    return nullptr;
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsUserToolBarItem::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::AddActionRefChild )
    | static_cast<int>( ItemCapability::Rename )
    | static_cast<int>( ItemCapability::Delete )
  );
}

////////////////

QgsCustomization::QgsToolBarsItem::QgsToolBarsItem()
  : QgsItem()
{
  mName = "ToolBars";
  setTitle( QObject::tr( "ToolBars" ) );
}

std::unique_ptr<QgsCustomization::QgsToolBarsItem> QgsCustomization::QgsToolBarsItem::cloneToolBarsItem( QgsCustomization::QgsItem * ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsToolBarsItem>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsToolBarsItem::xmlTag() const
{
  return u"ToolBars"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsToolBarsItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ToolBar"_L1 )
    return std::make_unique<QgsToolBarItem>( this );
  else if ( childElem.tagName() == "UserToolBar"_L1 )
    return std::make_unique<QgsUserToolBarItem>( this );
  else
    return nullptr;
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsToolBarsItem::capabilities() const
{
  return ItemCapability::AddUserToolBarChild;
}

////////////////

QgsCustomization::QgsMenusItem::QgsMenusItem()
  : QgsItem()
{
  mName = "Menus";
  setTitle( QObject::tr( "Menus" ) );
}

std::unique_ptr<QgsCustomization::QgsMenusItem> QgsCustomization::QgsMenusItem::cloneMenusItem( QgsCustomization::QgsItem * ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsMenusItem>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsMenusItem::xmlTag() const
{
  return u"Menus"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsMenusItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<QgsMenuItem>( this );
  else if ( childElem.tagName() == "UserMenu"_L1 )
    return std::make_unique<QgsUserMenuItem>( this );
  else
    return nullptr;
}

QgsCustomization::QgsItem::ItemCapability QgsCustomization::QgsMenusItem::capabilities() const
{
  return ItemCapability::AddUserMenuChild;
}

////////////////

QgsCustomization::QgsDockItem::QgsDockItem( QgsItem *parent )
  : QgsItem( parent )
{
}

QgsCustomization::QgsDockItem::QgsDockItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsItem( name, title, parent )
{
}

QString QgsCustomization::QgsDockItem::xmlTag() const
{
  return u"Dock"_s;
};

void QgsCustomization::QgsDockItem::copyItemAttributes( const QgsItem *other )
{
  QgsItem::copyItemAttributes( other );
  if ( const QgsDockItem *dock = dynamic_cast<const QgsDockItem *>( other ) )
  {
    mWasVisible = dock->mWasVisible;
  }
}

void QgsCustomization::QgsDockItem::setWasVisible( const bool &wasVisible )
{
  mWasVisible = wasVisible;
}

bool QgsCustomization::QgsDockItem::wasVisible() const
{
  return mWasVisible;
}

std::unique_ptr<QgsCustomization::QgsDockItem> QgsCustomization::QgsDockItem::cloneDockItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsDockItem>( parent );
  clone->copyItemAttributes( this );
  clone->mWasVisible = mWasVisible;
  return clone;
}

////////////////

QgsCustomization::QgsDocksItem::QgsDocksItem()
  : QgsItem()
{
  mName = "Docks";
  setTitle( QObject::tr( "Docks" ) );
}

std::unique_ptr<QgsCustomization::QgsDocksItem> QgsCustomization::QgsDocksItem::cloneDocksItem( QgsCustomization::QgsItem * ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsDocksItem>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsDocksItem::xmlTag() const
{
  return u"Docks"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsDocksItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Dock"_L1 )
    return std::make_unique<QgsDockItem>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::QgsBrowserElementItem::QgsBrowserElementItem( QgsItem *parent )
  : QgsItem( parent )
{
}

QgsCustomization::QgsBrowserElementItem::QgsBrowserElementItem( const QString &name, const QString &title, QgsItem *parent )
  : QgsItem( name, title, parent )
{
}

std::unique_ptr<QgsCustomization::QgsBrowserElementItem> QgsCustomization::QgsBrowserElementItem::cloneBrowserElementItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsBrowserElementItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}


QString QgsCustomization::QgsBrowserElementItem::xmlTag() const
{
  return u"BrowserItem"_s;
};

////////////////

QgsCustomization::QgsBrowserElementsItem::QgsBrowserElementsItem()
  : QgsItem()
{
  mName = "BrowserItems";
  setTitle( QObject::tr( "Browser" ) );
}

std::unique_ptr<QgsCustomization::QgsBrowserElementsItem> QgsCustomization::QgsBrowserElementsItem::cloneBrowserElementsItem( QgsCustomization::QgsItem * ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsBrowserElementsItem>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsBrowserElementsItem::xmlTag() const
{
  return u"BrowserItems"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsBrowserElementsItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "BrowserItem"_L1 )
    return std::make_unique<QgsBrowserElementItem>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::QgsStatusBarWidgetItem::QgsStatusBarWidgetItem( QgsItem *parent )
  : QgsItem( parent )
{}

QgsCustomization::QgsStatusBarWidgetItem::QgsStatusBarWidgetItem( const QString &name, QgsItem *parent )
  : QgsItem( name, QString(), parent ) {}

std::unique_ptr<QgsCustomization::QgsStatusBarWidgetItem> QgsCustomization::QgsStatusBarWidgetItem::cloneStatusBarWidgetItem( QgsCustomization::QgsItem *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsStatusBarWidgetItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsStatusBarWidgetItem::xmlTag() const
{
  return u"StatusBarWidget"_s;
};

////////////////

QgsCustomization::QgsStatusBarWidgetsItem::QgsStatusBarWidgetsItem()
  : QgsItem()
{
  mName = "StatusBarWidgets";
  setTitle( QObject::tr( "Status Bar" ) );
}

std::unique_ptr<QgsCustomization::QgsStatusBarWidgetsItem> QgsCustomization::QgsStatusBarWidgetsItem::cloneStatusBarWidgetsItem( QgsCustomization::QgsItem * ) const
{
  auto clone = std::make_unique<QgsCustomization::QgsStatusBarWidgetsItem>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::QgsStatusBarWidgetsItem::xmlTag() const
{
  return u"StatusBarWidgets"_s;
};

std::unique_ptr<QgsCustomization::QgsItem> QgsCustomization::QgsStatusBarWidgetsItem::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "StatusBarWidget"_L1 )
    return std::make_unique<QgsStatusBarWidgetItem>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::QgsCustomization( const QString &customizationFile )
  : mCustomizationFile( customizationFile )
{
  const QFileInfo fileInfo( customizationFile );
  // TODO QGIS 5: remove QGIS 3 .ini customization file import logic
  if ( !fileInfo.exists() && fileInfo.absoluteDir().exists( "QGISCUSTOMIZATION3.ini" ) )
  {
    loadOldIniFile( fileInfo.absoluteDir().filePath( "QGISCUSTOMIZATION3.ini" ) );
  }
  else if ( fileInfo.exists() )
  {
    read();
  }
}

void QgsCustomization::setQgisApp( QgisApp *qgisApp )
{
  const bool newApp = mQgisApp != qgisApp;

  mQgisApp = qgisApp;
  if ( newApp )
    load();

  apply();
}

QgsCustomization::~QgsCustomization() = default;

/**
 * Copy constructor
 */
QgsCustomization::QgsCustomization( const QgsCustomization &other )
{
  *this = other;
}

/**
 * Assignment operator
 */
QgsCustomization &QgsCustomization::operator=( const QgsCustomization &other )
{
  if ( this == &other )
    return *this;

  mBrowserItems = other.mBrowserItems->cloneBrowserElementsItem();
  mDocks = other.mDocks->cloneDocksItem();
  mMenus = other.mMenus->cloneMenusItem();
  mStatusBarWidgets = other.mStatusBarWidgets->cloneStatusBarWidgetsItem();
  mToolBars = other.mToolBars->cloneToolBarsItem();
  mEnabled = other.mEnabled;
  mSplashPath = other.mSplashPath;
  mQgisApp = other.mQgisApp;
  mCustomizationFile = other.mCustomizationFile;

  return *this;
}

void QgsCustomization::load()
{
  loadApplicationBrowserItems();
  loadApplicationDocks();
  loadApplicationMenus();
  loadApplicationStatusBarWidgets();
  loadApplicationToolBars();
}

bool QgsCustomization::isEnabled() const
{
  return mEnabled;
}

void QgsCustomization::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

QString QgsCustomization::splashPath() const
{
  return isEnabled() ? mSplashPath : QgsApplication::splashPath();
}

QgsCustomization::QgsBrowserElementsItem *QgsCustomization::browserElementsItem() const
{
  return mBrowserItems.get();
}

QgsCustomization::QgsDocksItem *QgsCustomization::docksItem() const
{
  return mDocks.get();
}

QgsCustomization::QgsMenusItem *QgsCustomization::menusItem() const
{
  return mMenus.get();
}

QgsCustomization::QgsStatusBarWidgetsItem *QgsCustomization::statusBarWidgetsItem() const
{
  return mStatusBarWidgets.get();
}

QgsCustomization::QgsToolBarsItem *QgsCustomization::toolBarsItem() const
{
  return mToolBars.get();
}

void QgsCustomization::addActions( QgsItem *item, QWidget *widget ) const
{
  if ( !item || !widget )
    return;

  for ( QgsQActionsIterator::Info it : QgsQActionsIterator( widget ) )
  {
    if ( it.name.isEmpty() )
      continue;

    // submenu
    QgsActionItem *childItem = item->getChild<QgsActionItem>( it.name );
    if ( !childItem )
    {
      if ( it.isMenu )
      {
        auto menuItem = std::make_unique<QgsMenuItem>( it.name, it.title, item );
        item->addChild( std::move( menuItem ) );
      }
      else
      {
        auto action = std::make_unique<QgsActionItem>( it.name, it.title, item );
        item->addChild( std::move( action ) );
      }

      childItem = item->lastChild<QgsActionItem>();
    }

    childItem->setIcon( it.icon );
    childItem->setTitle( it.title );
    childItem->setQAction( it.action, it.index );
    addActions( childItem, it.widget );
  }
}

void QgsCustomization::loadApplicationToolBars()
{
  if ( !mToolBars )
  {
    mToolBars = std::make_unique<QgsToolBarsItem>();
  }

  const auto toolbars = mQgisApp->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolbars )
  {
    const QString name = tb->objectName();
    if ( name.isEmpty() )
      continue;

    QgsToolBarItem *t = mToolBars->getChild<QgsToolBarItem>( name );
    if ( !t )
    {
      auto toolBar = std::make_unique<QgsToolBarItem>( tb->objectName(), tb->windowTitle(), mToolBars.get() );
      mToolBars->addChild( std::move( toolBar ) );
      t = mToolBars->lastChild<QgsToolBarItem>();
    }

    addActions( t, tb );
    t->setWasVisible( tb->isVisible() );
  }
}

void QgsCustomization::loadApplicationMenus()
{
  if ( !mQgisApp )
    return;

  if ( !mMenus )
  {
    mMenus = std::make_unique<QgsMenusItem>();
  }

  QMenuBar *menuBar = mQgisApp->menuBar();
  addActions( mMenus.get(), menuBar );
}

void QgsCustomization::loadApplicationDocks()
{
  if ( !mQgisApp )
    return;

  if ( !mDocks )
  {
    mDocks = std::make_unique<QgsDocksItem>();
  }

  const auto dockWidgets = mQgisApp->findChildren<QDockWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QDockWidget *dw : dockWidgets )
  {
    const QString name = dw->objectName();
    if ( name.isEmpty() )
      continue;

    QgsDockItem *d = mDocks->getChild<QgsDockItem>( name );
    if ( !d )
    {
      auto dock = std::make_unique<QgsDockItem>( name, dw->windowTitle(), mDocks.get() );
      mDocks->addChild( std::move( dock ) );
      d = mDocks->lastChild<QgsDockItem>();
    }

    d->setWasVisible( dw->isVisible() );
  }
}

void QgsCustomization::loadApplicationBrowserItems()
{
  if ( !mBrowserItems )
  {
    mBrowserItems = std::make_unique<QgsBrowserElementsItem>();
    const QList<QPair<QString, QString>> staticItems = {
      { u"special:Home"_s, QObject::tr( "Home Folder" ) },
      { u"special:ProjectHome"_s, QObject::tr( "Project Home Folder" ) },
      { u"special:Favorites"_s, QObject::tr( "Favorites Folder" ) },
      { u"special:Drives"_s, QObject::tr( "Drive Folders (e.g. C:\\)" ) },
#ifdef Q_OS_MAC
      { u"special:Volumes"_s, QObject::tr( "Volume Folder (MacOS only)" ) }
#endif
    };

    for ( const QPair<QString, QString> &staticItem : staticItems )
    {
      auto browserItem = std::make_unique<QgsBrowserElementItem>( staticItem.first, staticItem.second, mBrowserItems.get() );
      mBrowserItems->addChild( std::move( browserItem ) );
    }
  }

  const auto constProviders = QgsApplication::dataItemProviderRegistry()->providers();
  for ( QgsDataItemProvider *pr : constProviders )
  {
    const Qgis::DataItemProviderCapabilities capabilities = pr->capabilities();
    const QString name = pr->name();
    if ( !name.isEmpty() && capabilities != Qgis::DataItemProviderCapabilities( Qgis::DataItemProviderCapability::NoCapabilities ) )
    {
      if ( !mBrowserItems->getChild<QgsBrowserElementItem>( name ) )
      {
        auto browserItem = std::make_unique<QgsBrowserElementItem>( name, QObject::tr( "Data Item Provider: %1" ).arg( name ), mBrowserItems.get() );
        mBrowserItems->addChild( std::move( browserItem ) );
      }
    }
  }
}

void QgsCustomization::loadApplicationStatusBarWidgets()
{
  if ( !mQgisApp )
    return;

  if ( !mStatusBarWidgets )
  {
    mStatusBarWidgets = std::make_unique<QgsStatusBarWidgetsItem>();
  }

  QgsStatusBar *sb = mQgisApp->statusBarIface();
  const auto children = sb->findChildren<QWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidget *statusBarWidget : children )
  {
    const QString name = statusBarWidget->objectName();
    if ( name.isEmpty() )
      continue;

    QgsStatusBarWidgetItem *s = mStatusBarWidgets->getChild<QgsStatusBarWidgetItem>( name );
    if ( !s )
    {
      auto statusBarWidget = std::make_unique<QgsStatusBarWidgetItem>( name, mStatusBarWidgets.get() );
      mStatusBarWidgets->addChild( std::move( statusBarWidget ) );
    }
  }
}

void QgsCustomization::apply() const
{
  if ( !mEnabled )
    return;

  applyToBrowserItems();
  applyToDocks();
  applyToMenus();
  applyToStatusBarWidgets();
  applyToToolBars();
}

void QgsCustomization::applyToBrowserItems() const
{
  if ( !mQgisApp )
    return;

  QStringList disabledDataItems;
  for ( const std::unique_ptr<QgsItem> &item : mBrowserItems->childItemList() )
  {
    QgsBrowserElementItem *browserItem = dynamic_cast<QgsBrowserElementItem *>( item.get() );
    if ( browserItem && !browserItem->isVisible() )
      disabledDataItems << browserItem->name();
  }

  if ( mQgisApp->browserWidget() )
    mQgisApp->browserWidget()->setDisabledDataItemsKeys( disabledDataItems );

  if ( mQgisApp->browserWidget2() )
    mQgisApp->browserWidget2()->setDisabledDataItemsKeys( disabledDataItems );
}

void QgsCustomization::applyToDocks() const
{
  if ( !mDocks || !mQgisApp )
    return;

  const auto dockWidgets = mQgisApp->findChildren<QDockWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QDockWidget *dw : dockWidgets )
  {
    const QString name = dw->objectName();
    if ( QgsDockItem *d = mDocks->getChild<QgsDockItem>( name ) )
    {
      dw->setVisible( d->wasVisible() && d->isVisible() );
      dw->toggleViewAction()->setVisible( d->isVisible() );
    }
  }
}

QgsCustomization::QgsQActionsIterator::QgsQActionsIterator( QWidget *widget )
  : mWidget( widget ) {};

QgsCustomization::QgsQActionsIterator::Iterator::Iterator( QWidget *ptr, qsizetype idx )
  : mIdx( idx ), mActions( ptr->actions() ) {}

QgsCustomization::QgsQActionsIterator::Info QgsCustomization::QgsQActionsIterator::Iterator::operator*() const
{
  if ( mIdx < 0 || mIdx >= mActions.count() )
    throw std::out_of_range {
      "Action iterator out of range"
    };

  QAction *act = mActions.at( mIdx );
  Info infos;

  // submenu
  if ( QMenu *menu = act->menu() )
  {
    infos.isMenu = true;
    infos.name = menu->objectName();
    infos.title = menu->title().remove( '&' );
    infos.icon = menu->icon();
    infos.widget = menu;
  }
  // ordinary action
  else
  {
    infos.isMenu = false;
    infos.name = act->objectName();
    infos.title = act->text().remove( "&" );
    infos.icon = act->icon();

    QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>( act );
    infos.widget = widgetAction ? widgetAction->defaultWidget() : nullptr;
  }

  infos.action = act;
  infos.index = mIdx;
  return infos;
}

QgsCustomization::QgsQActionsIterator::Iterator &QgsCustomization::QgsQActionsIterator::Iterator::operator++()
{
  mIdx++;
  while ( mIdx < mActions.count() && mActions.at( mIdx )->isSeparator() )
    mIdx++;
  return *this;
}

bool QgsCustomization::QgsQActionsIterator::Iterator::operator==( const Iterator &b ) const
{
  Q_ASSERT( mIdx < 0 || mIdx >= mActions.count() || mActions.at( mIdx ) == b.mActions.at( mIdx ) );
  return mIdx == b.mIdx;
}

QgsCustomization::QgsQActionsIterator::Iterator QgsCustomization::QgsQActionsIterator::begin()
{
  return Iterator( mWidget, 0 );
}

QgsCustomization::QgsQActionsIterator::Iterator QgsCustomization::QgsQActionsIterator::end()
{
  return Iterator( mWidget, mWidget->actions().count() );
}

QWidget *QgsCustomization::findQWidget( const QString &path )
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  QgisApp *app = QgisApp::instance();
  const QHash<QString, QWidget *> rootWidgets = { { "Menus", app->menuBar() }, { "ToolBars", app }, { "Docks", app }, { "StatusBarWidgets", app->statusBarIface() } };

  const QString rootElem = pathElems.takeFirst();
  QWidget *currentWidget = rootWidgets.value( rootElem );
  if ( !currentWidget )
    return nullptr;

  for ( const QString &pathElem : pathElems )
  {
    if ( dynamic_cast<QToolBar *>( currentWidget )
         || dynamic_cast<QMenu *>( currentWidget )
         || dynamic_cast<QMenuBar *>( currentWidget ) )
    {
      QgsQActionsIterator actionsIterator( currentWidget );
      currentWidget = nullptr;
      for ( QgsQActionsIterator::Info it : actionsIterator )
      {
        if ( it.name == pathElem )
        {
          currentWidget = it.widget;
          break;
        }
      }
    }
    else
    {
      QList<QObject *> children = currentWidget->children();
      QList<QObject *>::const_iterator it = std::find_if( children.cbegin(), children.cend(), [&pathElem]( QObject *obj ) { return dynamic_cast<QWidget *>( obj ) && obj->objectName() == pathElem; } );
      currentWidget = it == children.cend() ? nullptr : dynamic_cast<QWidget *>( *it );
    }

    if ( !currentWidget )
      return nullptr;
  }

  return currentWidget;
}

QAction *QgsCustomization::findQAction( const QString &path )
{
  qsizetype lastSlashIndex = path.lastIndexOf( "/" );
  if ( lastSlashIndex < 0 )
    return nullptr;

  QWidget *currentWidget = findQWidget( path.first( lastSlashIndex ) );
  if ( !currentWidget )
    return nullptr;

  const QString actionName = path.mid( lastSlashIndex + 1 );

  const QList<QAction *> actions = currentWidget->actions();
  const QList<QAction *>::const_iterator actionIt = std::find_if( actions.cbegin(), actions.cend(), [&actionName]( QAction *action ) { return action->objectName() == actionName; } );
  return actionIt != actions.cend() ? *actionIt : nullptr;
}

template<class WidgetType>
void QgsCustomization::updateMenuActionVisibility( QgsCustomization::QgsItem *parentItem, WidgetType *parentWidget )
{
  // clear all user menu
  const QList<QAction *> widgetActions = parentWidget->actions();
  for ( QAction *action : widgetActions )
  {
    const QMenu *menu = action->menu();
    if ( menu && menu->property( USER_MENU_PROPERTY ).toBool() )
    {
      parentWidget->removeAction( action );
    }
  }

  // update non-user menu visibility
  updateActionVisibility( parentItem, parentWidget );

  // add user menu
  for ( const std::unique_ptr<QgsCustomization::QgsItem> &childItem : parentItem->childItemList() )
  {
    if ( !childItem->isVisible() )
      continue;

    if ( QgsCustomization::QgsUserMenuItem *userMenu = dynamic_cast<QgsCustomization::QgsUserMenuItem *>( childItem.get() ) )
    {
      QMenu *menu = new QMenu( userMenu->title(), parentWidget );
      menu->setProperty( USER_MENU_PROPERTY, true );
      menu->setObjectName( userMenu->name() );
      parentWidget->addMenu( menu );

      updateMenuActionVisibility( userMenu, menu );
    }
    else if ( QgsCustomization::QgsActionRefItem *actionRef = dynamic_cast<QgsCustomization::QgsActionRefItem *>( childItem.get() ) )
    {
      if ( QAction *action = findQAction( actionRef->actionRefPath() ) )
      {
        parentWidget->addAction( action );
      }
    }
  }
}

void QgsCustomization::updateActionVisibility( QgsCustomization::QgsItem *item, QWidget *widget )
{
  if ( !item || !widget )
    return;

  QSet<QgsCustomization::QgsItem *> processedChildItems;
  for ( QgsQActionsIterator::Info it : QgsQActionsIterator( widget ) )
  {
    if ( QgsCustomization::QgsItem *childItem = item->getChild( it.name ) )
    {
      processedChildItems << childItem;

      if ( !childItem->isVisible() )
      {
        widget->removeAction( it.action );
      }

      if ( QMenu *menu = dynamic_cast<QMenu *>( it.widget ) )
        updateMenuActionVisibility( childItem, menu );
      else
        updateActionVisibility( childItem, it.widget );
    }
  }

  // all have been processed, no need to continue
  if ( static_cast<size_t>( processedChildItems.count() ) == item->childItemList().size() )
    return;

  // Some action have been previously removed and could be visible again. If so, we need to add them again
  int nbRemoved = 0;
  for ( const std::unique_ptr<QgsItem> &childItem : item->childItemList() )
  {
    QgsActionItem *action = dynamic_cast<QgsActionItem *>( childItem.get() );
    if ( !action )
    {
      QgsDebugError( u"Invalid child type, Action expected"_s );
      continue;
    }

    if ( !action->isVisible() )
      nbRemoved++;

    if ( action->qAction() && childItem->isVisible() && !processedChildItems.contains( childItem.get() ) )
    {
      int index = static_cast<int>( action->qActionIndex() ) - nbRemoved;
      if ( index >= 0 && index < widget->actions().count() )
        widget->insertAction( widget->actions().at( index ), action->qAction() );
      else
        widget->addAction( action->qAction() );
    }
  }
}

void QgsCustomization::applyToMenus() const
{
  if ( !mQgisApp )
    return;

  QMenuBar *menuBar = mQgisApp->menuBar();
  updateMenuActionVisibility( mMenus.get(), menuBar );
}

void QgsCustomization::applyToStatusBarWidgets() const
{
  if ( !mQgisApp )
    return;

  QgsStatusBar *sb = mQgisApp->statusBarIface();
  const auto children = sb->findChildren<QWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidget *statusBarWidget : children )
  {
    const QString name = statusBarWidget->objectName();
    if ( name.isEmpty() )
      continue;

    if ( QgsStatusBarWidgetItem *s = mStatusBarWidgets->getChild<QgsStatusBarWidgetItem>( name ) )
    {
      statusBarWidget->setVisible( s->isVisible() );
    }
  }
}

void QgsCustomization::applyToToolBars() const
{
  if ( !mQgisApp )
    return;

  const auto toolBarWidgets = mQgisApp->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolBarWidgets )
  {
    if ( !tb )
      continue;

    if ( tb->property( USER_TOOLBAR_PROPERTY ).toBool() )
    {
      // delete old toolbar, will recreate it later
      QgisApp::instance()->removeToolBar( tb );
      delete tb;
    }
    else if ( QgsToolBarItem *t = mToolBars->getChild<QgsToolBarItem>( tb->objectName() ) )
    {
      tb->setVisible( t->wasVisible() && t->isVisible() );
      tb->toggleViewAction()->setVisible( t->isVisible() );
      updateActionVisibility( t, tb );
    }
  }

  for ( const std::unique_ptr<QgsCustomization::QgsItem> &childItem : toolBarsItem()->childItemList() )
  {
    if ( QgsCustomization::QgsUserToolBarItem *userToolBar = dynamic_cast<QgsCustomization::QgsUserToolBarItem *>( childItem.get() );
         userToolBar && userToolBar->isVisible() )
    {
      QToolBar *toolBar = new QToolBar( userToolBar->title(), QgisApp::instance() );
      toolBar->setProperty( USER_TOOLBAR_PROPERTY, true );
      toolBar->setObjectName( userToolBar->name() );
      QgisApp::instance()->addToolBar( toolBar );

      for ( const std::unique_ptr<QgsCustomization::QgsItem> &actionRefItem : userToolBar->childItemList() )
      {
        if ( QgsCustomization::QgsActionRefItem *actionRef = dynamic_cast<QgsCustomization::QgsActionRefItem *>( actionRefItem.get() ) )
        {
          if ( QAction *action = findQAction( actionRef->actionRefPath() ) )
          {
            toolBar->addAction( action );
          }
        }
      }

      updateActionVisibility( userToolBar, toolBar );
    }
  }
}


QString QgsCustomization::writeFile( const QString &fileName ) const
{
  QDomDocument doc( u"Customization"_s );
  QDomElement root = doc.createElement( u"Customization"_s );
  root.setAttribute( u"version"_s, QStringLiteral( CUSTOMIZATION_CURRENT_VERSION ) );
  root.setAttribute( u"enabled"_s, ( mEnabled ? "true" : "false" ) );

  if ( !mSplashPath.isEmpty() )
    root.setAttribute( u"splashPath"_s, mSplashPath );

  doc.appendChild( root );

  mBrowserItems->writeXml( doc, root );
  mDocks->writeXml( doc, root );
  mMenus->writeXml( doc, root );
  mStatusBarWidgets->writeXml( doc, root );
  mToolBars->writeXml( doc, root );

  QFile f( fileName );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    return QObject::tr( "Error while writing file '%1'" ).arg( fileName );
  }

  QTextStream ts( &f );
  doc.save( ts, 2 );
  f.close();

  return QString();
}

QString QgsCustomization::write() const
{
  return writeFile( mCustomizationFile );
}

QString QgsCustomization::readFile( const QString &fileName )
{
  QDomDocument doc( u"customization"_s );
  QFile f( fileName );
  if ( !f.open( QFile::ReadOnly ) )
  {
    return QObject::tr( "Error opening the XML file" );
  }

  if ( !doc.setContent( &f ) )
  {
    return QObject::tr( "Badly formatted XML file" );
  }
  f.close();

  QDomElement docEl = doc.documentElement();
  if ( docEl.tagName() != "Customization"_L1 )
  {
    return QObject::tr( "Invalid XML file : root tag must be 'Customization'" );
  }

  mEnabled = docEl.attribute( u"enabled"_s ) == "true"_L1;
  if ( docEl.hasAttribute( u"splashPath"_s ) )
    mSplashPath = docEl.attribute( u"splashPath"_s );

  const QString version = docEl.attribute( u"version"_s );
  if ( version != QLatin1String( CUSTOMIZATION_CURRENT_VERSION ) )
  {
    return QObject::tr( "Invalid XML file : incorrect version" );
  }

  mBrowserItems = std::make_unique<QgsBrowserElementsItem>();
  mBrowserItems->readXml( docEl.firstChildElement( u"BrowserItems"_s ) );
  mDocks = std::make_unique<QgsDocksItem>();
  mDocks->readXml( docEl.firstChildElement( u"Docks"_s ) );
  mMenus = std::make_unique<QgsMenusItem>();
  mMenus->readXml( docEl.firstChildElement( u"Menus"_s ) );
  mStatusBarWidgets = std::make_unique<QgsStatusBarWidgetsItem>();
  mStatusBarWidgets->readXml( docEl.firstChildElement( u"StatusBarWidgets"_s ) );
  mToolBars = std::make_unique<QgsToolBarsItem>();
  mToolBars->readXml( docEl.firstChildElement( u"ToolBars"_s ) );

  return QString();
}

void QgsCustomization::read()
{
  ( void ) readFile( mCustomizationFile );
}

void QgsCustomization::loadOldIniFile( const QString &filePath )
{
  // enabled state is in application ini file
  mEnabled = QSettings().value( "UI/Customization/enabled", false ).toBool();

  QSettings settings( filePath, QSettings::IniFormat );
  mSplashPath = settings.value( u"/Customization/splashpath"_s, QgsApplication::splashPath() ).toString();

  mBrowserItems = std::make_unique<QgsBrowserElementsItem>();
  mDocks = std::make_unique<QgsDocksItem>();
  mMenus = std::make_unique<QgsMenusItem>();
  mStatusBarWidgets = std::make_unique<QgsStatusBarWidgetsItem>();
  mToolBars = std::make_unique<QgsToolBarsItem>();

  // menus
  settings.beginGroup( u"Customization/Menus"_s );

  for ( const QString &key : settings.allKeys() )
  {
    QgsItem *rootItem = menusItem();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( QgsItem *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else if ( i < keyElems.count() - 1 ) // Menu
      {
        rootItem->addChild( std::make_unique<QgsMenuItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
      else // Action
      {
        rootItem->addChild( std::make_unique<QgsActionItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  // toolbars
  settings.beginGroup( u"Customization/Toolbars"_s );

  for ( const QString &key : settings.allKeys() )
  {
    QgsItem *rootItem = toolBarsItem();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( QgsItem *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else if ( i == 0 ) // ToolBar
      {
        rootItem->addChild( std::make_unique<QgsToolBarItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
      else // Action
      {
        rootItem->addChild( std::make_unique<QgsActionItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  // dock widgets
  settings.beginGroup( u"Customization/Docks"_s );
  for ( const QString &key : settings.allKeys() )
  {
    QgsItem *rootItem = docksItem();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( QgsItem *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // Dock
      {
        rootItem->addChild( std::make_unique<QgsDockItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  statusBarWidgetsItem()->setVisible( settings.value( "Customization/StatusBar", true ).toBool() );
  settings.beginGroup( u"Customization/StatusBar"_s );

  for ( const QString &key : settings.allKeys() )
  {
    QgsItem *rootItem = statusBarWidgetsItem();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( QgsItem *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // StatusBarWidget
      {
        rootItem->addChild( std::make_unique<QgsStatusBarWidgetItem>( keyElem, rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  settings.beginGroup( u"Customization/Browser"_s );

  for ( const QString &key : settings.allKeys() )
  {
    QgsItem *rootItem = browserElementsItem();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( QgsItem *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // BrowserItem
      {
        rootItem->addChild( std::make_unique<QgsBrowserElementItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();
}

namespace
{

  int maxSuffixNum( const QgsCustomization::QgsItem *item, const QString &baseName )
  {
    int max = 0;

    if ( item->name().startsWith( baseName ) )
    {
      bool ok = false;
      const int suffixNum = item->name().mid( baseName.length() ).toInt( &ok );
      if ( ok )
        max = suffixNum;
    }

    for ( const std::unique_ptr<QgsCustomization::QgsItem> &childItem : item->childItemList() )
    {
      const int childSuffixNum = maxSuffixNum( childItem.get(), baseName );
      max = std::max( childSuffixNum, max );
    }

    return max;
  }

} //namespace

QString QgsCustomization::uniqueItemName( const QString &baseName ) const
{
  // Now, we can only create only new child item for Menus and ToolBars
  // We could have the same name in Menus and ToolBars but it's cleaner to have unique name within the 2

  int suffixNum = maxSuffixNum( mMenus.get(), baseName );
  suffixNum = std::max( suffixNum, maxSuffixNum( mToolBars.get(), baseName ) );

  return QString( "%1%2" ).arg( baseName ).arg( ++suffixNum );
}

QString QgsCustomization::uniqueMenuName() const
{
  return uniqueItemName( u"UserMenu_"_s );
}

QString QgsCustomization::uniqueToolBarName() const
{
  return uniqueItemName( u"UserToolBar_"_s );
}

QString QgsCustomization::uniqueActionName( const QString &originalActionName ) const
{
  return uniqueItemName( u"ActionRef_"_s + originalActionName + "_" );
}

QgsCustomization::QgsItem *QgsCustomization::getItem( const QString &path ) const
{
  const QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  const QHash<QString, QgsCustomization::QgsItem *> rootItems = {
    { "Menus", menusItem() },
    { "ToolBars", toolBarsItem() },
    { "Docks", docksItem() },
    { "BrowserItems", browserElementsItem() },
    { "StatusBarWidgets", statusBarWidgetsItem() }
  };

  QgsCustomization::QgsItem *currentItem = nullptr;
  for ( const QString &pathElem : pathElems )
  {
    if ( currentItem )
    {
      currentItem = currentItem->getChild( pathElem );
    }
    else
    {
      currentItem = rootItems.value( pathElem );
    }

    if ( !currentItem )
      return nullptr;
  }

  return currentItem;
}
