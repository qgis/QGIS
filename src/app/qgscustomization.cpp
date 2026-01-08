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
#include <QToolButton>
#include <QWidgetAction>

#define CUSTOMIZATION_CURRENT_VERSION "1"

QgsCustomization::Item::Item( QgsCustomization::Item *parent )
  : mParent( parent )
{
}

QgsCustomization::Item::Item( const QString &name, const QString &title, Item *parent )
  : mName( name )
  , mTitle( title )
  , mParent( parent )
{}

QgsCustomization::Item::~Item() = default;

const QString &QgsCustomization::Item::name() const
{
  return mName;
}

const QString &QgsCustomization::Item::title() const
{
  return mTitle;
}

void QgsCustomization::Item::setTitle( const QString &title )
{
  mTitle = title;
}

QgsCustomization::Item *QgsCustomization::Item::parent() const
{
  return mParent;
}

bool QgsCustomization::Item::isVisible() const
{
  return mVisible;
}

void QgsCustomization::Item::setVisible( bool isVisible )
{
  mVisible = isVisible;
}

void QgsCustomization::Item::setIcon( const QIcon &icon )
{
  mIcon = icon;
}

QIcon QgsCustomization::Item::icon() const
{
  return mIcon;
}

void QgsCustomization::Item::addItem( std::unique_ptr<Item> item )
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

QgsCustomization::Item *QgsCustomization::Item::getChild( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( mChildItemList.size() ) )
    return nullptr;

  return mChildItemList.at( index ).get();
}

QgsCustomization::Item *QgsCustomization::Item::getChild( const QString &name ) const
{
  return mChildItems.value( name, nullptr );
}

const std::vector<std::unique_ptr<QgsCustomization::Item>> &QgsCustomization::Item::childItemList() const
{
  return mChildItemList;
}

QgsCustomization::Item *QgsCustomization::Item::lastChild() const
{
  return mChildItemList.back().get();
}


long QgsCustomization::Item::indexOf( Item *item ) const
{
  const auto it = std::find_if( mChildItemList.cbegin(), mChildItemList.cend(), [item]( const std::unique_ptr<Item> &currentItem ) {
    return currentItem.get() == item;
  } );

  if ( it != mChildItemList.cend() )
    return std::distance( mChildItemList.cbegin(), it );
  else
    return -1;
}

unsigned long QgsCustomization::Item::childrenCount() const
{
  return mChildItemList.size();
}

void QgsCustomization::Item::insertItem( int position, std::unique_ptr<Item> item )
{
  if ( position < 0 && position >= static_cast<int>( mChildItemList.size() ) )
  {
    QgsDebugError( u"Insert item impossible, invalid position"_s );
    return;
  }

  mChildItemList.insert( std::next( mChildItemList.cbegin(), position ), std::move( item ) );
  Item *pitem = mChildItemList.at( position ).get();
  mChildItems[pitem->name()] = pitem;
}

void QgsCustomization::Item::deleteItem( int position )
{
  if ( position < 0 && position >= static_cast<int>( mChildItemList.size() ) )
  {
    QgsDebugError( u"Delete item impossible, invalid position"_s );
    return;
  }

  mChildItems.take( mChildItemList.at( position )->name() );
  mChildItemList.erase( std::next( mChildItemList.cbegin(), position ) );
}

void QgsCustomization::Item::writeXml( QDomDocument &doc, QDomElement &parent ) const
{
  QDomElement itemElem = doc.createElement( xmlTag() );
  itemElem.setAttribute( u"name"_s, mName );
  itemElem.setAttribute( u"visible"_s, mVisible );

  writeXmlItem( itemElem );

  for ( const std::unique_ptr<Item> &childItem : mChildItemList )
  {
    childItem->writeXml( doc, itemElem );
  }

  parent.appendChild( itemElem );
}

QString QgsCustomization::Item::readXml( const QDomElement &elem )
{
  mVisible = elem.attribute( u"visible"_s ) == "1"_L1;
  mName = elem.attribute( u"name"_s );
  if ( mName.isEmpty() )
  {
    return QObject::tr( "Invalid XML file : empty name for tag '%1'" ).arg( elem.tagName() );
  }

  readXmlItem( elem );

  for ( QDomElement childElem = elem.firstChildElement(); !childElem.isNull(); childElem = childElem.nextSiblingElement() )
  {
    std::unique_ptr<Item> childItem = createChildItem( childElem );
    if ( !childItem )
    {
      return QObject::tr( "Invalid XML file : failed to create an item '%1(%2)' as a child of item '%3(%4)'" )
        .arg( childElem.tagName() )
        .arg( childElem.attribute( u"name"_s ) )
        .arg( xmlTag() )
        .arg( mName );
    }
    childItem->readXml( childElem );
    addItem( std::move( childItem ) );
  }

  return QString();
}

