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

QIcon QgsCollapsibleGroupBox::mCollapseIcon;
QIcon QgsCollapsibleGroupBox::mExpandIcon;

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
    : QGroupBox( parent ), mCollapsed( false )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent ), mCollapsed( false )
{
  init();
}

void QgsCollapsibleGroupBox::init()
{
  // init icons
  if ( mCollapseIcon.isNull() )
  {
    mCollapseIcon = QgsApplication::getThemeIcon( "/mIconCollapse.png" );
    mExpandIcon = QgsApplication::getThemeIcon( "/mIconExpand.png" );
  }

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

  // collapse button
  mCollapseButton = new QToolButton( this );
  mCollapseButton->setObjectName( "collapseButton" );
  mCollapseButton->setAutoRaise( true );
  mCollapseButton->setFixedSize( 16, 16 );
  // TODO set size (as well as margins) depending on theme
  mCollapseButton->setIconSize( QSize( 12, 12 ) );
  mCollapseButton->setIcon( mCollapseIcon );

  // clear toolbutton default background and border
  // TODO: move to app stylesheet system, when appropriate
  QString ssd;
  ssd = QString( "QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );

  connect( mCollapseButton, SIGNAL( clicked() ), this, SLOT( toggleCollapsed() ) );
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( checkToggled( bool ) ) );
}

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  QGroupBox::showEvent( event );
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
}

void QgsCollapsibleGroupBox::mouseReleaseEvent( QMouseEvent *event )
{
  // catch mouse release over title when non checkable, to collapse/expand
  if ( !isCheckable() && event->button() == Qt::LeftButton )
  {
    QStyleOptionGroupBox box;
    initStyleOption( &box );
    QRect rect = style()->subControlRect( QStyle::CC_GroupBox, &box,
                                          QStyle::SC_GroupBoxLabel, this );
    if ( rect.contains( event->pos() ) )
    {
      toggleCollapsed();
      return;
    }
  }
  // default behaviour - pass to QGroupBox
  QGroupBox::mouseReleaseEvent( event );
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

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  mCollapsed = collapse;

  if ( !isVisible() )
    return;

  // for consistent look/spacing across platforms when collapsed
  setFlat( collapse );
  // avoid flicker in X11
  QApplication::processEvents();
  // set maximum height to 25 to hide contents - does this work in all envs?
  setMaximumHeight( collapse ? 25 : 16777215 );
  mCollapseButton->setIcon( collapse ? mExpandIcon : mCollapseIcon );

  emit collapsedStateChanged( this );
}

