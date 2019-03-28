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

#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"


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

bool QgsSnappingConfig::IndividualLayerSettings::operator !=( const QgsSnappingConfig::IndividualLayerSettings &other ) const
{
  return mValid != other.mValid
         || mEnabled != other.mEnabled
         || mType != other.mType
         || mTolerance != other.mTolerance
         || mUnits != other.mUnits;
}

bool QgsSnappingConfig::IndividualLayerSettings::operator ==( const QgsSnappingConfig::IndividualLayerSettings &other ) const
{
  return mValid == other.mValid
         && mEnabled == other.mEnabled
         && mType == other.mType
         && mTolerance == other.mTolerance
         && mUnits == other.mUnits;
}


QgsSnappingConfig::QgsSnappingConfig( QgsProject *project )
  : mProject( project )
{
  if ( project )
    reset();
}

bool QgsSnappingConfig::operator==( const QgsSnappingConfig &other ) const
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
  bool enabled = QgsSettings().value( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ), false ).toBool();
  SnappingMode mode = QgsSettings().enumValue( QStringLiteral( "/qgis/digitizing/default_snap_mode" ),  AllLayers );
  if ( mode == 0 )
  {
    // backward compatibility with QGIS 2.x
    // could be removed in 3.4+
    mode = AllLayers;
  }
  SnappingType type = QgsSettings().enumValue( QStringLiteral( "/qgis/digitizing/default_snap_type" ),  Vertex );
  double tolerance = QgsSettings().value( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance" ), Qgis::DEFAULT_SNAP_TOLERANCE ).toDouble();
  QgsTolerance::UnitType units = QgsSettings().enumValue( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance_unit" ),  Qgis::DEFAULT_SNAP_UNITS );

  // assign main (standard) config
  mEnabled = enabled;
  mMode = mode;
  mType = type;
  mTolerance = tolerance;
  // do not allow unit to be "layer" if not in advanced configuration
  if ( mUnits == QgsTolerance::LayerUnits && mMode != AdvancedConfiguration )
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
  Q_FOREACH ( QgsMapLayer *ml, mProject->mapLayers() )
  {
    QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer *>( ml );
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

QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> QgsSnappingConfig::individualLayerSettings() const
{
  return mIndividualLayerSettings;
}

QgsSnappingConfig::IndividualLayerSettings QgsSnappingConfig::individualLayerSettings( QgsVectorLayer *vl ) const
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

void QgsSnappingConfig::setIndividualLayerSettings( QgsVectorLayer *vl, const IndividualLayerSettings &individualLayerSettings )
{
  if ( !vl || !vl->isSpatial() || mIndividualLayerSettings.value( vl ) == individualLayerSettings )
  {
    return;
  }
  mIndividualLayerSettings.insert( vl, individualLayerSettings );
}

bool QgsSnappingConfig::operator!=( const QgsSnappingConfig &other ) const
{
  return mEnabled != other.mEnabled
         || mMode != other.mMode
         || mType != other.mType
         || mTolerance != other.mTolerance
         || mUnits != other.mUnits
         || mIndividualLayerSettings != other.mIndividualLayerSettings;
}

void QgsSnappingConfig::readProject( const QDomDocument &doc )
{
  QDomElement snapSettingsElem = doc.firstChildElement( QStringLiteral( "qgis" ) ).firstChildElement( QStringLiteral( "snapping-settings" ) );
  if ( snapSettingsElem.isNull() )
  {
    readLegacySettings();
    return;
  }

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "enabled" ) ) )
    mEnabled = snapSettingsElem.attribute( QStringLiteral( "enabled" ) ) == QLatin1String( "1" );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "mode" ) ) )
    mMode = ( SnappingMode )snapSettingsElem.attribute( QStringLiteral( "mode" ) ).toInt();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "type" ) ) )
    mType = ( SnappingType )snapSettingsElem.attribute( QStringLiteral( "type" ) ).toInt();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "tolerance" ) ) )
    mTolerance = snapSettingsElem.attribute( QStringLiteral( "tolerance" ) ).toDouble();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "unit" ) ) )
    mUnits = ( QgsTolerance::UnitType )snapSettingsElem.attribute( QStringLiteral( "unit" ) ).toInt();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "intersection-snapping" ) ) )
    mIntersectionSnapping = snapSettingsElem.attribute( QStringLiteral( "intersection-snapping" ) ) == QLatin1String( "1" );

  // do not clear the settings as they must be automatically synchronized with current layers
  QDomNodeList nodes = snapSettingsElem.elementsByTagName( QStringLiteral( "individual-layer-settings" ) );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    QDomNodeList settingNodes = node.childNodes();
    int layerCount = settingNodes.count();
    for ( int i = 0; i < layerCount; ++i )
    {
      QDomElement settingElement = settingNodes.at( i ).toElement();
      if ( settingElement.tagName() != QLatin1String( "layer-setting" ) )
      {
        QgsLogger::warning( QApplication::translate( "QgsProjectSnappingSettings", "Cannot read individual settings. Unexpected tag '%1'" ).arg( settingElement.tagName() ) );
        continue;
      }

      QString layerId = settingElement.attribute( QStringLiteral( "id" ) );
      bool enabled = settingElement.attribute( QStringLiteral( "enabled" ) ) == QLatin1String( "1" );
      SnappingType type = ( SnappingType )settingElement.attribute( QStringLiteral( "type" ) ).toInt();
      double tolerance = settingElement.attribute( QStringLiteral( "tolerance" ) ).toDouble();
      QgsTolerance::UnitType units = ( QgsTolerance::UnitType )settingElement.attribute( QStringLiteral( "units" ) ).toInt();

      QgsMapLayer *ml = mProject->mapLayer( layerId );
      if ( !ml || ml->type() != QgsMapLayerType::VectorLayer )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );

      IndividualLayerSettings setting = IndividualLayerSettings( enabled, type, tolerance, units );
      mIndividualLayerSettings.insert( vl, setting );
    }
  }
}