bool QgsCustomization::Item::hasCapability( QgsCustomization::Item::ItemCapability pcapability ) const
{
  return static_cast<int>( capabilities() ) & static_cast<int>( pcapability );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Item::createChildItem( const QDomElement & )
{
  return nullptr;
}

void QgsCustomization::Item::copyItemAttributes( const QgsCustomization::Item *other )
{
  mName = other->mName;
  mTitle = other->mTitle;
  mVisible = other->mVisible;
  mIcon = other->mIcon;
  for ( const std::unique_ptr<QgsCustomization::Item> &otherChildItem : other->mChildItemList )
  {
    addItem( otherChildItem->clone( this ) );
  }
}

void QgsCustomization::Item::writeXmlItem( QDomElement & ) const {
};

void QgsCustomization::Item::readXmlItem( const QDomElement & ) {
};

QgsCustomization::Item::ItemCapability QgsCustomization::Item::capabilities() const
{
  return ItemCapability::None;
}

////////////////

QgsCustomization::Action::Action( QgsCustomization::Item *parent )
  : QgsCustomization::Item( parent )
{
}


QgsCustomization::Action::Action( const QString &name, const QString &title, Item *parent )
  : Item( name, title, parent )
{}

QString QgsCustomization::Action::xmlTag() const
{
  return u"Action"_s;
};

void QgsCustomization::Action::setQAction( QAction *qaction, qsizetype qActionIndex )
{
  mQAction = qaction;
  mQActionIndex = qActionIndex;
}

QAction *QgsCustomization::Action::qAction() const
{
  return mQAction;
}

qsizetype QgsCustomization::Action::qActionIndex() const
{
  return mQActionIndex;
}

QString QgsCustomization::Action::path() const
{
  QString path = name();

  Item const *currentItem = this;
  while ( ( currentItem = currentItem->parent() ) )
  {
    path.prepend( currentItem->name() + "/" );
  }

  return path;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Action::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::Action>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Action::createChildItem( const QDomElement &childElem )
{
  // Action with a menu can have child action
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<Action>( this );
  else
    return nullptr;
}

void QgsCustomization::Action::copyItemAttributes( const Item *other )
{
  Item::copyItemAttributes( other );
  if ( const Action *action = dynamic_cast<const Action *>( other ) )
  {
    mQAction = action->mQAction;
    mQActionIndex = action->mQActionIndex;
  }
}

QgsCustomization::Item::ItemCapability QgsCustomization::Action::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::Drag )
  );
}

////////////////

QgsCustomization::ActionRef::ActionRef( Item *parent )
  : Action( parent ) {}

QgsCustomization::ActionRef::ActionRef( const QString &name, const QString &title, const QString &path, Item *parent )
  : Action( name, title, parent )
  , mPath( path ) {}

const QString &QgsCustomization::ActionRef::path() const
{
  return mPath;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ActionRef::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::ActionRef>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::ActionRef::xmlTag() const
{
  return u"ActionRef"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ActionRef::createChildItem( const QDomElement & )
{
  return nullptr;
}

void QgsCustomization::ActionRef::readXmlItem( const QDomElement &elem )
{
  mPath = elem.attribute( u"path"_s );
};

void QgsCustomization::ActionRef::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"path"_s, mPath );
}

QgsCustomization::Item::ItemCapability QgsCustomization::ActionRef::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::Delete )
  );
};

void QgsCustomization::ActionRef::copyItemAttributes( const Item *other )
{
  Action::copyItemAttributes( other );
  if ( const ActionRef *action = dynamic_cast<const ActionRef *>( other ) )
  {
    mPath = action->mPath;
  }
}

////////////////

QgsCustomization::Menu::Menu( Item *parent )
  : Action( parent )
{}
QgsCustomization::Menu::Menu( const QString &name, const QString &title, Item *parent )
  : Action( name, title, parent )
{}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menu::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::Menu>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::Menu::xmlTag() const
{
  return u"Menu"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menu::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<QgsCustomization::Action>( this );
  else if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<QgsCustomization::Menu>( this );
  else
    return nullptr;
}

QgsCustomization::Item::ItemCapability QgsCustomization::Menu::capabilities() const
{
  return static_cast<ItemCapability>( ItemCapability::None );
}

////////////////

QgsCustomization::UserMenu::UserMenu( Item *parent )
  : Menu( parent )
{}

