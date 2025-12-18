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

void QgsCustomization::Item::writeXml( QDomDocument &doc, QDomElement &parent ) const
{
  QDomElement itemElem = doc.createElement( xmlTag() );
  itemElem.setAttribute( QStringLiteral( "name" ), mName );
  itemElem.setAttribute( QStringLiteral( "visible" ), mVisible );

  for ( const std::unique_ptr<Item> &childItem : mChildItemList )
  {
    childItem->writeXml( doc, itemElem );
  }

  parent.appendChild( itemElem );
}

QString QgsCustomization::Item::readXml( const QDomElement &elem )
{
  mVisible = elem.attribute( QStringLiteral( "visible" ) ) == QStringLiteral( "1" );
  mName = elem.attribute( QStringLiteral( "name" ) );
  if ( mName.isEmpty() )
  {
    return QObject::tr( "Invalid XML file : empty name for tag '%1'" ).arg( elem.tagName() );
  }

  for ( QDomElement childElem = elem.firstChildElement(); !childElem.isNull(); childElem = childElem.nextSiblingElement() )
  {
    std::unique_ptr<Item> childItem = createChildItem( childElem );
    if ( !childItem )
    {
      return QObject::tr( "Invalid XML file : failed to create an item '%1(%2)' as a child of item '%3(%4)'" )
        .arg( childElem.tagName() )
        .arg( childElem.attribute( QStringLiteral( "name" ) ) )
        .arg( xmlTag() )
        .arg( mName );
    }
    childItem->readXml( childElem );
    addItem( std::move( childItem ) );
  }

  return QString();
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
  return QStringLiteral( "Action" );
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

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Action::clone( QgsCustomization::Item *parent ) const
{
  auto clone = std::make_unique<QgsCustomization::Action>( parent );
  clone->copyItemAttributes( this );
  return clone;
}

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Action::createChildItem( const QDomElement &childElem )
{
  // Action with a menu can have child action
  if ( childElem.tagName() == QStringLiteral( "Action" ) )
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
  return QStringLiteral( "Menu" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menu::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "Action" ) )
    return std::make_unique<QgsCustomization::Action>( this );
  else if ( childElem.tagName() == QStringLiteral( "Menu" ) )
    return std::make_unique<QgsCustomization::Menu>( this );
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
  return QStringLiteral( "ToolBar" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBar::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "Action" ) )
    return std::make_unique<Action>( this );
  if ( childElem.tagName() == QStringLiteral( "Menu" ) )
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
  return QStringLiteral( "ToolBars" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::ToolBars::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "ToolBar" ) )
    return std::make_unique<ToolBar>( this );
  else
    return nullptr;
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
  return QStringLiteral( "Menus" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Menus::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "Menu" ) )
    return std::make_unique<Menu>( this );
  else
    return nullptr;
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
  return QStringLiteral( "Dock" );
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
  return QStringLiteral( "Docks" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::Docks::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "Dock" ) )
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
  return QStringLiteral( "BrowserItem" );
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
  return QStringLiteral( "BrowserItems" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::BrowserItems::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "BrowserItem" ) )
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
  return QStringLiteral( "StatusBarWidget" );
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
  return QStringLiteral( "StatusBarWidgets" );
};

