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
#include <QPushButton>
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
  mSyncParent = 0;
  mSyncGroup = "";
  mAltDown = false;
  mShiftDown = false;
  mTitleClicked = false;

  // init icons
  if ( mCollapseIcon.isNull() )
  {
    mCollapseIcon = QgsApplication::getThemeIcon( "/mIconCollapse.png" );
    mExpandIcon = QgsApplication::getThemeIcon( "/mIconExpand.png" );
  }

  // collapse button
  mCollapseButton = new QgsGroupBoxCollapseButton( this );
  mCollapseButton->setObjectName( "collapseButton" );
  mCollapseButton->setAutoRaise( true );
  mCollapseButton->setFixedSize( 16, 16 );
  // TODO set size (as well as margins) depending on theme, in updateStyle()
  mCollapseButton->setIconSize( QSize( 12, 12 ) );
  mCollapseButton->setIcon( mCollapseIcon );

  connect( mCollapseButton, SIGNAL( clicked() ), this, SLOT( toggleCollapsed() ) );
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( checkToggled( bool ) ) );
  connect( this, SIGNAL( clicked( bool ) ), this, SLOT( checkClicked( bool ) ) );
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

  // verify triangle mirrors groupbox's enabled state
  mCollapseButton->setEnabled( isEnabled() );

  // set mShown after first setCollapsed call or expanded groupboxes
  // will scroll scroll areas when first shown
  mShown = true;
  event->accept();
}

void QgsCollapsibleGroupBoxBasic::mousePressEvent( QMouseEvent *event )
{
  // avoid leaving checkbox in pressed state if alt- or shift-clicking
  if ( event->modifiers() & ( Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier )
       && titleRect().contains( event->pos() )
       && isCheckable() )
  {
    event->ignore();
    return;
  }

  // default behaviour - pass to QGroupBox
  QGroupBox::mousePressEvent( event );
}