QgsCustomization::UserMenu::UserMenu( const QString &name, const QString &title, Item *parent )
  : Menu( name, title, parent )
{}

QgsCustomization::Item::ItemCapability QgsCustomization::UserMenu::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::ActionRefChild )
    | static_cast<int>( ItemCapability::UserMenuChild )
    | static_cast<int>( ItemCapability::Rename )
    | static_cast<int>( ItemCapability::Delete )
  );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::UserMenu::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::UserMenu>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::UserMenu::xmlTag() const
{
  return u"UserMenu"_s;
}

void QgsCustomization::UserMenu::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"title"_s, title() );
};

void QgsCustomization::UserMenu::readXmlItem( const QDomElement &elem )
{
  setTitle( elem.attribute( u"title"_s ) );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::UserMenu::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ActionRef"_L1 )
    return std::make_unique<ActionRef>( this );
  else if ( childElem.tagName() == "UserMenu"_L1 )
    return std::make_unique<UserMenu>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::ToolBar::ToolBar( Item *parent )
  : Item( parent )
{}

QgsCustomization::ToolBar::ToolBar( const QString &name, const QString &title, Item *parent )
  : Item( name, title, parent ) {}

void QgsCustomization::ToolBar::setWasVisible( const bool &wasVisible )
{
  mWasVisible = wasVisible;
}

bool QgsCustomization::ToolBar::wasVisible() const
{
  return mWasVisible;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBar::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::ToolBar>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::ToolBar::xmlTag() const
{
  return u"ToolBar"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBar::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Action"_L1 )
    return std::make_unique<Action>( this );
  if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<Menu>( this );
  else
    return nullptr;
}

void QgsCustomization::ToolBar::copyItemAttributes( const Item *other )
{
  Item::copyItemAttributes( other );
  if ( const ToolBar *tb = dynamic_cast<const ToolBar *>( other ) )
  {
    mWasVisible = tb->mWasVisible;
  }
}

////////////////

QgsCustomization::UserToolBar::UserToolBar( Item *parent )
  : ToolBar( parent )
{}

QgsCustomization::UserToolBar::UserToolBar( const QString &name, const QString &title, Item *parent )
  : ToolBar( name, title, parent )
{}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::UserToolBar::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::UserToolBar>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::UserToolBar::xmlTag() const
{
  return u"UserToolBar"_s;
}

void QgsCustomization::UserToolBar::writeXmlItem( QDomElement &elem ) const
{
  elem.setAttribute( u"title"_s, title() );
};

void QgsCustomization::UserToolBar::readXmlItem( const QDomElement &elem )
{
  setTitle( elem.attribute( u"title"_s ) );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::UserToolBar::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ActionRef"_L1 )
    return std::make_unique<ActionRef>( this );
  else
    return nullptr;
}

QgsCustomization::Item::ItemCapability QgsCustomization::UserToolBar::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::ActionRefChild )
    | static_cast<int>( ItemCapability::Rename )
    | static_cast<int>( ItemCapability::Delete )
  );
}

////////////////

QgsCustomization::ToolBars::ToolBars()
  : Item()
{
  mName = "ToolBars";
  setTitle( QObject::tr( "ToolBars" ) );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBars::clone( QgsCustomization::Item * ) const
{
  auto clone = std::make_unique<QgsCustomization::ToolBars>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::ToolBars::xmlTag() const
{
  return u"ToolBars"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBars::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "ToolBar"_L1 )
    return std::make_unique<ToolBar>( this );
  else if ( childElem.tagName() == "UserToolBar"_L1 )
    return std::make_unique<UserToolBar>( this );
  else
    return nullptr;
}

QgsCustomization::Item::ItemCapability QgsCustomization::ToolBars::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::UserToolBarChild )
  );
}

////////////////

