/***************************************************************************
  qgsquickplugin.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qqml.h>

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsvectorlayer.h"

#include "qgsquickmapcanvasmap.h"
#include "qgsquickmapsettings.h"
#include "qgsquickplugin.h"
#include "qgsquickutils.h"

static QObject *_utilsProvider( QQmlEngine *engine, QJSEngine *scriptEngine )
{
  Q_UNUSED( engine )
  Q_UNUSED( scriptEngine )
  return new QgsQuickUtils();  // the object will be owned by QML engine and destroyed by the engine on exit
}

void QgisQuickPlugin::registerTypes( const char *uri )
{
  qRegisterMetaType< QList<QgsMapLayer *> >( "QList<QgsMapLayer*>" );
  qRegisterMetaType< QgsAttributes > ( "QgsAttributes" );
  qRegisterMetaType< QgsCoordinateReferenceSystem >( "QgsCoordinateReferenceSystem" );
  qRegisterMetaType< QgsCoordinateTransformContext >( "QgsCoordinateTransformContext" );
  qRegisterMetaType< QgsFeature > ( "QgsFeature " );
  qRegisterMetaType< QgsFeatureId > ( "QgsFeatureId" );
  qRegisterMetaType< QgsPoint >( "QgsPoint" );
  qRegisterMetaType< QgsPointXY >( "QgsPointXY" );

  qmlRegisterType< QgsProject >( uri, 0, 1, "Project" );
  qmlRegisterType< QgsQuickMapCanvasMap >( uri, 0, 1, "MapCanvasMap" );
  qmlRegisterType< QgsQuickMapSettings >( uri, 0, 1, "MapSettings" );
  qmlRegisterType< QgsVectorLayer >( uri, 0, 1, "VectorLayer" );

  qmlRegisterSingletonType< QgsQuickUtils >( uri, 0, 1, "Utils", _utilsProvider );

  qmlRegisterUncreatableType< QgsRelationManager >( uri, 0, 1, "RelationManager", "The relation manager is available from the Project. Try `qgisProject.relationManager`" );
  qmlRegisterUncreatableType< QgsMessageLog >( uri, 0, 1, "QgsMessageLog", "Expose MessageLevel" );
}

