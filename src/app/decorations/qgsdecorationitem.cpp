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
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QAction>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QLocale>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QPolygon>
#include <QString>

#include "moc_qgsdecorationitem.cpp"

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
  mEnabled = QgsProject::instance()->readBoolEntry( mConfigurationName, u"/Enabled"_s, false );
  mPlacement = static_cast<Placement>( QgsProject::instance()->readNumEntry( mConfigurationName, u"/Placement"_s, static_cast<int>( mPlacement ) ) );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( QgsProject::instance()->readEntry( mConfigurationName, u"/MarginUnit"_s, QgsUnitTypes::encodeUnit( mMarginUnit ) ) );
}

void QgsDecorationItem::saveToProject()
{
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Enabled"_s, mEnabled );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Placement"_s, static_cast<int>( mPlacement ) );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/MarginUnit"_s, QgsUnitTypes::encodeUnit( mMarginUnit ) );
}