QgsCustomization::Menus::Menus()
  : Item()
{
  mName = "Menus";
  setTitle( QObject::tr( "Menus" ) );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menus::clone( QgsCustomization::Item * ) const
{
  auto clone = std::make_unique<QgsCustomization::Menus>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::Menus::xmlTag() const
{
  return u"Menus"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menus::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Menu"_L1 )
    return std::make_unique<Menu>( this );
  else if ( childElem.tagName() == "UserMenu"_L1 )
    return std::make_unique<UserMenu>( this );
  else
    return nullptr;
}

QgsCustomization::Item::ItemCapability QgsCustomization::Menus::capabilities() const
{
  return static_cast<ItemCapability>(
    static_cast<int>( ItemCapability::UserMenuChild )
  );
}

////////////////

QgsCustomization::Dock::Dock( Item *parent )
  : Item( parent )
{
}

QgsCustomization::Dock::Dock( const QString &name, const QString &title, Item *parent )
  : Item( name, title, parent )
{
}

QString QgsCustomization::Dock::xmlTag() const
{
  return u"Dock"_s;
};

void QgsCustomization::Dock::copyItemAttributes( const Item *other )
{
  Item::copyItemAttributes( other );
  if ( const Dock *dock = dynamic_cast<const Dock *>( other ) )
  {
    mWasVisible = dock->mWasVisible;
  }
}

void QgsCustomization::Dock::setWasVisible( const bool &wasVisible )
{
  mWasVisible = wasVisible;
}

bool QgsCustomization::Dock::wasVisible() const
{
  return mWasVisible;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Dock::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::Dock>( parent );
  clone->copyItemAttributes( this );
  clone->mWasVisible = mWasVisible;
  return clone;
}

////////////////

QgsCustomization::Docks::Docks()
  : Item()
{
  mName = "Docks";
  setTitle( QObject::tr( "Docks" ) );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Docks::clone( QgsCustomization::Item * ) const
{
  auto clone = std::make_unique<QgsCustomization::Docks>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::Docks::xmlTag() const
{
  return u"Docks"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Docks::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "Dock"_L1 )
    return std::make_unique<Dock>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::BrowserItem::BrowserItem( Item *parent )
  : Item( parent )
{
}

QgsCustomization::BrowserItem::BrowserItem( const QString &name, const QString &title, Item *parent )
  : Item( name, title, parent )
{
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::BrowserItem::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::BrowserItem>( parent );
  clone->copyItemAttributes( this );
  return clone;
}


QString QgsCustomization::BrowserItem::xmlTag() const
{
  return u"BrowserItem"_s;
};

////////////////

QgsCustomization::BrowserItems::BrowserItems()
  : Item()
{
  mName = "BrowserItems";
  setTitle( QObject::tr( "Browser" ) );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::BrowserItems::clone( QgsCustomization::Item * ) const
{
  auto clone = std::make_unique<QgsCustomization::BrowserItems>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::BrowserItems::xmlTag() const
{
  return u"BrowserItems"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::BrowserItems::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "BrowserItem"_L1 )
    return std::make_unique<BrowserItem>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::StatusBarWidget::StatusBarWidget( Item *parent )
  : Item( parent )
{}

QgsCustomization::StatusBarWidget::StatusBarWidget( const QString &name, Item *parent )
  : Item( name, QString(), parent ) {}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::StatusBarWidget::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::StatusBarWidget>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::StatusBarWidget::xmlTag() const
{
  return u"StatusBarWidget"_s;
};

////////////////

QgsCustomization::StatusBarWidgets::StatusBarWidgets()
  : Item()
{
  mName = "StatusBarWidgets";
  setTitle( QObject::tr( "Status Bar" ) );
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::StatusBarWidgets::clone( QgsCustomization::Item * ) const
{
  auto clone = std::make_unique<QgsCustomization::StatusBarWidgets>();
  clone->copyItemAttributes( this );
  return clone;
}

QString QgsCustomization::StatusBarWidgets::xmlTag() const
{
  return u"StatusBarWidgets"_s;
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::StatusBarWidgets::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == "StatusBarWidget"_L1 )
    return std::make_unique<StatusBarWidget>( this );
  else
    return nullptr;
}

////////////////

QgsCustomization::QgsCustomization( const QString &customizationFile )
  : mCustomizationFile( customizationFile )
{
  const QFileInfo fileInfo( customizationFile );
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
  : mBrowserItems( dynamic_cast<BrowserItems *>( other.mBrowserItems->clone().release() ) )
  , mDocks( dynamic_cast<Docks *>( other.mDocks->clone().release() ) )
  , mMenus( dynamic_cast<Menus *>( other.mMenus->clone().release() ) )
  , mStatusBarWidgets( dynamic_cast<StatusBarWidgets *>( other.mStatusBarWidgets->clone().release() ) )
  , mToolBars( dynamic_cast<ToolBars *>( other.mToolBars->clone().release() ) )
  , mEnabled( other.mEnabled )
  , mSplashPath( other.mSplashPath )
  , mQgisApp( other.mQgisApp )
  , mCustomizationFile( other.mCustomizationFile )
{
}

/**
 * Assignment operator
 */
QgsCustomization &QgsCustomization::operator=( const QgsCustomization &other )
{
  if ( this == &other )
    return *this;

  mBrowserItems.reset( dynamic_cast<BrowserItems *>( other.mBrowserItems->clone().release() ) );
  mDocks.reset( dynamic_cast<Docks *>( other.mDocks->clone().release() ) );
  mMenus.reset( dynamic_cast<Menus *>( other.mMenus->clone().release() ) );
  mStatusBarWidgets.reset( dynamic_cast<StatusBarWidgets *>( other.mStatusBarWidgets->clone().release() ) );
  mToolBars.reset( dynamic_cast<ToolBars *>( other.mToolBars->clone().release() ) );
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

const std::unique_ptr<QgsCustomization::BrowserItems> &QgsCustomization::browserItems() const
{
  return mBrowserItems;
}

const std::unique_ptr<QgsCustomization::Docks> &QgsCustomization::docks() const
{
  return mDocks;
}

const std::unique_ptr<QgsCustomization::Menus> &QgsCustomization::menus() const
{
  return mMenus;
}

const std::unique_ptr<QgsCustomization::StatusBarWidgets> &QgsCustomization::statusBarWidgets() const
{
  return mStatusBarWidgets;
}

const std::unique_ptr<QgsCustomization::ToolBars> &QgsCustomization::toolBars() const
{
  return mToolBars;
}

void QgsCustomization::addActions( Item *item, QWidget *widget ) const
{
  if ( !item || !widget )
    return;

  for ( QgsCustomization::QWidgetIterator::Info it : QgsCustomization::QWidgetIterator( widget ) )
  {
    if ( it.name.isEmpty() )
      continue;

    // submenu
    Action *childItem = item->getChild<Action>( it.name );
    if ( !childItem )
    {
      if ( it.isMenu )
      {
        auto menuItem = std::make_unique<Menu>( it.name, it.title, item );
        item->addItem( std::move( menuItem ) );
      }
      else
      {
        auto action = std::make_unique<Action>( it.name, it.title, item );
        item->addItem( std::move( action ) );
      }

      childItem = item->lastChild<Action>();
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
    mToolBars = std::make_unique<ToolBars>();
  }

  const auto toolbars = mQgisApp->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolbars )
  {
    const QString name = tb->objectName();
    if ( name.isEmpty() )
      continue;

    ToolBar *t = mToolBars->getChild<ToolBar>( name );
    if ( !t )
    {
      auto toolBar = std::make_unique<ToolBar>( tb->objectName(), tb->windowTitle(), mToolBars.get() );
      mToolBars->addItem( std::move( toolBar ) );
      t = mToolBars->lastChild<ToolBar>();
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
    mMenus = std::make_unique<Menus>();
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
    mDocks = std::make_unique<Docks>();
  }

  const auto dockWidgets = mQgisApp->findChildren<QDockWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QDockWidget *dw : dockWidgets )
  {
    const QString name = dw->objectName();
    if ( name.isEmpty() )
      continue;

    Dock *d = mDocks->getChild<Dock>( name );
    if ( !d )
    {
      auto dock = std::make_unique<Dock>( name, dw->windowTitle(), mDocks.get() );
      mDocks->addItem( std::move( dock ) );
      d = mDocks->lastChild<Dock>();
    }

    d->setWasVisible( dw->isVisible() );
  }
}

void QgsCustomization::loadApplicationBrowserItems()
{
  if ( !mBrowserItems )
  {
    mBrowserItems = std::make_unique<BrowserItems>();
    const QList<QPair<QString, QString>> staticItems = {
      { u"special:Home"_s, QObject::tr( "Home Folder" ) },
      { u"special:ProjectHome"_s, QObject::tr( "Project Home Folder" ) },
      { u"special:Favorites"_s, QObject::tr( "Favorites Folder" ) },
      { u"special:Drives"_s, QObject::tr( "Drive Folders (e.g. C:\\)" ) },
      { u"special:Volumes"_s, QObject::tr( "Volume Folder (MacOS only)" ) }
    };

    for ( QPair<QString, QString> staticItem : staticItems )
    {
      auto browserItem = std::make_unique<BrowserItem>( staticItem.first, staticItem.second, mBrowserItems.get() );
      mBrowserItems->addItem( std::move( browserItem ) );
    }
  }

  const auto constProviders = QgsApplication::dataItemProviderRegistry()->providers();
  for ( QgsDataItemProvider *pr : constProviders )
  {
    const Qgis::DataItemProviderCapabilities capabilities = pr->capabilities();
    const QString name = pr->name();
    if ( !name.isEmpty() && capabilities != Qgis::DataItemProviderCapabilities( Qgis::DataItemProviderCapability::NoCapabilities ) )
    {
      if ( !mBrowserItems->getChild<BrowserItem>( name ) )
      {
        auto browserItem = std::make_unique<BrowserItem>( name, QObject::tr( "Data Item Provider: %1" ).arg( name ), mBrowserItems.get() );
        mBrowserItems->addItem( std::move( browserItem ) );
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
    mStatusBarWidgets = std::make_unique<StatusBarWidgets>();
  }

  QgsStatusBar *sb = mQgisApp->statusBarIface();
  const auto children = sb->findChildren<QWidget *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QWidget *statusBarWidget : children )
  {
    const QString name = statusBarWidget->objectName();
    if ( name.isEmpty() )
      continue;

    StatusBarWidget *s = mStatusBarWidgets->getChild<StatusBarWidget>( name );
    if ( !s )
    {
      auto statusBarWidget = std::make_unique<StatusBarWidget>( name, mStatusBarWidgets.get() );
      mStatusBarWidgets->addItem( std::move( statusBarWidget ) );
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
  for ( const std::unique_ptr<Item> &item : mBrowserItems->childItemList() )
  {
    BrowserItem *browserItem = dynamic_cast<BrowserItem *>( item.get() );
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
    if ( Dock *d = mDocks->getChild<Dock>( name ) )
    {
      dw->setVisible( d->wasVisible() && d->isVisible() );
      dw->toggleViewAction()->setVisible( d->isVisible() );
    }
  }
}

QgsCustomization::QWidgetIterator::QWidgetIterator( QWidget *widget )
  : mWidget( widget ) {};

QgsCustomization::QWidgetIterator::Iterator::Iterator( QWidget *ptr, qsizetype idx )
  : idx( idx ), mActions( ptr->actions() ) {}

QgsCustomization::QWidgetIterator::Info QgsCustomization::QWidgetIterator::Iterator::operator*() const
{
  if ( idx < 0 || idx >= mActions.count() )
    throw std::out_of_range {
      "Action iterator out of range"
    };

  QAction *act = mActions.at( idx );
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
  infos.index = idx;
  return infos;
}

QgsCustomization::QWidgetIterator::Iterator &QgsCustomization::QWidgetIterator::Iterator::operator++()
{
  idx++;
  while ( idx < mActions.count() && mActions.at( idx )->isSeparator() )
    idx++;
  return *this;
}

bool QgsCustomization::QWidgetIterator::Iterator::operator==( const Iterator &b ) const
{
  return idx == b.idx && ( idx < 0 || idx >= mActions.count() || mActions.at( idx ) == b.mActions.at( idx ) );
}

QgsCustomization::QWidgetIterator::Iterator QgsCustomization::QWidgetIterator::begin()
{
  return Iterator( mWidget, 0 );
}

QgsCustomization::QWidgetIterator::Iterator QgsCustomization::QWidgetIterator::end()
{
  return Iterator( mWidget, mWidget->actions().count() );
}

QWidget *QgsCustomization::findQWidget( const QString &path )
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  const QHash<QString, QWidget *> rootWidgets = { { "Menus", QgisApp::instance()->menuBar() }, { "ToolBars", QgisApp::instance() }, { "Docks", QgisApp::instance() }, { "StatusBarWidgets", QgisApp::instance()->statusBarIface() } };

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
      QgsCustomization::QWidgetIterator widgetIterator( currentWidget );
      currentWidget = nullptr;
      for ( QgsCustomization::QWidgetIterator::Info it : widgetIterator )
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
      currentWidget = dynamic_cast<QWidget *>( *it );
    }

    if ( !currentWidget )
      return nullptr;
  }

  return currentWidget;
}

QAction *QgsCustomization::findQAction( const QString &path )
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

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
void QgsCustomization::updateMenuActionVisibility( QgsCustomization::Item *parentItem, WidgetType *parentWidget )
{
  // clear all user menu
  const QList<QAction *> widgetActions = parentWidget->actions();
  for ( QAction *action : widgetActions )
  {
    const QMenu *menu = action->menu();
    if ( menu && menu->property( "__usermenu__" ).toBool() )
    {
      parentWidget->removeAction( action );
    }
  }

  // update non-user menu visibility
  updateActionVisibility( parentItem, parentWidget );

  // add user menu
  for ( const std::unique_ptr<QgsCustomization::Item> &childItem : parentItem->childItemList() )
  {
    if ( QgsCustomization::UserMenu *userMenu = dynamic_cast<QgsCustomization::UserMenu *>( childItem.get() ) )
    {
      QMenu *menu = new QMenu( userMenu->title(), parentWidget );
      menu->setProperty( "__usermenu__", true );
      menu->setObjectName( userMenu->name() );
      parentWidget->addMenu( menu );

      updateMenuActionVisibility( userMenu, menu );
    }
    else if ( QgsCustomization::ActionRef *actionRef = dynamic_cast<QgsCustomization::ActionRef *>( childItem.get() ) )
    {
      if ( QAction *action = findQAction( actionRef->path() ) )
      {
        parentWidget->addAction( action );
      }
    }
  }
}

void QgsCustomization::updateActionVisibility( QgsCustomization::Item *item, QWidget *widget )
{
  if ( !item || !widget )
    return;

  QSet<QgsCustomization::Item *> treatedChildItems;
  for ( QgsCustomization::QWidgetIterator::Info it : QgsCustomization::QWidgetIterator( widget ) )
  {
    if ( QgsCustomization::Item *childItem = item->getChild( it.name ) )
    {
      treatedChildItems << childItem;

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

  // all have been treated, no need to continue
  if ( static_cast<size_t>( treatedChildItems.count() ) == item->childItemList().size() )
    return;

  // Some action have been previously removed and could be visible again. If so, we need to add them again
  int nbRemoved = 0;
  for ( const std::unique_ptr<Item> &childItem : item->childItemList() )
  {
    Action *action = dynamic_cast<Action *>( childItem.get() );
    if ( !action )
    {
      QgsDebugError( u"Invalid child type, Action expected"_s );
      continue;
    }

    if ( !action->isVisible() )
      nbRemoved++;

    if ( action->qAction() && childItem->isVisible() && !treatedChildItems.contains( childItem.get() ) )
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

    if ( StatusBarWidget *s = mStatusBarWidgets->getChild<StatusBarWidget>( name ) )
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
    const QString name = tb->objectName();
    if ( tb && tb->property( "__usertoolbar__" ).toBool() )
    {
      // delete old toolbar, will recreate it later
      QgisApp::instance()->removeToolBar( tb );
      delete tb;
    }
    else if ( ToolBar *t = mToolBars->getChild<ToolBar>( name ) )
    {
      tb->setVisible( t->wasVisible() && t->isVisible() );
      tb->toggleViewAction()->setVisible( t->isVisible() );
      updateActionVisibility( t, tb );
    }
  }

  for ( const std::unique_ptr<QgsCustomization::Item> &childItem : toolBars()->childItemList() )
  {
    if ( QgsCustomization::UserToolBar *userToolBar = dynamic_cast<QgsCustomization::UserToolBar *>( childItem.get() ) )
    {
      QToolBar *toolBar = new QToolBar( userToolBar->title(), QgisApp::instance() );
      toolBar->setProperty( "__usertoolBar__", true );
      toolBar->setObjectName( userToolBar->name() );
      QgisApp::instance()->addToolBar( toolBar );

      for ( const std::unique_ptr<QgsCustomization::Item> &actionRefItem : userToolBar->childItemList() )
      {
        if ( QgsCustomization::ActionRef *actionRef = dynamic_cast<QgsCustomization::ActionRef *>( actionRefItem.get() ) )
        {
          if ( QAction *action = findQAction( actionRef->path() ) )
          {
            toolBar->addAction( action );
          }
        }
      }

      updateActionVisibility( userToolBar, toolBar );
    }
  }
}


QString QgsCustomization::writeXML( const QString &fileName ) const
{
  QDomDocument doc( u"Customization"_s );
  QDomElement root = doc.createElement( u"Customization"_s );
  root.setAttribute( u"version"_s, QStringLiteral( CUSTOMIZATION_CURRENT_VERSION ) );
  root.setAttribute( u"enabled"_s, mEnabled );

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
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
  ts.setCodec( "UTF-8" );
#endif
  doc.save( ts, 2 );
  f.close();

  return QString();
}

QString QgsCustomization::write() const
{
  return writeXML( mCustomizationFile );
}

QString QgsCustomization::writeFile( const QString &filePath ) const
{
  return writeXML( filePath );
}

QString QgsCustomization::readXml( const QString &fileName )
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

  mEnabled = docEl.attribute( u"enabled"_s ) == "1"_L1;
  if ( docEl.hasAttribute( u"splashPath"_s ) )
    mSplashPath = docEl.attribute( u"splashPath"_s );

  const QString version = docEl.attribute( u"version"_s );
  if ( version != QLatin1String( CUSTOMIZATION_CURRENT_VERSION ) && version != "1"_L1 )
  {
    return QObject::tr( "Invalid XML file : incorrect version" );
  }

  mBrowserItems = std::make_unique<BrowserItems>();
  mBrowserItems->readXml( docEl.firstChildElement( u"BrowserItems"_s ) );
  mDocks = std::make_unique<Docks>();
  mDocks->readXml( docEl.firstChildElement( u"Docks"_s ) );
  mMenus = std::make_unique<Menus>();
  mMenus->readXml( docEl.firstChildElement( u"Menus"_s ) );
  mStatusBarWidgets = std::make_unique<StatusBarWidgets>();
  mStatusBarWidgets->readXml( docEl.firstChildElement( u"StatusBarWidgets"_s ) );
  mToolBars = std::make_unique<ToolBars>();
  mToolBars->readXml( docEl.firstChildElement( u"ToolBars"_s ) );

  return QString();
}

void QgsCustomization::read()
{
  ( void ) readXml( mCustomizationFile );
}

QString QgsCustomization::readFile( const QString &filePath )
{
  return readXml( filePath );
}

void QgsCustomization::loadOldIniFile( const QString &filePath )
{
  // enabled state is in application ini file
  mEnabled = QSettings().value( "UI/Customization/enabled", false ).toBool();

  QSettings settings( filePath, QSettings::IniFormat );
  mSplashPath = settings.value( u"/Customization/splashpath"_s, QgsApplication::splashPath() ).toString();

  mBrowserItems = std::make_unique<BrowserItems>();
  mDocks = std::make_unique<Docks>();
  mMenus = std::make_unique<Menus>();
  mStatusBarWidgets = std::make_unique<StatusBarWidgets>();
  mToolBars = std::make_unique<ToolBars>();

  // menus
  settings.beginGroup( u"Customization/Menus"_s );

  for ( const QString &key : settings.allKeys() )
  {
    Item *rootItem = menus().get();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( Item *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else if ( i < keyElems.count() - 1 ) // Menu
      {
        rootItem->addItem( std::make_unique<Menu>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
      else // Action
      {
        rootItem->addItem( std::make_unique<Action>( keyElem, QString(), rootItem ) );
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
    Item *rootItem = toolBars().get();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( Item *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else if ( i == 0 ) // ToolBar
      {
        rootItem->addItem( std::make_unique<ToolBar>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
      else // Action
      {
        rootItem->addItem( std::make_unique<Action>( keyElem, QString(), rootItem ) );
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
    Item *rootItem = docks().get();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( Item *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // Dock
      {
        rootItem->addItem( std::make_unique<Dock>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  statusBarWidgets()->setVisible( settings.value( "Customization/StatusBar", true ).toBool() );
  settings.beginGroup( u"Customization/StatusBar"_s );

  for ( const QString &key : settings.allKeys() )
  {
    Item *rootItem = statusBarWidgets().get();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( Item *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // StatusBarWidget
      {
        rootItem->addItem( std::make_unique<StatusBarWidget>( keyElem, rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();

  settings.beginGroup( u"Customization/Browser"_s );

  for ( const QString &key : settings.allKeys() )
  {
    Item *rootItem = browserItems().get();
    const QStringList keyElems = key.split( "/" );
    for ( int i = 0; i < keyElems.count(); i++ )
    {
      const QString &keyElem = keyElems.at( i );
      if ( Item *tbItem = rootItem->getChild( keyElem ) )
      {
        rootItem = tbItem;
      }
      else // BrowserItem
      {
        rootItem->addItem( std::make_unique<BrowserItem>( keyElem, QString(), rootItem ) );
        rootItem = rootItem->childItemList().back().get();
      }
    }

    rootItem->setVisible( settings.value( key, true ).toBool() );
  }

  settings.endGroup();
}

int maxSuffixNum( const QgsCustomization::Item *item, const QString &baseName )
{
  int max = 0;

  if ( item->name().startsWith( baseName ) )
  {
    bool ok = false;
    const int suffixNum = item->name().mid( baseName.length() ).toInt( &ok );
    if ( ok )
      max = suffixNum;
  }

  for ( const std::unique_ptr<QgsCustomization::Item> &childItem : item->childItemList() )
  {
    const int childSuffixNum = maxSuffixNum( childItem.get(), baseName );
    max = std::max( childSuffixNum, max );
  }

  return max;
}

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

QgsCustomization::Item *QgsCustomization::getItem( const QString &path ) const
{
  QStringList pathElems = path.split( "/" );
  if ( pathElems.isEmpty() )
    return nullptr;

  const QHash<QString, QgsCustomization::Item *> rootItems = {
    { "Menus", menus().get() },
    { "ToolBars", toolBars().get() },
    { "Docks", docks().get() },
    { "BrowserItems", browserItems().get() },
    { "StatusBarWidgets", statusBarWidgets().get() }
  };

  QgsCustomization::Item *currentItem = nullptr;
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
