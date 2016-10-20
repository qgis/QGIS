/***************************************************************************
  qgsprojectsnappingsettings.cpp - QgsProjectSnappingSettings

 ---------------------
 begin                : 29.8.2016
 copyright            : (C) 2016 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssnappingconfig.h"

#include <QDomElement>
#include <QHeaderView>
#include <QSettings>

#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

QgsSnappingConfig::IndividualLayerSettings::IndividualLayerSettings()
    : mValid( false )
    , mEnabled( false )
    , mType( Vertex )
    , mTolerance( 0 )
    , mUnits( QgsTolerance::Pixels )
{}


QgsSnappingConfig::IndividualLayerSettings::IndividualLayerSettings( bool enabled, SnappingType type, double tolerance, QgsTolerance::UnitType units )
    : mValid( true )
    , mEnabled( enabled )
    , mType( type )
    , mTolerance( tolerance )
    , mUnits( units )
{}

bool QgsSnappingConfig::IndividualLayerSettings::valid() const
{
  return mValid;
}

bool QgsSnappingConfig::IndividualLayerSettings::enabled() const
{
  return mEnabled;
}

void QgsSnappingConfig::IndividualLayerSettings::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

QgsSnappingConfig::SnappingType QgsSnappingConfig::IndividualLayerSettings::type() const
{
  return mType;
}

void QgsSnappingConfig::IndividualLayerSettings::setType( SnappingType type )
{
  mType = type;
}

double QgsSnappingConfig::IndividualLayerSettings::tolerance() const
{
  return mTolerance;
}

void QgsSnappingConfig::IndividualLayerSettings::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

QgsTolerance::UnitType QgsSnappingConfig::IndividualLayerSettings::units() const
{
  return mUnits;
}

void QgsSnappingConfig::IndividualLayerSettings::setUnits( QgsTolerance::UnitType units )
{
  mUnits = units;
}

bool QgsSnappingConfig::IndividualLayerSettings::operator !=( const QgsSnappingConfig::IndividualLayerSettings& other ) const
{
  return mValid != other.mValid
         || mEnabled != other.mEnabled
         || mType != other.mType
         || mTolerance != other.mTolerance
         || mUnits != other.mUnits;
}

bool QgsSnappingConfig::IndividualLayerSettings::operator ==( const QgsSnappingConfig::IndividualLayerSettings& other ) const
{
  return mValid == other.mValid
         && mEnabled == other.mEnabled
         && mType == other.mType
         && mTolerance == other.mTolerance
         && mUnits == other.mUnits;
}


QgsSnappingConfig::QgsSnappingConfig()
{
  reset();
}

QgsSnappingConfig::~QgsSnappingConfig()
{
}

bool QgsSnappingConfig::operator==( const QgsSnappingConfig& other ) const
{
  return mEnabled == other.mEnabled
         && mMode == other.mMode
         && mType == other.mType
         && mTolerance == other.mTolerance
         && mUnits == other.mUnits
         && mIntersectionSnapping == other.mIntersectionSnapping
         && mIndividualLayerSettings == other.mIndividualLayerSettings;
}

void QgsSnappingConfig::reset()
{
  // get defaults values. They are both used for standard and advanced configuration (per layer)
  bool enabled = QSettings().value( "/qgis/digitizing/default_advanced_snap_enabled", true ).toBool();
  SnappingMode mode = ( SnappingMode )QSettings().value( "/qgis/digitizing/default_snap_mode", ( int )AllLayers ).toInt();
  if ( mMode == 0 )
  {
    // backward compatibility with QGIS 2.x
    // could be removed in 3.4+
    mMode = AllLayers;
  }
  SnappingType type = ( SnappingType )QSettings().value( "/qgis/digitizing/default_snap_type", ( int )Vertex ).toInt();
  double tolerance = QSettings().value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  QgsTolerance::UnitType units = ( QgsTolerance::UnitType )QSettings().value( "/qgis/digitizing/default_snapping_tolerance_unit", ( int )QgsTolerance::ProjectUnits ).toInt();

  // assign main (standard) config
  mEnabled = enabled;
  mMode = mode;
  mType = type;
  mTolerance = tolerance;
  // do not allow unit to be "layer" if not in advanced configuration
  if ( mMode != AdvancedConfiguration )
  {
    mUnits = QgsTolerance::ProjectUnits;
  }
  else
  {
    mUnits = units;
  }
  mIntersectionSnapping = false;

  // set advanced config
  mIndividualLayerSettings = QHash<QgsVectorLayer *, IndividualLayerSettings>();
  Q_FOREACH ( QgsMapLayer *ml, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( ml );
    if ( vl )
    {
      mIndividualLayerSettings.insert( vl, IndividualLayerSettings( enabled, type, tolerance, units ) );
    }
  }
}

bool QgsSnappingConfig::enabled() const
{
  return mEnabled;
}

void QgsSnappingConfig::setEnabled( bool enabled )
{
  if ( mEnabled == enabled )
  {
    return;
  }
  mEnabled = enabled;
}

QgsSnappingConfig::SnappingMode QgsSnappingConfig::mode() const
{
  return mMode;
}

void QgsSnappingConfig::setMode( QgsSnappingConfig::SnappingMode mode )
{
  if ( mMode == mode )
  {
    return;
  }
  mMode = mode;
}

QgsSnappingConfig::SnappingType QgsSnappingConfig::type() const
{
  return mType;
}

void QgsSnappingConfig::setType( QgsSnappingConfig::SnappingType type )
{
  if ( mType == type )
  {
    return;
  }
  mType = type;
}

double QgsSnappingConfig::tolerance() const
{
  return mTolerance;
}

void QgsSnappingConfig::setTolerance( double tolerance )
{
  if ( mTolerance == tolerance )
  {
    return;
  }
  mTolerance = tolerance;
}

QgsTolerance::UnitType QgsSnappingConfig::units() const
{
  return mUnits;
}

void QgsSnappingConfig::setUnits( QgsTolerance::UnitType units )
{
  if ( mUnits == units )
  {
    return;
  }
  mUnits = units;
}

bool QgsSnappingConfig::intersectionSnapping() const
{
  return mIntersectionSnapping;
}

void QgsSnappingConfig::setIntersectionSnapping( bool enabled )
{
  mIntersectionSnapping = enabled;
}

QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings> QgsSnappingConfig::individualLayerSettings() const
{
  return mIndividualLayerSettings;
}

QgsSnappingConfig::IndividualLayerSettings QgsSnappingConfig::individualLayerSettings( QgsVectorLayer* vl ) const
{
  if ( vl && mIndividualLayerSettings.contains( vl ) )
  {
    return mIndividualLayerSettings.value( vl );
  }
  else
  {
    // return invalid settings
    return IndividualLayerSettings();
  }
}

void QgsSnappingConfig::setIndividualLayerSettings( QgsVectorLayer* vl, IndividualLayerSettings individualLayerSettings )
{
  if ( !vl || mIndividualLayerSettings.value( vl ) == individualLayerSettings )
  {
    return;
  }
  mIndividualLayerSettings.insert( vl, individualLayerSettings );
}

bool QgsSnappingConfig::operator!=( const QgsSnappingConfig& other ) const
{
  return mEnabled != other.mEnabled
         || mMode != other.mMode
         || mType != other.mType
         || mTolerance != other.mTolerance
         || mUnits != other.mUnits
         || mIndividualLayerSettings != other.mIndividualLayerSettings;
}

void QgsSnappingConfig::readProject( const QDomDocument& doc )
{
  QDomElement snapSettingsElem = doc.firstChildElement( "qgis" ).firstChildElement( "snapping-settings" );
  if ( snapSettingsElem.isNull() )
  {
    readLegacySettings();
    return;
  }

  if ( snapSettingsElem.hasAttribute( "enabled" ) )
    mEnabled = snapSettingsElem.attribute( "enabled" ) == "1";

  if ( snapSettingsElem.hasAttribute( "mode" ) )
    mMode = ( SnappingMode )snapSettingsElem.attribute( "mode" ).toInt();

  if ( snapSettingsElem.hasAttribute( "type" ) )
    mType = ( SnappingType )snapSettingsElem.attribute( "type" ).toInt();

  if ( snapSettingsElem.hasAttribute( "tolerance" ) )
    mTolerance = snapSettingsElem.attribute( "tolerance" ).toDouble();

  if ( snapSettingsElem.hasAttribute( "unit" ) )
    mUnits = ( QgsTolerance::UnitType )snapSettingsElem.attribute( "unit" ).toInt();

  if ( snapSettingsElem.hasAttribute( "intersection-snapping" ) )
    mIntersectionSnapping = snapSettingsElem.attribute( "intersection-snapping" ) == "1";

  // do not clear the settings as they must be automatically synchronized with current layers
  QDomNodeList nodes = snapSettingsElem.elementsByTagName( "individual-layer-settings" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    QDomNodeList settingNodes = node.childNodes();
    int layerCount = settingNodes.count();
    for ( int i = 0; i < layerCount; ++i )
    {
      QDomElement settingElement = settingNodes.at( i ).toElement();
      if ( settingElement.tagName() != "layer-setting" )
      {
        QgsLogger::warning( QApplication::translate( "QgsProjectSnappingSettings", "Cannot read individual settings. Unexpected tag '%1'" ).arg( settingElement.tagName() ) );
        continue;
      }

      QString layerId = settingElement.attribute( "id" );
      bool enabled = settingElement.attribute( "enabled" ) == "1";
      SnappingType type = ( SnappingType )settingElement.attribute( "type" ).toInt();
      double tolerance = settingElement.attribute( "tolerance" ).toDouble();
      QgsTolerance::UnitType units = ( QgsTolerance::UnitType )settingElement.attribute( "units" ).toInt();

      QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
      if ( !ml || ml->type() != QgsMapLayer::VectorLayer )
        continue;

      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );

      IndividualLayerSettings setting = IndividualLayerSettings( enabled, type, tolerance, units );
      mIndividualLayerSettings.insert( vl, setting );
    }
  }
}

void QgsSnappingConfig::writeProject( QDomDocument& doc )
{
  QDomElement snapSettingsElem = doc.createElement( "snapping-settings" );
  snapSettingsElem.setAttribute( "enabled", QString::number( mEnabled ) );
  snapSettingsElem.setAttribute( "mode", ( int )mMode );
  snapSettingsElem.setAttribute( "type", ( int )mType );
  snapSettingsElem.setAttribute( "tolerance", mTolerance );
  snapSettingsElem.setAttribute( "unit", ( int )mUnits );
  snapSettingsElem.setAttribute( "intersection-snapping", QString::number( mIntersectionSnapping ) );

  QDomElement ilsElement = doc.createElement( "individual-layer-settings" );
  Q_FOREACH ( QgsVectorLayer* vl, mIndividualLayerSettings.keys() )
  {
    IndividualLayerSettings setting = mIndividualLayerSettings.value( vl );

    QDomElement layerElement = doc.createElement( "layer-setting" );
    layerElement.setAttribute( "id", vl->id() );
    layerElement.setAttribute( "enabled", QString::number( setting.enabled() ) );
    layerElement.setAttribute( "type", ( int )setting.type() );
    layerElement.setAttribute( "tolerance", setting.tolerance() );
    layerElement.setAttribute( "units", ( int )setting.units() );
    ilsElement.appendChild( layerElement );
  }
  snapSettingsElem.appendChild( ilsElement );

  doc.firstChildElement( "qgis" ).appendChild( snapSettingsElem );
}

bool QgsSnappingConfig::addLayers( const QList<QgsMapLayer*>& layers )
{
  bool changed = false;
  bool enabled = QSettings().value( "/qgis/digitizing/default_advanced_snap_enabled", true ).toBool();
  SnappingType type = ( SnappingType )QSettings().value( "/qgis/digitizing/default_snap_type", Vertex ).toInt();
  double tolerance = QSettings().value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  QgsTolerance::UnitType units = ( QgsTolerance::UnitType )QSettings().value( "/qgis/digitizing/default_snapping_tolerance_unit", QgsTolerance::ProjectUnits ).toInt();

  Q_FOREACH ( QgsMapLayer* ml, layers )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );
    if ( vl )
    {
      mIndividualLayerSettings.insert( vl, IndividualLayerSettings( enabled, type, tolerance, units ) );
      changed = true;
    }
  }
  return changed;
}

bool QgsSnappingConfig::removeLayers( const QList<QgsMapLayer*>& layers )
{
  bool changed = false;
  Q_FOREACH ( QgsMapLayer* ml, layers )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );
    if ( vl )
    {
      mIndividualLayerSettings.remove( vl );
      changed = true;
    }
  }
  return changed;
}

void QgsSnappingConfig::readLegacySettings()
{
  mMode = ActiveLayer;
  mIndividualLayerSettings.clear();

  QString snapMode = QgsProject::instance()->readEntry( "Digitizing", "/SnappingMode" );

  QString snapType = QgsProject::instance()->readEntry( "Digitizing", "/DefaultSnapType", QString( "off" ) );
  if ( snapType == "to segment" )
    mType = Segment;
  else if ( snapType == "to vertex and segment" )
    mType = VertexAndSegment;
  else if ( snapType == "to vertex" )
    mType = Vertex;
  mTolerance = QgsProject::instance()->readDoubleEntry( "Digitizing", "/DefaultSnapTolerance", 0 );
  mUnits =  static_cast< QgsTolerance::UnitType >( QgsProject::instance()->readNumEntry( "Digitizing", "/DefaultSnapToleranceUnit", QgsTolerance::ProjectUnits ) );

  mIntersectionSnapping = QgsProject::instance()->readNumEntry( "Digitizing", "/IntersectionSnapping", 0 );

  //read snapping settings from project
  bool snappingDefinedInProject, ok;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", QStringList(), &snappingDefinedInProject );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", QStringList(), &ok );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", QStringList(), &ok );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", QStringList(), &ok );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", QStringList(), &ok );

  // lists must have the same size, otherwise something is wrong
  if ( layerIdList.size() != enabledList.size() ||
       layerIdList.size() != toleranceList.size() ||
       layerIdList.size() != toleranceUnitList.size() ||
       layerIdList.size() != snapToList.size() )
    return;

  if ( !snappingDefinedInProject )
    return; // nothing defined in project - use current layer

  // Use snapping information from the project
  if ( snapMode == "current_layer" )
    mMode = ActiveLayer;
  else if ( snapMode == "all_layers" )
    mMode = AllLayers;
  else   // either "advanced" or empty (for background compatibility)
    mMode = AdvancedConfiguration;

  // load layers, tolerances, snap type
  QStringList::const_iterator layerIt( layerIdList.constBegin() );
  QStringList::const_iterator tolIt( toleranceList.constBegin() );
  QStringList::const_iterator tolUnitIt( toleranceUnitList.constBegin() );
  QStringList::const_iterator snapIt( snapToList.constBegin() );
  QStringList::const_iterator enabledIt( enabledList.constBegin() );
  for ( ; layerIt != layerIdList.constEnd(); ++layerIt, ++tolIt, ++tolUnitIt, ++snapIt, ++enabledIt )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( *layerIt ) );
    if ( !vlayer || !vlayer->hasGeometryType() )
      continue;

    SnappingType t( *snapIt == "to_vertex" ? Vertex :
                    ( *snapIt == "to_segment" ? Segment :
                      VertexAndSegment
                    )
                  );

    mIndividualLayerSettings.insert( vlayer, IndividualLayerSettings( *enabledIt == "enabled", t, tolIt->toDouble(), static_cast<QgsTolerance::UnitType>( tolUnitIt->toInt() ) ) );
  }
}
