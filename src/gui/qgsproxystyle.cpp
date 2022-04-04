/***************************************************************************
  qgsproxystyle.cpp
  -----------------
  Date                 : March 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproxystyle.h"
#include "qgsimageoperation.h"
#include "qgis.h"
#include <QStyleFactory>
#include <QStyle>
#include <QStyleOption>
#include <QApplication>
#include <QWindow>

QgsProxyStyle::QgsProxyStyle( QWidget *parent )
  : QProxyStyle( nullptr ) // no base style yet - it transfers ownership, so we need a NEW QStyle object for the base style
{
  // get application style
  const QString appStyle = QApplication::style()->objectName();
  if ( appStyle == QLatin1String( "QgsAppStyle" ) )
  {
    setBaseStyle( static_cast< QgsAppStyle * >( QApplication::style() )->clone() );
  }
  else if ( !appStyle.isEmpty() )
  {
    if ( QStyle *style = QStyleFactory::create( appStyle ) )
      setBaseStyle( style );
  }

  // set lifetime to match parent widget's
  setParent( parent );
}

///@cond PRIVATE

//
// QgsAppStyle
//

QgsAppStyle::QgsAppStyle( const QString &base )
  : QProxyStyle( nullptr ) // no base style yet - it transfers ownership, so we need a NEW QStyle object for the base style
  , mBaseStyle( base )
{
  if ( !mBaseStyle.isEmpty() )
  {
    if ( QStyle *style = QStyleFactory::create( mBaseStyle ) )
      setBaseStyle( style );
  }

  setObjectName( QStringLiteral( "QgsAppStyle" ) );
}

QPixmap QgsAppStyle::generatedIconPixmap( QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt ) const
{
  switch ( iconMode )
  {
    case QIcon::Disabled:
    {
      if ( !pixmap.isNull() )
      {
        // override disabled icon style, with something which works better across different light/dark themes.
        // the default Qt style here only works nicely for light themes.
        QImage im = pixmap.toImage().convertToFormat( QImage::Format_ARGB32 );
        QgsImageOperation::adjustHueSaturation( im, 0.2 );
        QgsImageOperation::multiplyOpacity( im, 0.3 );
        return QPixmap::fromImage( im );
      }
      break;
    }

    case QIcon::Normal:
    case QIcon::Active:
    case QIcon::Selected:
      return pixmap;
  }
  BUILTIN_UNREACHABLE
  return QProxyStyle::generatedIconPixmap( iconMode, pixmap, opt );
}

void QgsAppStyle::polish( QWidget *widget )
{
  QProxyStyle::polish( widget );
#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
  if ( mBaseStyle.contains( QLatin1String( "fusion" ), Qt::CaseInsensitive )
       || mBaseStyle.contains( QLatin1String( "adwaita" ), Qt::CaseInsensitive ) )
  {
    // fix broken inactive window coloring applying to unfocused docks or list/tree widgets
    // see eg https://github.com/FedoraQt/adwaita-qt/issues/126
    // the detection used by themes to determine if a widget belongs to an activated window is fragile, which
    // results in unfocused list/tree views or widget children being shown in the "deactivated window" palette coloring.
    // Gnome (adwaita) defaults to a coloring which makes widgets looks disabled in this inactive state.
    // So the best we can do here is force disable the inactive palette coloring to prevent this unwanted behavior.
    QPalette pal = widget->palette();
    pal.setColor( QPalette::Inactive, QPalette::Text, pal.color( QPalette::Active, QPalette::Text ) );
    pal.setColor( QPalette::Inactive, QPalette::Window, pal.color( QPalette::Active, QPalette::Window ) );
    pal.setColor( QPalette::Inactive, QPalette::WindowText, pal.color( QPalette::Active, QPalette::WindowText ) );
    pal.setColor( QPalette::Inactive, QPalette::Button, pal.color( QPalette::Active, QPalette::Button ) );
    pal.setColor( QPalette::Inactive, QPalette::ButtonText, pal.color( QPalette::Active, QPalette::ButtonText ) );
    widget->setPalette( pal );
  }
#endif
}

QProxyStyle *QgsAppStyle::clone()
{
  return new QgsAppStyle( mBaseStyle );
}

///@endcond