std::unique_ptr<QgsCustomization::Item> QgsCustomization::StatusBarWidgets::createChildItem( const QDomElement &childElem )
{
  if ( childElem.tagName() == QStringLiteral( "StatusBarWidget" ) )
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
        std::unique_ptr<Action> action = std::make_unique<Action>( it.name, it.title, item );
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
      std::unique_ptr<ToolBar> toolBar = std::make_unique<ToolBar>( tb->objectName(), tb->windowTitle(), mToolBars.get() );
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
      std::unique_ptr<Dock> dock = std::make_unique<Dock>( name, dw->windowTitle(), mDocks.get() );
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
      { QStringLiteral( "special:Home" ), QObject::tr( "Home Folder" ) },
      { QStringLiteral( "special:ProjectHome" ), QObject::tr( "Project Home Folder" ) },
      { QStringLiteral( "special:Favorites" ), QObject::tr( "Favorites Folder" ) },
      { QStringLiteral( "special:Drives" ), QObject::tr( "Drive Folders (e.g. C:\\)" ) },
      { QStringLiteral( "special:Volumes" ), QObject::tr( "Volume Folder (MacOS only)" ) }
    };

    for ( QPair<QString, QString> staticItem : staticItems )
    {
      std::unique_ptr<BrowserItem> browserItem = std::make_unique<BrowserItem>( staticItem.first, staticItem.second, mBrowserItems.get() );
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
        std::unique_ptr<BrowserItem> browserItem = std::make_unique<BrowserItem>( name, QObject::tr( "Data Item Provider: %1" ).arg( name ), mBrowserItems.get() );
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
      std::unique_ptr<StatusBarWidget> statusBarWidget = std::make_unique<StatusBarWidget>( name, mStatusBarWidgets.get() );
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
      QgsDebugError( QStringLiteral( "Invalid child type, Action expected" ) );
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
  updateActionVisibility( mMenus.get(), menuBar );
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

  const auto toolBars = mQgisApp->findChildren<QToolBar *>( QString(), Qt::FindDirectChildrenOnly );
  for ( QToolBar *tb : toolBars )
  {
    const QString name = tb->objectName();
    if ( ToolBar *t = mToolBars->getChild<ToolBar>( name ) )
    {
      tb->setVisible( t->wasVisible() && t->isVisible() );
      tb->toggleViewAction()->setVisible( t->isVisible() );
      updateActionVisibility( t, tb );
    }
  }
}


QString QgsCustomization::writeXML( const QString &fileName ) const
{
  QDomDocument doc( QStringLiteral( "Customization" ) );
  QDomElement root = doc.createElement( QStringLiteral( "Customization" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( CUSTOMIZATION_CURRENT_VERSION ) );
  root.setAttribute( QStringLiteral( "enabled" ), mEnabled );

  if ( !mSplashPath.isEmpty() )
    root.setAttribute( QStringLiteral( "splashPath" ), mSplashPath );

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
  QDomDocument doc( QStringLiteral( "customization" ) );
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
  if ( docEl.tagName() != QLatin1String( "Customization" ) )
  {
    return QObject::tr( "Invalid XML file : root tag must be 'Customization'" );
  }

  mEnabled = docEl.attribute( QStringLiteral( "enabled" ) ) == QStringLiteral( "1" );
  if ( docEl.hasAttribute( QStringLiteral( "splashPath" ) ) )
    mSplashPath = docEl.attribute( QStringLiteral( "splashPath" ) );

  const QString version = docEl.attribute( QStringLiteral( "version" ) );
  if ( version != QLatin1String( CUSTOMIZATION_CURRENT_VERSION ) && version != QLatin1String( "1" ) )
  {
    return QObject::tr( "Invalid XML file : incorrect version" );
  }

  mBrowserItems = std::make_unique<BrowserItems>();
  mBrowserItems->readXml( docEl.firstChildElement( QStringLiteral( "BrowserItems" ) ) );
  mDocks = std::make_unique<Docks>();
  mDocks->readXml( docEl.firstChildElement( QStringLiteral( "Docks" ) ) );
  mMenus = std::make_unique<Menus>();
  mMenus->readXml( docEl.firstChildElement( QStringLiteral( "Menus" ) ) );
  mStatusBarWidgets = std::make_unique<StatusBarWidgets>();
  mStatusBarWidgets->readXml( docEl.firstChildElement( QStringLiteral( "StatusBarWidgets" ) ) );
  mToolBars = std::make_unique<ToolBars>();
  mToolBars->readXml( docEl.firstChildElement( QStringLiteral( "ToolBars" ) ) );

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
  mSplashPath = settings.value( QStringLiteral( "/Customization/splashpath" ), QgsApplication::splashPath() ).toString();

  mBrowserItems = std::make_unique<BrowserItems>();
  mDocks = std::make_unique<Docks>();
  mMenus = std::make_unique<Menus>();
  mStatusBarWidgets = std::make_unique<StatusBarWidgets>();
  mToolBars = std::make_unique<ToolBars>();

  // menus
  settings.beginGroup( QStringLiteral( "Customization/Menus" ) );

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
  settings.beginGroup( QStringLiteral( "Customization/Toolbars" ) );

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
  settings.beginGroup( QStringLiteral( "Customization/Docks" ) );
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
  settings.beginGroup( QStringLiteral( "Customization/StatusBar" ) );

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

  settings.beginGroup( QStringLiteral( "Customization/Browser" ) );

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