void QgsCollapsibleGroupBoxBasic::mouseReleaseEvent( QMouseEvent *event )
{
  mAltDown = ( event->modifiers() & ( Qt::AltModifier | Qt::ControlModifier ) );
  mShiftDown = ( event->modifiers() & Qt::ShiftModifier );
  mTitleClicked = ( titleRect().contains( event->pos() ) );

  // sync group when title is alt-clicked
  // collapse/expand when title is clicked and non-checkable
  // expand current and collapse others on shift-click
  if ( event->button() == Qt::LeftButton && mTitleClicked &&
       ( mAltDown || mShiftDown || !isCheckable() ) )
  {
    toggleCollapsed();
    return;
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

void QgsCollapsibleGroupBoxBasic::setSyncGroup( QString grp )
{
  mSyncGroup = grp;
  QString tipTxt = QString( "" );
  if ( !grp.isEmpty() )
  {
    tipTxt = tr( "Ctrl (or Alt)-click to toggle all" ) + "\n" + tr( "Shift-click to expand, then collapse others" );
  }
  mCollapseButton->setToolTip( tipTxt );
}

QRect QgsCollapsibleGroupBoxBasic::titleRect() const
{
  QStyleOptionGroupBox box;
  initStyleOption( &box );
  return style()->subControlRect( QStyle::CC_GroupBox, &box,
                                  QStyle::SC_GroupBoxLabel, this );
}

void QgsCollapsibleGroupBoxBasic::clearModifiers()
{
  mCollapseButton->setAltDown( false );
  mCollapseButton->setShiftDown( false );
  mAltDown = false;
  mShiftDown = false;
}

void QgsCollapsibleGroupBoxBasic::checkToggled( bool chkd )
{
  Q_UNUSED( chkd );
  mCollapseButton->setEnabled( true ); // always keep enabled
}

void QgsCollapsibleGroupBoxBasic::checkClicked( bool chkd )
{
  // expand/collapse when checkbox toggled by user click.
  // don't do this on toggle signal, otherwise group boxes will default to collapsed
  // in option dialog constructors, reducing discovery of options by new users and
  // overriding user's auto-saved collapsed/expanded state for the group box
  if ( chkd && isCollapsed() )
    setCollapsed( false );
  else if ( ! chkd && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBoxBasic::toggleCollapsed()
{
  // verify if sender is this group box's collapse button
  bool senderCollBtn = false;
  QgsGroupBoxCollapseButton* collBtn = qobject_cast<QgsGroupBoxCollapseButton*>( QObject::sender() );
  senderCollBtn = ( collBtn && collBtn == mCollapseButton );

  mAltDown = ( mAltDown || mCollapseButton->altDown() );
  mShiftDown = ( mShiftDown || mCollapseButton->shiftDown() );

  // find any sync group siblings and toggle them
  if (( senderCollBtn || mTitleClicked )
      && ( mAltDown || mShiftDown )
      && !mSyncGroup.isEmpty() )
  {
    QgsDebugMsg( "Alt or Shift key down, syncing group" );
    // get pointer to parent or grandparent widget
    if ( parentWidget() )
    {
      mSyncParent = parentWidget();
      if ( mSyncParent->parentWidget() )
      {
        // don't use whole app for grandparent (common for dialogs that use main window for parent)
        if ( mSyncParent->parentWidget()->objectName() != QString( "QgisApp" ) )
        {
          mSyncParent = mSyncParent->parentWidget();
        }
      }
    }
    else
    {
      mSyncParent = 0;
    }

    if ( mSyncParent )
    {
      QgsDebugMsg( "found sync parent: " + mSyncParent->objectName() );

      bool thisCollapsed = mCollapsed; // get state of current box before its changed
      foreach ( QgsCollapsibleGroupBoxBasic *grpbox, mSyncParent->findChildren<QgsCollapsibleGroupBoxBasic*>() )
      {
        if ( grpbox->syncGroup() == syncGroup() && grpbox->isEnabled() )
        {
          if ( mShiftDown && grpbox == dynamic_cast<QgsCollapsibleGroupBoxBasic *>( this ) )
          {
            // expand current group box on shift-click
            setCollapsed( false );
          }
          else
          {
            grpbox->setCollapsed( mShiftDown ? true : !thisCollapsed );
          }
        }
      }

      clearModifiers();
      return;
    }
    else
    {
      QgsDebugMsg( "did not find a sync parent" );
    }
  }

  // expand current group box on shift-click, even if no sync group
  if ( mShiftDown )
  {
    setCollapsed( false );
  }
  else
  {
    setCollapsed( !mCollapsed );
  }

  clearModifiers();
}

void QgsCollapsibleGroupBoxBasic::updateStyle()
{
  setUpdatesEnabled( false );

  QSettings settings;
  // NOTE: QGIS-Style groupbox styled in app stylesheet
  bool usingQgsStyle = settings.value( "qgis/stylesheet/groupBoxCustom", QVariant( false ) ).toBool();

  QStyleOptionGroupBox box;
  initStyleOption( &box );
  QRect rectFrame = style()->subControlRect( QStyle::CC_GroupBox, &box,
                    QStyle::SC_GroupBoxFrame, this );
  QRect rectTitle = titleRect();

  // margin/offset defaults
  int marginLeft = 20;  // title margin for disclosure triangle
  int marginRight = 5;  // a little bit of space on the right, to match space on the left
  int offsetLeft = 0;   // offset for oxygen theme
  int offsetStyle = QApplication::style()->objectName().contains( "macintosh" ) ? ( usingQgsStyle ? 1 : 8 ) : 0;
  int topBuffer = ( usingQgsStyle ? 3 : 1 ) + offsetStyle; // space between top of title or triangle and widget above
  int offsetTop =  topBuffer;
  int offsetTopTri = topBuffer; // offset for triangle

  if ( mCollapseButton->height() < rectTitle.height() ) // triangle's height > title text's, offset triangle
  {
    offsetTopTri += ( rectTitle.height() - mCollapseButton->height() ) / 2 ;
//    offsetTopTri += rectTitle.top();
  }
  else if ( rectTitle.height() < mCollapseButton->height() ) // title text's height < triangle's, offset title
  {
    offsetTop += ( mCollapseButton->height() - rectTitle.height() ) / 2;
  }

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
        offsetTopTri = offsetTop + 1;
      }
      else
      {
        offsetTop = 6 + rectFrame.top();
        offsetTopTri = offsetTop;
      }
    }
  }

  QgsDebugMsg( QString( "groupbox: %1 style: %2 offset: left=%3 top=%4 top2=%5" ).arg(
                 objectName() ).arg( QApplication::style()->objectName() ).arg( offsetLeft ).arg( offsetTop ).arg( offsetTopTri ) );

  // customize style sheet for collapse/expand button and force left-aligned title
  QString ss;
  if ( usingQgsStyle || QApplication::style()->objectName().contains( "macintosh" ) )
  {
    ss += "QgsCollapsibleGroupBoxBasic, QgsCollapsibleGroupBox {";
    ss += QString( "  margin-top: %1px;" ).arg( topBuffer + ( usingQgsStyle ? rectTitle.height() + 5 : rectFrame.top() ) );
    ss += "}";
  }
  ss += "QgsCollapsibleGroupBoxBasic::title, QgsCollapsibleGroupBox::title {";
  ss += "  subcontrol-origin: margin;";
  ss += "  subcontrol-position: top left;";
  ss += QString( "  margin-left: %1px;" ).arg( marginLeft );
  ss += QString( "  margin-right: %1px;" ).arg( marginRight );
  ss += QString( "  left: %1px;" ).arg( offsetLeft );
  ss += QString( "  top: %1px;" ).arg( offsetTop );
  if ( QApplication::style()->objectName().contains( "macintosh" ) )
  {
    ss += "  background-color: rgba(0,0,0,0)";
  }
  ss += "}";
  setStyleSheet( ss );

  // clear toolbutton default background and border and apply offset
  QString ssd;
  ssd = QString( "QgsCollapsibleGroupBoxBasic > QToolButton#%1, QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );
  if ( offsetLeft != 0 || offsetTopTri != 0 )
    mCollapseButton->move( offsetLeft, offsetTopTri );

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
  // NOTE: this causes app to crash when loading a project that hits a group box with
  //       'collapse' set via dynamic property or in code (especially if auto-launching project)
  // TODO: find another means of avoiding the X11 flicker
//  QApplication::processEvents();

  // handle visual fixes for collapsing/expanding
  collapseExpandFixes();

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

void QgsCollapsibleGroupBoxBasic::collapseExpandFixes()
{
  if ( QApplication::style()->objectName().contains( "macintosh" ) )
  {
    // handle QPushButtons in form layouts that stay partly visible on collapse (Qt bug?)
    // hide on collapse for fix, but only show buttons that were specifically hidden when expanding
    // key hiding off of this group box's object name so it does not affect child group boxes
    const QByteArray objKey = QString( "CollGrpBxHiddenButton_%1" ).arg( objectName() ).toUtf8();
    const char* pbHideKey = objKey.constData();

    // handle child group box widgets that don't hide their frames on collapse of parent
    const char* gbHideKey = "CollGrpBxHideGrpBx";

    if ( mCollapsed )
    {
      // first hide all child group boxes, regardless of whether they are collapsible
      foreach ( QGroupBox* gbx, findChildren<QGroupBox *>() )
      {
        if ( gbx->isVisible() && !gbx->property( gbHideKey ).isValid() )
        {
          gbx->setProperty( gbHideKey, QVariant( true ) );
          gbx->hide();
        }
      }

      // hide still visible push buttons belonging to this group box
      foreach ( QPushButton* pBtn, findChildren<QPushButton *>() )
      {
        if ( pBtn->isVisible() && !pBtn->property( pbHideKey ).isValid() )
        {
          pBtn->setProperty( pbHideKey, QVariant( true ) );
          pBtn->hide();
        }
      }
    }
    else // on expand
    {
      // first show push buttons belonging to this group box
      foreach ( QPushButton* pBtn, findChildren<QPushButton *>() )
      {
        if ( pBtn->property( pbHideKey ).isValid() ) // don't have to check bool value
        {
          pBtn->setProperty( pbHideKey, QVariant() ); // remove property
          pBtn->show();
        }
      }

      // show all hidden child group boxes
      foreach ( QGroupBox* gbx, findChildren<QGroupBox *>() )
      {
        if ( gbx->property( gbHideKey ).isValid() ) // don't have to check bool value
        {
          gbx->setProperty( gbHideKey, QVariant() ); // remove property
          gbx->show();
        }
      }
    }
  }
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

