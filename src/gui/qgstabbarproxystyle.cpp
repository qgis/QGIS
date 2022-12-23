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
  : QgsProxyStyle( tabBar )
{
}

void QgsTabBarProxyStyle::drawControl( ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
{

  QTabBar *tabBar { qobject_cast<QTabBar *>( parent() ) };

  if ( tabBar )
  {
    if ( element == CE_TabBarTab && mTabStyles.contains( tabBar->tabAt( option->rect.center() ) ) )
    {
      if ( const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>( option ) )
      {
        painter->save();
        const QgsAttributeEditorElement::LabelStyle &style { mTabStyles.value( tabBar->tabAt( option->rect.center() ) ) };
        if ( style.overrideFont )
        {
          painter->setFont( style.font );
        }
        QStyleOptionTab opt { *tab };
        if ( style.overrideColor && style.color.isValid( ) )
        {
          opt.palette.setBrush( QPalette::WindowText, style.color );
        }
        QProxyStyle::drawControl( element, &opt, painter, widget );
        painter->restore();
        return;
      }
    }
  }

  QProxyStyle::drawControl( element, option, painter, widget );

}

void QgsTabBarProxyStyle::addStyle( int tabIndex, const QgsAttributeEditorElement::LabelStyle &style )
{
  mTabStyles.insert( tabIndex, style );
}

const QMap<int, QgsAttributeEditorElement::LabelStyle> &QgsTabBarProxyStyle::tabStyles() const
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
