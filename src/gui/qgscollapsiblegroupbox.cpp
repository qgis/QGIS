/***************************************************************************
                          qgscollapsiblegroupbox.cpp
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscollapsiblegroupbox.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include <QToolButton>
#include <QMouseEvent>
#include <QStyleOptionGroupBox>
#include <QSettings>
#include <QScrollArea>

QIcon QgsCollapsibleGroupBoxBasic::mCollapseIcon;
QIcon QgsCollapsibleGroupBoxBasic::mExpandIcon;

QgsCollapsibleGroupBoxBasic::QgsCollapsibleGroupBoxBasic( QWidget *parent )
    : QGroupBox( parent )
{
  init();
}

QgsCollapsibleGroupBoxBasic::QgsCollapsibleGroupBoxBasic( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent )
{
  init();
}

QgsCollapsibleGroupBoxBasic::~QgsCollapsibleGroupBoxBasic()
{
  //QgsDebugMsg( "Entered" );
}


void QgsCollapsibleGroupBoxBasic::init()
{
  //QgsDebugMsg( "Entered" );
  // variables
  mCollapsed = false;
  mInitFlat = false;
  mInitFlatChecked = false;
  mScrollOnExpand = true;
  mShown = false;
  mParentScrollArea = 0;

  // init icons
  if ( mCollapseIcon.isNull() )
  {
    mCollapseIcon = QgsApplication::getThemeIcon( "/mIconCollapse.png" );
    mExpandIcon = QgsApplication::getThemeIcon( "/mIconExpand.png" );
  }

  // collapse button
  mCollapseButton = new QToolButton( this );
  mCollapseButton->setObjectName( "collapseButton" );
  mCollapseButton->setAutoRaise( true );
  mCollapseButton->setFixedSize( 16, 16 );
  // TODO set size (as well as margins) depending on theme, in updateStyle()
  mCollapseButton->setIconSize( QSize( 12, 12 ) );
  mCollapseButton->setIcon( mCollapseIcon );

  connect( mCollapseButton, SIGNAL( clicked() ), this, SLOT( toggleCollapsed() ) );
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( checkToggled( bool ) ) );
}

void QgsCollapsibleGroupBoxBasic::showEvent( QShowEvent * event )
{
  //QgsDebugMsg( "Entered" );
  // initialise widget on first show event only
  if ( mShown )
  {
    event->accept();
    return;
  }

  // check if groupbox was set to flat in Designer or in code
  if ( !mInitFlatChecked )
  {
    mInitFlat = isFlat();
    mInitFlatChecked = true;
  }

  // find parent QScrollArea - this might not work in complex layouts - should we look deeper?
  if ( parent() && parent()->parent() )
    mParentScrollArea = dynamic_cast<QScrollArea*>( parent()->parent()->parent() );
  else
    mParentScrollArea = 0;
  if ( mParentScrollArea )
  {
    QgsDebugMsg( "found a QScrollArea parent: " + mParentScrollArea->objectName() );
  }
  else
  {
    QgsDebugMsg( "did not find a QScrollArea parent" );
  }

  updateStyle();

  // expand if needed - any calls to setCollapsed() before only set mCollapsed, but have UI effect
  if ( mCollapsed )
  {
    setCollapsed( mCollapsed );
  }
  else
  {
    // emit signal for connections using collapsed state
    emit collapsedStateChanged( isCollapsed() );
  }
  // set mShown after first setCollapsed call or expanded groupboxes
  // will scroll scroll areas when first shown
  mShown = true;
  event->accept();
}

void QgsCollapsibleGroupBoxBasic::mouseReleaseEvent( QMouseEvent *event )
{
  // catch mouse release over title when non checkable, to collapse/expand
  if ( !isCheckable() && event->button() == Qt::LeftButton )
  {
    if ( titleRect().contains( event->pos() ) )
    {
      toggleCollapsed();
      return;
    }
  }
  // default behaviour - pass to QGroupBox
  QGroupBox::mouseReleaseEvent( event );
}

void QgsCollapsibleGroupBoxBasic::changeEvent( QEvent *event )
{
  // always re-enable mCollapseButton when groupbox was previously disabled
  // e.g. resulting from a disabled parent of groupbox, or a signal/slot connection

  // default behaviour - pass to QGroupBox
  QGroupBox::changeEvent( event );

  if ( event->type() == QEvent::EnabledChange && isEnabled() )
    mCollapseButton->setEnabled( true );
}

QRect QgsCollapsibleGroupBoxBasic::titleRect() const
{
  QStyleOptionGroupBox box;
  initStyleOption( &box );
  return style()->subControlRect( QStyle::CC_GroupBox, &box,
                                  QStyle::SC_GroupBoxLabel, this );
}

void QgsCollapsibleGroupBoxBasic::checkToggled( bool chkd )
{
  mCollapseButton->setEnabled( true ); // always keep enabled
  // expand/collapse when toggled
  if ( chkd && isCollapsed() )
    setCollapsed( false );
  else if ( ! chkd && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBoxBasic::toggleCollapsed()
{
  setCollapsed( !mCollapsed );
}

void QgsCollapsibleGroupBoxBasic::updateStyle()
{
  setUpdatesEnabled( false );

  // margin/offset defaults
  int marginLeft = 20;  // title margin for disclosure triangle
  int marginRight = 5;  // a little bit of space on the right, to match space on the left
  int offsetLeft = 0;   // offset for oxygen theme
  int offsetTop = 0;
//  int offsetTop2 = 0;   // offset for triangle

  // starting top offset for custom groupboxes in app stylesheet
  QStyleOptionGroupBox box;
  initStyleOption( &box );
  QRect rectCheckBox = style()->subControlRect( QStyle::CC_GroupBox, &box,
                       QStyle::SC_GroupBoxCheckBox, this );
  int offsetTop2 = rectCheckBox.top();   // offset for triangle


  // calculate offset if frame overlaps triangle (oxygen theme)
  // using an offset of 6 pixels from frame border
  if ( QApplication::style()->objectName().toLower() == "oxygen" )
  {
    QStyleOptionGroupBox box;
    initStyleOption( &box );
    QRect rectFrame = style()->subControlRect( QStyle::CC_GroupBox, &box,
                      QStyle::SC_GroupBoxFrame, this );
    QRect rectCheckBox = style()->subControlRect( QStyle::CC_GroupBox, &box,
                         QStyle::SC_GroupBoxCheckBox, this );
    if ( rectFrame.left() <= 0 )
      offsetLeft = 6 + rectFrame.left();
    if ( rectFrame.top() <= 0 )
    {
      if ( isCheckable() )
      {
        // if is checkable align with checkbox
        offsetTop = ( rectCheckBox.height() / 2 ) -
                    ( mCollapseButton->height() / 2 ) + rectCheckBox.top();
        offsetTop2 = offsetTop + 1;
      }
      else
      {
        offsetTop = 6 + rectFrame.top();
        offsetTop2 = offsetTop;
      }
    }
  }

  QgsDebugMsg( QString( "groupbox: %1 style: %2 offset: left=%3 top=%4 top2=%5" ).arg(
                 objectName() ).arg( QApplication::style()->objectName() ).arg( offsetLeft ).arg( offsetTop ).arg( offsetTop2 ) );

  // customize style sheet for collapse/expand button and force left-aligned title
  // TODO: move to app stylesheet system, when appropriate
  QString ss;
  ss += "QgsCollapsibleGroupBoxBasic::title, QgsCollapsibleGroupBox::title {";
  ss += "  subcontrol-origin: margin;";
  ss += "  subcontrol-position: top left;";
  ss += QString( "  margin-left: %1px;" ).arg( marginLeft );
  ss += QString( "  margin-right: %1px;" ).arg( marginRight );
  ss += QString( "  left: %1px;" ).arg( offsetLeft );
  ss += QString( "  top: %1px;" ).arg( offsetTop );
  ss += "}";
  setStyleSheet( ss );

  // clear toolbutton default background and border and apply offset
  QString ssd;
  ssd = QString( "QgsCollapsibleGroupBoxBasic > QToolButton#%1, QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );
  if ( offsetLeft != 0 || offsetTop2 != 0 )
    mCollapseButton->move( offsetLeft, offsetTop2 );

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBoxBasic::setCollapsed( bool collapse )
{
  mCollapsed = collapse;

  if ( !isVisible() )
    return;

  // for consistent look/spacing across platforms when collapsed
  if ( ! mInitFlat ) // skip if initially set to flat in Designer
    setFlat( collapse );

  // avoid flicker in X11
  QApplication::processEvents();

  // set maximum height to hide contents - does this work in all envs?
  // setMaximumHeight( collapse ? 25 : 16777215 );
  setMaximumHeight( collapse ? titleRect().bottom() + 6 : 16777215 );
  mCollapseButton->setIcon( collapse ? mExpandIcon : mCollapseIcon );

  // if expanding and is in a QScrollArea, scroll down to make entire widget visible
  if ( mShown && mScrollOnExpand && !collapse && mParentScrollArea )
  {
    // process events so entire widget is shown
    QApplication::processEvents();
    mParentScrollArea->ensureWidgetVisible( this );
  }
  // emit signal for connections using collapsed state
  emit collapsedStateChanged( isCollapsed() );
}

// ----

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent, QSettings* settings )
    : QgsCollapsibleGroupBoxBasic( parent ), mSettings( settings )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent, QSettings* settings )
    : QgsCollapsibleGroupBoxBasic( title, parent ), mSettings( settings )
{
  init();
}

QgsCollapsibleGroupBox::~QgsCollapsibleGroupBox()
{
  //QgsDebugMsg( "Entered" );
  saveState();
  if ( mDelSettings ) // local settings obj to delete
    delete mSettings;
  mSettings = 0; // null the pointer (in case of outside settings obj)
}

void QgsCollapsibleGroupBox::setSettings( QSettings* settings )
{
  if ( mDelSettings ) // local settings obj to delete
    delete mSettings;
  mSettings = settings;
  mDelSettings = false; // don't delete outside obj
}


void QgsCollapsibleGroupBox::init()
{
  //QgsDebugMsg( "Entered" );
  // use pointer to app qsettings if no custom qsettings specified
  // custom qsettings object may be from Python plugin
  mDelSettings = false;
  if ( !mSettings )
  {
    mSettings = new QSettings();
    mDelSettings = true; // only delete obj created by class
  }
  // variables
  mSaveCollapsedState = true;
  // NOTE: only turn on mSaveCheckedState for groupboxes NOT used
  // in multiple places or used as options for different parent objects
  mSaveCheckedState = false;
  mSettingGroup = ""; // if not set, use window object name
}

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  //QgsDebugMsg( "Entered" );
  // initialise widget on first show event only
  if ( mShown )
  {
    event->accept();
    return;
  }

  // check if groupbox was set to flat in Designer or in code
  if ( !mInitFlatChecked )
  {
    mInitFlat = isFlat();
    mInitFlatChecked = true;
  }

  loadState();

  QgsCollapsibleGroupBoxBasic::showEvent( event );
}

QString QgsCollapsibleGroupBox::saveKey() const
{
  // save key for load/save state
  // currently QgsCollapsibleGroupBox/window()/object
  QString saveKey = "/" + objectName();
  // QObject* parentWidget = parent();
  // while ( parentWidget != NULL )
  // {
  //   saveKey = "/" + parentWidget->objectName() + saveKey;
  //   parentWidget = parentWidget->parent();
  // }
  // if ( parent() != NULL )
  //   saveKey = "/" + parent()->objectName() + saveKey;
  QString setgrp = mSettingGroup.isEmpty() ? window()->objectName() : mSettingGroup;
  saveKey = "/" + setgrp + saveKey;
  saveKey = "QgsCollapsibleGroupBox" + saveKey;
  return saveKey;
}

void QgsCollapsibleGroupBox::loadState()
{
  //QgsDebugMsg( "Entered" );
  if ( !mSettings )
    return;

  if ( !isEnabled() || ( !mSaveCollapsedState && !mSaveCheckedState ) )
    return;

  setUpdatesEnabled( false );

  QString key = saveKey();
  QVariant val;
  if ( mSaveCheckedState )
  {
    val = mSettings->value( key + "/checked" );
    if ( ! val.isNull() )
      setChecked( val.toBool() );
  }
  if ( mSaveCollapsedState )
  {
    val = mSettings->value( key + "/collapsed" );
    if ( ! val.isNull() )
      setCollapsed( val.toBool() );
  }

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBox::saveState()
{
  //QgsDebugMsg( "Entered" );
  if ( !mSettings )
    return;

  if ( !isEnabled() || ( !mSaveCollapsedState && !mSaveCheckedState ) )
    return;

  QString key = saveKey();

  if ( mSaveCheckedState )
    mSettings->setValue( key + "/checked", isChecked() );
  if ( mSaveCollapsedState )
    mSettings->setValue( key + "/collapsed", isCollapsed() );
}