void QgsSnappingConfig::writeProject( QDomDocument &doc )
{
  QDomElement snapSettingsElem = doc.createElement( QStringLiteral( "snapping-settings" ) );
  snapSettingsElem.setAttribute( QStringLiteral( "enabled" ), QString::number( mEnabled ) );
  snapSettingsElem.setAttribute( QStringLiteral( "mode" ), static_cast<int>( mMode ) );
  snapSettingsElem.setAttribute( QStringLiteral( "type" ), static_cast<int>( mType ) );
  snapSettingsElem.setAttribute( QStringLiteral( "tolerance" ), mTolerance );
  snapSettingsElem.setAttribute( QStringLiteral( "unit" ), static_cast<int>( mUnits ) );
  snapSettingsElem.setAttribute( QStringLiteral( "intersection-snapping" ), QString::number( mIntersectionSnapping ) );

  QDomElement ilsElement = doc.createElement( QStringLiteral( "individual-layer-settings" ) );
  QHash<QgsVectorLayer *, IndividualLayerSettings>::const_iterator layerIt = mIndividualLayerSettings.constBegin();
  for ( ; layerIt != mIndividualLayerSettings.constEnd(); ++layerIt )
  {
    const IndividualLayerSettings &setting = layerIt.value();

    QDomElement layerElement = doc.createElement( QStringLiteral( "layer-setting" ) );
    layerElement.setAttribute( QStringLiteral( "id" ), layerIt.key()->id() );
    layerElement.setAttribute( QStringLiteral( "enabled" ), QString::number( setting.enabled() ) );
    layerElement.setAttribute( QStringLiteral( "type" ), static_cast<int>( setting.type() ) );
    layerElement.setAttribute( QStringLiteral( "tolerance" ), setting.tolerance() );
    layerElement.setAttribute( QStringLiteral( "units" ), static_cast<int>( setting.units() ) );
    ilsElement.appendChild( layerElement );
  }
  snapSettingsElem.appendChild( ilsElement );

  doc.firstChildElement( QStringLiteral( "qgis" ) ).appendChild( snapSettingsElem );
}

