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

#include <QStyleOptionGroupBox>
#include <QStylePainter>
#include <QLayout>

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
    : QGroupBox( parent ), mCollapsed( false )
{
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( setToggled( bool ) ) );
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title, QWidget *parent )
    : QGroupBox( title, parent ), mCollapsed( false )
{}

void QgsCollapsibleGroupBox::paintEvent( QPaintEvent * event )
{
  QGroupBox::paintEvent( event );

  // paint expand/collapse icon only if groupbox is checkable as icon replaces check box
  if ( ! isCheckable() )
    return;

  // create background mask + expand/collapse icon
  // icons from http://www.iconfinder.com/search/?q=iconset%3AsplashyIcons
  QPixmap icon( mCollapsed ?
                QgsApplication::getThemePixmap( "/mIconExpand.png" ) :
                QgsApplication::getThemePixmap( "/mIconCollapse.png" ) );
  QPixmap background( icon.width() + 2, icon.height() + 2 );
  background.fill( palette().color( backgroundRole() ) );

  // paint on top of checkbox - does this work with all platforms/themes?
  QStylePainter paint( this );
  QStyleOptionGroupBox option;
  initStyleOption( &option );
  paint.drawComplexControl( QStyle::CC_GroupBox, option );
  paint.drawItemPixmap( option.rect.adjusted( 4, -1, 0, 0 ),
                        Qt::AlignTop | Qt::AlignLeft,
                        background );
  paint.drawItemPixmap( option.rect.adjusted( 6, 0, 0, 0 ),
                        Qt::AlignTop | Qt::AlignLeft,
                        icon );
}

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  QGroupBox::showEvent( event );
  // collapse if needed - any calls to setCollapsed() before have no effect
  if ( isCheckable() && ! isChecked() && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  if ( ! isVisible() )
    return;

  // minimize layout margins and save for subsequent restore
  if ( collapse )
  {
    if ( layout() )
    {
      mMargins = layout()->contentsMargins();
      layout()->setContentsMargins( 1, 1, 1, 1 );
    }
  }
  else
  {
    if ( layout() )
    {
      layout()->setContentsMargins( mMargins );
    }
  }

  mCollapsed = collapse;
  foreach ( QWidget *widget, findChildren<QWidget*>() )
    widget->setHidden( collapse );

  if ( mCollapsed )
    emit collapsed( this );
  else
    emit expanded( this );
}

