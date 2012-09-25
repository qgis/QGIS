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

QIcon QgsCollapsibleGroupBox::mCollapseIcon;
QIcon QgsCollapsibleGroupBox::mExpandIcon;

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
    : QGroupBox( parent )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent )
{
  init();
}

QgsCollapsibleGroupBox::~QgsCollapsibleGroupBox()
{
  saveState();
}

void QgsCollapsibleGroupBox::init()
{
  // variables
  mCollapsed = false;
  mSaveCollapsedState = true;
  // NOTE: only turn on mSaveCheckedState for groupboxes NOT used
  // in multiple places or used as options for different parent objects
  mSaveCheckedState = false;
  mSettingGroup = ""; // if not set, use window object name
  mInitFlat = false;
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

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  // initialise widget on first show event only
  if ( mShown )
  {
    event->accept();
    return;
  }

  // check if groupbox was set to flat in Designer or in code
  mInitFlat = isFlat();

  // find parent QScrollArea - this might not work in complex layouts - should we look deeper?
  if ( parent() && parent()->parent() )
    mParentScrollArea = dynamic_cast<QScrollArea*>( parent()->parent()->parent() );
  else
    mParentScrollArea = 0;
  if ( mParentScrollArea )
    QgsDebugMsg( "found a QScrollArea parent: " + mParentScrollArea->objectName() );
  else
    QgsDebugMsg( "did not find a QScrollArea parent" );

  loadState();

  updateStyle();

  // expand if needed - any calls to setCollapsed() before only set mCollapsed, but have UI effect
  if ( mCollapsed )
  {
    setCollapsed( mCollapsed );
  }
  else
  {
    // emit signal for connections using expanded state
    emit collapsedStateChanged( this );
  }
  // set mShown after first setCollapsed call or expanded groupboxes
  // will scroll scroll areas when first shown
  mShown = true;
  event->accept();
}

void QgsCollapsibleGroupBox::mouseReleaseEvent( QMouseEvent *event )
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

void QgsCollapsibleGroupBox::changeEvent( QEvent *event )
{
  // always re-enable mCollapseButton when groupbox was previously disabled
  // e.g. resulting from a disabled parent of groupbox, or a signal/slot connection

  // default behaviour - pass to QGroupBox
  QGroupBox::changeEvent( event );

  if ( event->type() == QEvent::EnabledChange && isEnabled() )
    mCollapseButton->setEnabled( true );
}

QRect QgsCollapsibleGroupBox::titleRect() const
{
  QStyleOptionGroupBox box;
  initStyleOption( &box );
  return style()->subControlRect( QStyle::CC_GroupBox, &box,
                                  QStyle::SC_GroupBoxLabel, this );
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
  if ( !isEnabled() || ( !mSaveCollapsedState && !mSaveCheckedState ) )
    return;

  setUpdatesEnabled( false );

  QSettings settings;
  QString key = saveKey();
  QVariant val;
  if ( mSaveCheckedState )
  {
    val = settings.value( key + "/checked" );
    if ( ! val.isNull() )
      setChecked( val.toBool() );
  }
  if ( mSaveCollapsedState )
  {
    val = settings.value( key + "/collapsed" );
    if ( ! val.isNull() )
      setCollapsed( val.toBool() );
  }

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBox::saveState()
{
  if ( !isEnabled() || ( !mSaveCollapsedState && !mSaveCheckedState ) )
    return;

  QSettings settings;
  QString key = saveKey();

  if ( mSaveCheckedState )
    settings.setValue( key + "/checked", isChecked() );
  if ( mSaveCollapsedState )
    settings.setValue( key + "/collapsed", isCollapsed() );
}

void QgsCollapsibleGroupBox::checkToggled( bool chkd )
{
  mCollapseButton->setEnabled( true ); // always keep enabled
  // expand/collapse when toggled
  if ( chkd && isCollapsed() )
    setCollapsed( false );
  else if ( ! chkd && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBox::toggleCollapsed()
{
  setCollapsed( !mCollapsed );
}

void QgsCollapsibleGroupBox::updateStyle()
{
  setUpdatesEnabled( false );

  // margin/offset defaults
  int marginLeft = 20;  // title margin for disclosure triangle
  int marginRight = 5;  // a little bit of space on the right, to match space on the left
  int offsetLeft = 0;   // offset for oxygen theme
  int offsetTop = 0;
  int offsetTop2 = 0;   // offset for triangle

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
  ss += "QgsCollapsibleGroupBox::title {";
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
  ssd = QString( "QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );
  if ( offsetLeft != 0 || offsetTop2 != 0 )
    mCollapseButton->move( offsetLeft, offsetTop2 );

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
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
  emit collapsedStateChanged( this );
}

