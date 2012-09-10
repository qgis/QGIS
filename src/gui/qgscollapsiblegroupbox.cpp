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

QIcon QgsCollapsibleGroupBox::mCollapseIcon;
QIcon QgsCollapsibleGroupBox::mExpandIcon;

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
  : QGroupBox( parent ), mCollapsed( false ), mSaveState( true )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent ), mCollapsed( false ), mSaveState( true )
{
  init();
}

QgsCollapsibleGroupBox::~QgsCollapsibleGroupBox()
{
  saveState();
}

void QgsCollapsibleGroupBox::init()
{
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
  loadState();

  updateStyle();

  // expand if needed - any calls to setCollapsed() before only set mCollapsed
  if ( mCollapsed )
  {
    setCollapsed( mCollapsed );
  }
  else
  {
    /* manually expanding (already default) on show may scroll scroll areas;
       still emit signal for connections using expanded state */
    emit collapsedStateChanged( this );
  }
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
  saveKey = "/" + window()->objectName() + saveKey;
  saveKey = "QgsCollapsibleGroupBox" + saveKey;
  return saveKey;
}

void QgsCollapsibleGroupBox::loadState()
{
  if ( ! mSaveState ) 
    return;
  
  setUpdatesEnabled( false );

  QSettings settings;
  QString key = saveKey();
  QVariant val = settings.value( key + "/checked" );
  if ( ! val.isNull() )
    setChecked( val.toBool() );
  val = settings.value( key + "/collapsed" );
  if ( ! val.isNull() )
    setCollapsed( val.toBool() );

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBox::saveState()
{
  if ( ! mSaveState ) 
    return;
  QgsDebugMsg( "key = " + saveKey() + " objectName = " + objectName() );
  QSettings settings;
  QString key = saveKey();
  settings.setValue( key + "/checked", isChecked() );
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

  // customize style sheet
  // TODO: move to app stylesheet system, when appropriate
  QString ss;
  ss += "QgsCollapsibleGroupBox::title {";
  ss += "  subcontrol-origin: margin;";
  ss += "  subcontrol-position: top left;";
  ss += "  margin-left: 20px;";  // offset for disclosure triangle
  ss += "  margin-right: 5px;";  // a little bit of space on the right, to match space on the left
  ss += "}";
  setStyleSheet( ss );

  // clear toolbutton default background and border
  // TODO: move to app stylesheet system, when appropriate
  QString ssd;
  ssd = QString( "QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );

  setUpdatesEnabled( true );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  mCollapsed = collapse;

  if ( !isVisible() )
    return;

  // for consistent look/spacing across platforms when collapsed
  setFlat( collapse );
  // avoid flicker in X11
  QApplication::processEvents();
  // set maximum height to hide contents - does this work in all envs?
  // setMaximumHeight( collapse ? 25 : 16777215 );
  setMaximumHeight( collapse ? titleRect().bottom() + 2 : 16777215 );
  mCollapseButton->setIcon( collapse ? mExpandIcon : mCollapseIcon );

  emit collapsedStateChanged( this );
}

