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
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance

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

QgsDecorationItem::QgsDecorationItem( QObject* parent )
    : QObject( parent )
{
  mEnabled = false;
}

QgsDecorationItem::~QgsDecorationItem()
{

}

void QgsDecorationItem::update() 
{ 
  saveToProject();
  QgisApp::instance()->mapCanvas()->refresh(); 
}

void QgsDecorationItem::projectRead()
{
  QgsDebugMsg( "Entered" );
  mEnabled = QgsProject::instance()->readBoolEntry( mNameConfig, "/Enabled", false );
}

void QgsDecorationItem::saveToProject()
{
  QgsDebugMsg( "Entered" );
  QgsProject::instance()->writeEntry( mNameConfig, "/Enabled", mEnabled );
}
void QgsDecorationItem::setName( const char *name ) 
{ 
  mName = name; 
  mNameConfig = name;
  mNameConfig.remove( " " );
  mNameTranslated = tr( name );
  QgsDebugMsg( QString( "name=%1 nameconfig=%2 nametrans=%3").arg(mName).arg(mNameConfig).arg(mNameTranslated) );
}