bool QgsSnappingConfig::addLayers( const QList<QgsMapLayer *> &layers )
{
  bool changed = false;
  bool enabled = QgsSettings().value( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ), true ).toBool();
  SnappingType type = QgsSettings().enumValue( QStringLiteral( "/qgis/digitizing/default_snap_type" ), Vertex );
  double tolerance = QgsSettings().value( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance" ), Qgis::DEFAULT_SNAP_TOLERANCE ).toDouble();
  QgsTolerance::UnitType units = QgsSettings().enumValue( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance_unit" ), Qgis::DEFAULT_SNAP_UNITS );

  Q_FOREACH ( QgsMapLayer *ml, layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( vl && vl->isSpatial() )
    {
      mIndividualLayerSettings.insert( vl, IndividualLayerSettings( enabled, type, tolerance, units ) );
      changed = true;
    }
  }
  return changed;
}

bool QgsSnappingConfig::removeLayers( const QList<QgsMapLayer *> &layers )
{
  bool changed = false;
  Q_FOREACH ( QgsMapLayer *ml, layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
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
  //
  mMode = ActiveLayer;

  QString snapMode = mProject->readEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/SnappingMode" ) );

  mTolerance = mProject->readDoubleEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapTolerance" ), 0 );
  mUnits = static_cast< QgsTolerance::UnitType >( mProject->readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapToleranceUnit" ), QgsTolerance::ProjectUnits ) );

  mIntersectionSnapping = mProject->readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/IntersectionSnapping" ), 0 );

  //read snapping settings from project
  QStringList layerIdList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingList" ), QStringList() );
  QStringList enabledList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingEnabledList" ), QStringList() );
  QStringList toleranceList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingToleranceList" ), QStringList() );
  QStringList toleranceUnitList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingToleranceUnitList" ), QStringList() );
  QStringList snapToList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnapToList" ), QStringList() );

  // lists must have the same size, otherwise something is wrong
  if ( layerIdList.size() != enabledList.size() ||
       layerIdList.size() != toleranceList.size() ||
       layerIdList.size() != toleranceUnitList.size() ||
       layerIdList.size() != snapToList.size() )
    return;

  // Use snapping information from the project
  if ( snapMode == QLatin1String( "current_layer" ) )
    mMode = ActiveLayer;
  else if ( snapMode == QLatin1String( "all_layers" ) )
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
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mProject->mapLayer( *layerIt ) );
    if ( !vlayer || !vlayer->isSpatial() )
      continue;

    SnappingType t( *snapIt == QLatin1String( "to_vertex" ) ? Vertex :
                    ( *snapIt == QLatin1String( "to_segment" ) ? Segment :
                      VertexAndSegment
                    )
                  );

    mIndividualLayerSettings.insert( vlayer, IndividualLayerSettings( *enabledIt == QLatin1String( "enabled" ), t, tolIt->toDouble(), static_cast<QgsTolerance::UnitType>( tolUnitIt->toInt() ) ) );
  }

  QString snapType = mProject->readEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapType" ), QStringLiteral( "off" ) );
  mEnabled = true;
  if ( snapType == QLatin1String( "to segment" ) )
    mType = Segment;
  else if ( snapType == QLatin1String( "to vertex and segment" ) )
    mType = VertexAndSegment;
  else if ( snapType == QLatin1String( "to vertex" ) )
    mType = Vertex;
  else if ( mMode != AdvancedConfiguration ) // Type is off but mode is advanced
  {
    mEnabled = false;
  }
}

QgsProject *QgsSnappingConfig::project() const
{
  return mProject;
}

void QgsSnappingConfig::setProject( QgsProject *project )
{
  if ( mProject != project )
    mProject = project;

  reset();
}
