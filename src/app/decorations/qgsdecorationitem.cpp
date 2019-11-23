/***************************************************************************
                         qgsdecorationitem.cpp
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationitem.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapdecoration.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h" //for pointOnLineWithDistance
#include "qgsunittypes.h"

#include <QPainter>
#include <QAction>
#include <QPen>
#include <QPolygon>
#include <QString>
#include <QFontMetrics>
#include <QFont>
#include <QColor>
#include <QMenu>
#include <QFile>
#include <QLocale>

//non qt includes
#include <cmath>

QgsDecorationItem::QgsDecorationItem( QObject *parent )
  : QObject( parent )
{
}

void QgsDecorationItem::update()
{
  saveToProject();
  QgisApp::instance()->mapCanvas()->refresh();
}

void QgsDecorationItem::projectRead()
{
  mEnabled = QgsProject::instance()->readBoolEntry( mNameConfig, QStringLiteral( "/Enabled" ), false );
  mPlacement = static_cast< Placement >( QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/Placement" ), static_cast< int >( mPlacement ) ) );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/MarginUnit" ), QgsUnitTypes::encodeUnit( mMarginUnit ) ) );
}

void QgsDecorationItem::saveToProject()
{
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Enabled" ), mEnabled );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Placement" ), static_cast< int >( mPlacement ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginUnit" ), QgsUnitTypes::encodeUnit( mMarginUnit ) );
}

void QgsDecorationItem::setName( const char *name )
{
  mName = name;
  mNameConfig = name;
  mNameConfig.remove( ' ' );
  mNameTranslated = tr( name );
  QgsDebugMsgLevel( QStringLiteral( "name=%1 nameconfig=%2 nametrans=%3" ).arg( mName, mNameConfig, mNameTranslated ), 3 );
}
