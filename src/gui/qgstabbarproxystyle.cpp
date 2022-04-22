/***************************************************************************
  qgstabbarproxystyle.cpp - QgsTabBarProxyStyle
 ---------------------
 begin                : 25.3.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstabbarproxystyle.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>


///@cond PRIVATE

QgsTabBarProxyStyle::QgsTabBarProxyStyle( QTabBar *tabBar )
  : QProxyStyle()
  , mTabBar( tabBar )
{
}

void QgsTabBarProxyStyle::drawControl( ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
{
  if ( element == CE_TabBarTab && mTabStyles.contains( mTabBar->tabAt( option->rect.center() ) ) )
  {
    painter->save(); // save the painter to restore later
    const TabStyle &style { mTabStyles.value( mTabBar->tabAt( option->rect.center() ) ) };
    painter->setFont( style.font ); // change the defaul font of painter
    QProxyStyle::drawControl( element, option, painter, widget ); // paint the TabBarTab using the default drawControl
    painter->restore(); // restore to default painter
  }
  else
  {
    QProxyStyle::drawControl( element, option, painter, widget ); // use default drawControl to paint all other components without changes.
  }
}

void QgsTabBarProxyStyle::addStyle( int tabIndex, const TabStyle &style )
{
  mTabStyles.insert( tabIndex, style );
}

const QMap<int, QgsTabBarProxyStyle::TabStyle> &QgsTabBarProxyStyle::tabStyles() const
{
  return mTabStyles;
}


QgsTabBar::QgsTabBar( QWidget *parent )
  : QTabBar( parent )
{
}

void QgsTabBar::setTabBarStyle( QgsTabBarProxyStyle *tabStyle )
{
  mTabBarStyle = tabStyle;
}

QSize QgsTabBar::tabSizeHint( int index ) const
{
  if ( mTabBarStyle->tabStyles().contains( index ) )
  {
    const QSize s = QTabBar::tabSizeHint( index );
    const QFontMetrics fm( font() );
    const int w = fm.horizontalAdvance( tabText( index ) );
    const QFont f = mTabBarStyle->tabStyles().value( index ).font;
    const QFontMetrics bfm( f );
    const int bw = bfm.horizontalAdvance( tabText( index ) );
    return QSize( s.width() - w + bw, s.height() );
  }
  else
  {
    return QTabBar::tabSizeHint( index );
  }
}



///@endcond
