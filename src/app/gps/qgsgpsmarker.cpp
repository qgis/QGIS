/***************************************************************************
    qgsgpsmarker.cpp  - canvas item which shows a gps position marker
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Tim Sutton
    email                : tim at linfiniti dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>
#include <QObject>

#include "qgsgpsmarker.h"
#include "moc_qgsgpsmarker.cpp"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsgui.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"


const QgsSettingsEntryString *QgsGpsMarker::settingLocationMarkerSymbol = new QgsSettingsEntryString( QStringLiteral( "location-marker-symbol" ), QgsSettingsTree::sTreeGps, QStringLiteral( R"""(<symbol alpha="1" frame_rate="10" is_animated="0" name="gps_marker" force_rhr="0" clip_to_extent="1" type="marker"> <data_defined_properties>  <Option type="Map">   <Option value="" name="name" type="QString"/>   <Option name="properties"/>   <Option value="collection" name="type" type="QString"/>  </Option> </data_defined_properties> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="square" name="cap_style" type="QString"/>   <Option value="35,35,35,255" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="round" name="joinstyle" type="QString"/>   <Option value="triangle" name="name" type="QString"/>   <Option value="0,-3.80000000000000115" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="255,255,255,255" name="outline_color" type="QString"/>   <Option value="solid" name="outline_style" type="QString"/>   <Option value="0.4" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="2" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="square"/>  <prop k="color" v="35,35,35,255"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="round"/>  <prop k="name" v="triangle"/>  <prop k="offset" v="0,-3.80000000000000115"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="255,255,255,255"/>  <prop k="outline_style" v="solid"/>  <prop k="outline_width" v="0.4"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="2"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="square" name="cap_style" type="QString"/>   <Option value="190,178,151,0" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="bevel" name="joinstyle" type="QString"/>   <Option value="circle" name="name" type="QString"/>   <Option value="0,0" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="255,255,255,255" name="outline_color" type="QString"/>   <Option value="solid" name="outline_style" type="QString"/>   <Option value="1.22" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="5.8" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="square"/>  <prop k="color" v="190,178,151,0"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="bevel"/>  <prop k="name" v="circle"/>  <prop k="offset" v="0,0"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="255,255,255,255"/>  <prop k="outline_style" v="solid"/>  <prop k="outline_width" v="1.22"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="5.8"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="square" name="cap_style" type="QString"/>   <Option value="190,178,151,0" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="bevel" name="joinstyle" type="QString"/>   <Option value="circle" name="name" type="QString"/>   <Option value="0,0" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="35,35,35,255" name="outline_color" type="QString"/>   <Option value="solid" name="outline_style" type="QString"/>   <Option value="0.8" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="5.8" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="square"/>  <prop k="color" v="190,178,151,0"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="bevel"/>  <prop k="name" v="circle"/>  <prop k="offset" v="0,0"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="35,35,35,255"/>  <prop k="outline_style" v="solid"/>  <prop k="outline_width" v="0.8"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="5.8"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="round" name="cap_style" type="QString"/>   <Option value="190,178,151,0" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="round" name="joinstyle" type="QString"/>   <Option value="cross" name="name" type="QString"/>   <Option value="0,0" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="255,255,255,255" name="outline_color" type="QString"/>   <Option value="solid" name="outline_style" type="QString"/>   <Option value="0.92" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="2.8" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="round"/>  <prop k="color" v="190,178,151,0"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="round"/>  <prop k="name" v="cross"/>  <prop k="offset" v="0,0"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="255,255,255,255"/>  <prop k="outline_style" v="solid"/>  <prop k="outline_width" v="0.92"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="2.8"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="round" name="cap_style" type="QString"/>   <Option value="190,178,151,0" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="round" name="joinstyle" type="QString"/>   <Option value="cross" name="name" type="QString"/>   <Option value="0,0" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="35,35,35,255" name="outline_color" type="QString"/>   <Option value="solid" name="outline_style" type="QString"/>   <Option value="0.46" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="2.8" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="round"/>  <prop k="color" v="190,178,151,0"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="round"/>  <prop k="name" v="cross"/>  <prop k="offset" v="0,0"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="35,35,35,255"/>  <prop k="outline_style" v="solid"/>  <prop k="outline_width" v="0.46"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="2.8"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer> <layer enabled="1" class="SimpleMarker" pass="0" locked="0">  <Option type="Map">   <Option value="0" name="angle" type="QString"/>   <Option value="square" name="cap_style" type="QString"/>   <Option value="35,35,35,255" name="color" type="QString"/>   <Option value="1" name="horizontal_anchor_point" type="QString"/>   <Option value="bevel" name="joinstyle" type="QString"/>   <Option value="triangle" name="name" type="QString"/>   <Option value="0,-3.80000000000000115" name="offset" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="offset_map_unit_scale" type="QString"/>   <Option value="MM" name="offset_unit" type="QString"/>   <Option value="35,35,35,255" name="outline_color" type="QString"/>   <Option value="no" name="outline_style" type="QString"/>   <Option value="0" name="outline_width" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="outline_width_map_unit_scale" type="QString"/>   <Option value="MM" name="outline_width_unit" type="QString"/>   <Option value="diameter" name="scale_method" type="QString"/>   <Option value="2" name="size" type="QString"/>   <Option value="3x:0,0,0,0,0,0" name="size_map_unit_scale" type="QString"/>   <Option value="MM" name="size_unit" type="QString"/>   <Option value="1" name="vertical_anchor_point" type="QString"/>  </Option>  <prop k="angle" v="0"/>  <prop k="cap_style" v="square"/>  <prop k="color" v="35,35,35,255"/>  <prop k="horizontal_anchor_point" v="1"/>  <prop k="joinstyle" v="bevel"/>  <prop k="name" v="triangle"/>  <prop k="offset" v="0,-3.80000000000000115"/>  <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="offset_unit" v="MM"/>  <prop k="outline_color" v="35,35,35,255"/>  <prop k="outline_style" v="no"/>  <prop k="outline_width" v="0"/>  <prop k="outline_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="outline_width_unit" v="MM"/>  <prop k="scale_method" v="diameter"/>  <prop k="size" v="2"/>  <prop k="size_map_unit_scale" v="3x:0,0,0,0,0,0"/>  <prop k="size_unit" v="MM"/>  <prop k="vertical_anchor_point" v="1"/>  <data_defined_properties>   <Option type="Map">    <Option value="" name="name" type="QString"/>    <Option name="properties"/>    <Option value="collection" name="type" type="QString"/>   </Option>  </data_defined_properties> </layer></symbol>)""" ), QStringLiteral( "Marker symbol to use for GPS location marker" ), Qgis::SettingsOptions(), 0 );

const QgsSettingsEntryBool *QgsGpsMarker::settingShowLocationMarker = new QgsSettingsEntryBool( QStringLiteral( "show-location-marker" ), QgsSettingsTree::sTreeGps, true, QStringLiteral( "Whether to show the current GPS location marker" ) );

const QgsSettingsEntryBool *QgsGpsMarker::settingRotateLocationMarker = new QgsSettingsEntryBool( QStringLiteral( "rotate-location-marker" ), QgsSettingsTree::sTreeGps, true, QStringLiteral( "Whether to rotate GPS marker symbol to follow GPS bearing" ), Qgis::SettingsOptions() );

QgsGpsMarker::QgsGpsMarker( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasMarkerSymbolItem( mapCanvas )
{
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  updateMarkerSymbol();

  QObject::connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsGpsMarker::updateMarkerSymbol );

  setZValue( 200 );
}

QgsGpsMarker::~QgsGpsMarker() = default;

void QgsGpsMarker::setGpsPosition( const QgsPointXY &point )
{
  //transform to map crs
  if ( mMapCanvas )
  {
    const QgsCoordinateTransform t( mWgs84CRS, mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      mCenter = t.transform( point );
    }
    catch ( QgsCsException &e ) //silently ignore transformation exceptions
    {
      QgsMessageLog::logMessage( QObject::tr( "Error transforming the map center point: %1" ).arg( e.what() ), QStringLiteral( "GPS" ), Qgis::MessageLevel::Warning );
      return;
    }
  }
  else
  {
    mCenter = point;
  }

  setPointLocation( mCenter );
  updateSize();
}

void QgsGpsMarker::setMarkerRotation( double rotation )
{
  QgsMarkerSymbol *renderedMarker = qgis::down_cast<QgsMarkerSymbol *>( symbol() );
  if ( !settingRotateLocationMarker->value() )
  {
    renderedMarker->setAngle( mMarkerSymbol->angle() );
  }
  else
  {
    renderedMarker->setAngle( mMarkerSymbol->angle() + rotation + mMapCanvas->rotation() );
  }
  updateSize();
}

void QgsGpsMarker::updateMarkerSymbol()
{
  const QString defaultSymbol = QgsGpsMarker::settingLocationMarkerSymbol->value();
  QDomDocument symbolDoc;
  symbolDoc.setContent( defaultSymbol );
  const QDomElement markerElement = symbolDoc.documentElement();
  mMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( markerElement, QgsReadWriteContext() ) );
  setSymbol( std::unique_ptr<QgsMarkerSymbol>( mMarkerSymbol->clone() ) );
  updateSize();
}
