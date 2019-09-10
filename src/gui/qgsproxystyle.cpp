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
#include <QStyleFactory>
#include <QStyle>
#include <QStyleOption>
#include <QApplication>

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
      break;

  }
  return QProxyStyle::generatedIconPixmap( iconMode, pixmap, opt );
}

QProxyStyle *QgsAppStyle::clone()
{
  return new QgsAppStyle( mBaseStyle );
}

///@endcond
