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
#include <QRegularExpression>

#include "qgssettingsregistrycore.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"


QgsSnappingConfig::IndividualLayerSettings::IndividualLayerSettings( bool enabled, Qgis::SnappingTypes type, double tolerance, QgsTolerance::UnitType units, double minScale, double maxScale )
  : mValid( true )
  , mEnabled( enabled )
  , mType( type )
  , mTolerance( tolerance )
  , mUnits( units )
  , mMinimumScale( minScale )
  , mMaximumScale( maxScale )
{}

QgsSnappingConfig::IndividualLayerSettings::IndividualLayerSettings( bool enabled, SnappingType type, double tolerance, QgsTolerance::UnitType units )
  : mValid( true )
  , mEnabled( enabled )
  , mTolerance( tolerance )
  , mUnits( units )
{
  Q_NOWARN_DEPRECATED_PUSH
  setType( type );
  Q_NOWARN_DEPRECATED_POP
}

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

Qgis::SnappingTypes QgsSnappingConfig::IndividualLayerSettings::typeFlag() const
{
  return mType;
}

QgsSnappingConfig::SnappingType QgsSnappingConfig::IndividualLayerSettings::type() const
{

  if ( ( mType & QgsSnappingConfig::SnappingType::Segment ) && ( mType & QgsSnappingConfig::SnappingType::Vertex ) )
    return QgsSnappingConfig::SnappingType::VertexAndSegment;
  else if ( mType & QgsSnappingConfig::SnappingType::Segment )
    return QgsSnappingConfig::SnappingType::Segment;
  else
    return QgsSnappingConfig::SnappingType::Vertex;

}

void QgsSnappingConfig::IndividualLayerSettings::setType( QgsSnappingConfig::SnappingType type )
{
  switch ( type )
  {
    case 1:
      mType = Qgis::SnappingType::Vertex;
      break;
    case 2:
      mType = Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment;
      break;
    case 3:
      mType = Qgis::SnappingType::Segment;
      break;
    default:
      mType = Qgis::SnappingType::NoSnap;
      break;
  }
}
void QgsSnappingConfig::IndividualLayerSettings::setTypeFlag( Qgis::SnappingTypes type )
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

double QgsSnappingConfig::IndividualLayerSettings::minimumScale() const
{
  return mMinimumScale;
}

void QgsSnappingConfig::IndividualLayerSettings::setMinimumScale( double minScale )
{
  mMinimumScale = minScale;
}

double QgsSnappingConfig::IndividualLayerSettings::maximumScale() const
{
  return mMaximumScale;
}

void QgsSnappingConfig::IndividualLayerSettings::setMaximumScale( double maxScale )
{
  mMaximumScale = maxScale;
}

bool QgsSnappingConfig::IndividualLayerSettings::operator !=( const QgsSnappingConfig::IndividualLayerSettings &other ) const
{
  return mValid != other.mValid
         || mEnabled != other.mEnabled
         || mType != other.mType
         || mTolerance != other.mTolerance
         || mUnits != other.mUnits
         || mMinimumScale != other.mMinimumScale
         || mMaximumScale != other.mMaximumScale;
}

bool QgsSnappingConfig::IndividualLayerSettings::operator ==( const QgsSnappingConfig::IndividualLayerSettings &other ) const
{
  return mValid == other.mValid
         && mEnabled == other.mEnabled
         && mType == other.mType
         && mTolerance == other.mTolerance
         && mUnits == other.mUnits
         && mMinimumScale == other.mMinimumScale
         && mMaximumScale == other.mMaximumScale;
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
         && mSelfSnapping == other.mSelfSnapping
         && mIndividualLayerSettings == other.mIndividualLayerSettings
         && mScaleDependencyMode == other.mScaleDependencyMode
         && mMinimumScale == other.mMinimumScale
         && mMaximumScale == other.mMaximumScale;
}

void QgsSnappingConfig::reset()
{
  // get defaults values. They are both used for standard and advanced configuration (per layer)
  const bool enabled = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled.value();
  const Qgis::SnappingMode mode = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapMode.value();
  const Qgis::SnappingType type = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType.value();
  const double tolerance = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance.value();
  const QgsTolerance::UnitType units = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit.value();

  // assign main (standard) config
  mEnabled = enabled;
  mMode = mode;
  mType = type;
  mTolerance = tolerance;
  mScaleDependencyMode = Disabled;
  mMinimumScale = 0.0;
  mMaximumScale = 0.0;
  // do not allow unit to be "layer" if not in advanced configuration
  if ( mUnits == QgsTolerance::LayerUnits && mMode != Qgis::SnappingMode::AdvancedConfiguration )
  {
    mUnits = QgsTolerance::ProjectUnits;
  }
  else
  {
    mUnits = units;
  }
  mIntersectionSnapping = false;
  mSelfSnapping = false;

  // set advanced config
  if ( mProject )
  {
    mIndividualLayerSettings = QHash<QgsVectorLayer *, IndividualLayerSettings>();
    const auto constMapLayers = mProject->mapLayers();
    for ( QgsMapLayer *ml : constMapLayers )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
      if ( vl )
      {
        mIndividualLayerSettings.insert( vl, IndividualLayerSettings( enabled, type, tolerance, units, 0.0, 0.0 ) );
      }
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

Qgis::SnappingMode QgsSnappingConfig::mode() const
{
  return mMode;
}

void QgsSnappingConfig::setMode( Qgis::SnappingMode mode )
{
  if ( mMode == mode )
  {
    return;
  }
  mMode = mode;
}

Qgis::SnappingTypes QgsSnappingConfig::typeFlag() const
{
  return mType;
}

QgsSnappingConfig::SnappingType QgsSnappingConfig::type() const
{
  if ( ( mType & QgsSnappingConfig::SnappingType::Segment ) && ( mType & QgsSnappingConfig::SnappingType::Vertex ) )
    return QgsSnappingConfig::SnappingType::VertexAndSegment;
  else if ( mType & QgsSnappingConfig::SnappingType::Segment )
    return QgsSnappingConfig::SnappingType::Segment;
  else
    return QgsSnappingConfig::SnappingType::Vertex;
}

QString QgsSnappingConfig::snappingTypeToString( Qgis::SnappingType type )
{
  switch ( type )
  {
    case Qgis::SnappingType::NoSnap:
      return QObject::tr( "No Snapping" );
    case Qgis::SnappingType::Vertex:
      return QObject::tr( "Vertex" );
    case Qgis::SnappingType::Segment:
      return QObject::tr( "Segment" );
    case Qgis::SnappingType::Area:
      return QObject::tr( "Area" );
    case Qgis::SnappingType::Centroid:
      return QObject::tr( "Centroid" );
    case Qgis::SnappingType::MiddleOfSegment:
      return QObject::tr( "Middle of Segments" );
    case Qgis::SnappingType::LineEndpoint:
      return QObject::tr( "Line Endpoints" );
  }
  return QString();
}

QIcon QgsSnappingConfig::snappingTypeToIcon( Qgis::SnappingType type )
{
  switch ( type )
  {
    case Qgis::SnappingType::NoSnap:
      return QIcon();
    case Qgis::SnappingType::Vertex:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingVertex.svg" ) );
    case Qgis::SnappingType::Segment:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingSegment.svg" ) );
    case Qgis::SnappingType::Area:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingArea.svg" ) );
    case Qgis::SnappingType::Centroid:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingCentroid.svg" ) );
    case Qgis::SnappingType::MiddleOfSegment:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingMiddle.svg" ) );
    case Qgis::SnappingType::LineEndpoint:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnappingEndpoint.svg" ) );
  }
  return QIcon();
}

void QgsSnappingConfig::setType( QgsSnappingConfig::SnappingType type )
{
  switch ( type )
  {
    case SnappingType::Vertex:
      mType = Qgis::SnappingType::Vertex;
      break;
    case SnappingType::VertexAndSegment:
      mType = static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment );
      break;
    case SnappingType::Segment:
      mType = Qgis::SnappingType::Segment;
      break;
    default:
      mType = Qgis::SnappingType::NoSnap;
      break;
  }
}
void QgsSnappingConfig::setTypeFlag( Qgis::SnappingTypes type )
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

bool QgsSnappingConfig::selfSnapping() const
{
  return mSelfSnapping;
}

void QgsSnappingConfig::setSelfSnapping( bool enabled )
{
  mSelfSnapping = enabled;
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

void QgsSnappingConfig::clearIndividualLayerSettings()
{
  mIndividualLayerSettings.clear();
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
         || mIndividualLayerSettings != other.mIndividualLayerSettings
         || mScaleDependencyMode != other.mScaleDependencyMode
         || mMinimumScale != other.mMinimumScale
         || mMaximumScale != other.mMaximumScale;
}

void QgsSnappingConfig::readProject( const QDomDocument &doc )
{
  const QDomElement snapSettingsElem = doc.firstChildElement( QStringLiteral( "qgis" ) ).firstChildElement( QStringLiteral( "snapping-settings" ) );
  if ( snapSettingsElem.isNull() )
  {
    readLegacySettings();
    return;
  }

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "enabled" ) ) )
    mEnabled = snapSettingsElem.attribute( QStringLiteral( "enabled" ) ) == QLatin1String( "1" );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "mode" ) ) )
    mMode = static_cast< Qgis::SnappingMode >( snapSettingsElem.attribute( QStringLiteral( "mode" ) ).toInt() );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "type" ) ) )
  {
    const int type = snapSettingsElem.attribute( QStringLiteral( "type" ) ).toInt();
    const QDomElement versionElem = doc.firstChildElement( QStringLiteral( "qgis" ) );
    QString version;
    bool before3_14 = false;
    if ( versionElem.hasAttribute( QStringLiteral( "version" ) ) )
    {
      version = versionElem.attribute( QStringLiteral( "version" ) );
      const QRegularExpression re( QStringLiteral( "([\\d]+)\\.([\\d]+)" ) );
      const QRegularExpressionMatch match = re.match( version );
      if ( match.hasMatch() )
      {
        if ( ( match.captured( 1 ).toInt() <= 3 ) && ( match.captured( 2 ).toInt() <= 12 ) )
          before3_14 = true;
      }
    }
    if ( before3_14 )
    {
      // BEFORE 3.12:
      // 1 = vertex
      // 2 = vertexandsegment
      // 3 = segment
      switch ( type )
      {
        case 1:
          mType = Qgis::SnappingType::Vertex;
          break;
        case 2:
          mType = static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment );
          break;
        case 3:
          mType = Qgis::SnappingType::Segment;
          break;
        default:
          mType = Qgis::SnappingType::NoSnap;
          break;
      }
    }
    else
      mType = static_cast<Qgis::SnappingTypes>( type );
  }

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "tolerance" ) ) )
    mTolerance = snapSettingsElem.attribute( QStringLiteral( "tolerance" ) ).toDouble();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "scaleDependencyMode" ) ) )
    mScaleDependencyMode = static_cast<QgsSnappingConfig::ScaleDependencyMode>( snapSettingsElem.attribute( QStringLiteral( "scaleDependencyMode" ) ).toInt() );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "minScale" ) ) )
    mMinimumScale = snapSettingsElem.attribute( QStringLiteral( "minScale" ) ).toDouble();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "maxScale" ) ) )
    mMaximumScale = snapSettingsElem.attribute( QStringLiteral( "maxScale" ) ).toDouble();

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "unit" ) ) )
    mUnits = static_cast< QgsTolerance::UnitType >( snapSettingsElem.attribute( QStringLiteral( "unit" ) ).toInt() );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "intersection-snapping" ) ) )
    mIntersectionSnapping = snapSettingsElem.attribute( QStringLiteral( "intersection-snapping" ) ) == QLatin1String( "1" );

  if ( snapSettingsElem.hasAttribute( QStringLiteral( "self-snapping" ) ) )
    mSelfSnapping = snapSettingsElem.attribute( QStringLiteral( "self-snapping" ) ) == QLatin1String( "1" );

  // do not clear the settings as they must be automatically synchronized with current layers
  const QDomNodeList nodes = snapSettingsElem.elementsByTagName( QStringLiteral( "individual-layer-settings" ) );
  if ( nodes.count() )
  {
    const QDomNode node = nodes.item( 0 );
    const QDomNodeList settingNodes = node.childNodes();
    const int layerCount = settingNodes.count();
    for ( int i = 0; i < layerCount; ++i )
    {
      const QDomElement settingElement = settingNodes.at( i ).toElement();
      if ( settingElement.tagName() != QLatin1String( "layer-setting" ) )
      {
        QgsLogger::warning( QApplication::translate( "QgsProjectSnappingSettings", "Cannot read individual settings. Unexpected tag '%1'" ).arg( settingElement.tagName() ) );
        continue;
      }

      const QString layerId = settingElement.attribute( QStringLiteral( "id" ) );
      const bool enabled = settingElement.attribute( QStringLiteral( "enabled" ) ) == QLatin1String( "1" );
      const Qgis::SnappingTypes type = static_cast<Qgis::SnappingTypes>( settingElement.attribute( QStringLiteral( "type" ) ).toInt() );
      const double tolerance = settingElement.attribute( QStringLiteral( "tolerance" ) ).toDouble();
      const QgsTolerance::UnitType units = static_cast< QgsTolerance::UnitType >( settingElement.attribute( QStringLiteral( "units" ) ).toInt() );
      const double minScale = settingElement.attribute( QStringLiteral( "minScale" ) ).toDouble();
      const double maxScale = settingElement.attribute( QStringLiteral( "maxScale" ) ).toDouble();

      QgsMapLayer *ml = mProject->mapLayer( layerId );
      if ( !ml || ml->type() != QgsMapLayerType::VectorLayer )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );

      const IndividualLayerSettings setting = IndividualLayerSettings( enabled, type, tolerance, units, minScale, maxScale );
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
  snapSettingsElem.setAttribute( QStringLiteral( "self-snapping" ), QString::number( mSelfSnapping ) );
  snapSettingsElem.setAttribute( QStringLiteral( "scaleDependencyMode" ), QString::number( mScaleDependencyMode ) );
  snapSettingsElem.setAttribute( QStringLiteral( "minScale" ), mMinimumScale );
  snapSettingsElem.setAttribute( QStringLiteral( "maxScale" ), mMaximumScale );

  QDomElement ilsElement = doc.createElement( QStringLiteral( "individual-layer-settings" ) );
  QHash<QgsVectorLayer *, IndividualLayerSettings>::const_iterator layerIt = mIndividualLayerSettings.constBegin();
  for ( ; layerIt != mIndividualLayerSettings.constEnd(); ++layerIt )
  {
    const IndividualLayerSettings &setting = layerIt.value();

    QDomElement layerElement = doc.createElement( QStringLiteral( "layer-setting" ) );
    layerElement.setAttribute( QStringLiteral( "id" ), layerIt.key()->id() );
    layerElement.setAttribute( QStringLiteral( "enabled" ), QString::number( setting.enabled() ) );
    layerElement.setAttribute( QStringLiteral( "type" ), static_cast<int>( setting.typeFlag() ) );
    layerElement.setAttribute( QStringLiteral( "tolerance" ), setting.tolerance() );
    layerElement.setAttribute( QStringLiteral( "units" ), static_cast<int>( setting.units() ) );
    layerElement.setAttribute( QStringLiteral( "minScale" ), setting.minimumScale() );
    layerElement.setAttribute( QStringLiteral( "maxScale" ), setting.maximumScale() );
    ilsElement.appendChild( layerElement );
  }
  snapSettingsElem.appendChild( ilsElement );

  doc.firstChildElement( QStringLiteral( "qgis" ) ).appendChild( snapSettingsElem );
}

bool QgsSnappingConfig::addLayers( const QList<QgsMapLayer *> &layers )
{
  bool changed = false;
  const bool enabled = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled.valueWithDefaultOverride( true );
  const Qgis::SnappingTypes type = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType.value();
  const double tolerance = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance.value();
  const QgsTolerance::UnitType units = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit.value();

  const auto constLayers = layers;
  for ( QgsMapLayer *ml : constLayers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( vl && vl->isSpatial() )
    {
      mIndividualLayerSettings.insert( vl, IndividualLayerSettings( enabled, type, tolerance, units, 0.0, 0.0 ) );
      changed = true;
    }
  }
  return changed;
}

bool QgsSnappingConfig::removeLayers( const QList<QgsMapLayer *> &layers )
{
  bool changed = false;
  const auto constLayers = layers;
  for ( QgsMapLayer *ml : constLayers )
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
  mMode = Qgis::SnappingMode::ActiveLayer;

  const QString snapMode = mProject->readEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/SnappingMode" ) );

  mTolerance = mProject->readDoubleEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapTolerance" ), 0 );
  mUnits = static_cast< QgsTolerance::UnitType >( mProject->readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapToleranceUnit" ), QgsTolerance::ProjectUnits ) );

  mIntersectionSnapping = mProject->readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/IntersectionSnapping" ), 0 );

  //read snapping settings from project
  const QStringList layerIdList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingList" ), QStringList() );
  const QStringList enabledList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingEnabledList" ), QStringList() );
  const QStringList toleranceList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingToleranceList" ), QStringList() );
  const QStringList toleranceUnitList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnappingToleranceUnitList" ), QStringList() );
  const QStringList snapToList = mProject->readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/LayerSnapToList" ), QStringList() );

  // lists must have the same size, otherwise something is wrong
  if ( layerIdList.size() != enabledList.size() ||
       layerIdList.size() != toleranceList.size() ||
       layerIdList.size() != toleranceUnitList.size() ||
       layerIdList.size() != snapToList.size() )
    return;

  // Use snapping information from the project
  if ( snapMode == QLatin1String( "current_layer" ) )
    mMode = Qgis::SnappingMode::ActiveLayer;
  else if ( snapMode == QLatin1String( "all_layers" ) )
    mMode = Qgis::SnappingMode::AllLayers;
  else   // either "advanced" or empty (for background compatibility)
    mMode = Qgis::SnappingMode::AdvancedConfiguration;

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

    const Qgis::SnappingTypes t( *snapIt == QLatin1String( "to_vertex" ) ? Qgis::SnappingType::Vertex :
                                 ( *snapIt == QLatin1String( "to_segment" ) ? Qgis::SnappingType::Segment :
                                   static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment )
                                 )
                               );

    mIndividualLayerSettings.insert( vlayer, IndividualLayerSettings( *enabledIt == QLatin1String( "enabled" ), t, tolIt->toDouble(), static_cast<QgsTolerance::UnitType>( tolUnitIt->toInt() ), 0.0, 0.0 ) );
  }

  const QString snapType = mProject->readEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/DefaultSnapType" ), QStringLiteral( "off" ) );
  mEnabled = true;
  if ( snapType == QLatin1String( "to segment" ) )
    mType = Qgis::SnappingType::Segment;
  else if ( snapType == QLatin1String( "to vertex and segment" ) )
    mType = static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment );
  else if ( snapType == QLatin1String( "to vertex" ) )
    mType = Qgis::SnappingType::Vertex;
  else if ( mMode != Qgis::SnappingMode::AdvancedConfiguration ) // Type is off but mode is advanced
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

double QgsSnappingConfig::minimumScale() const
{
  return mMinimumScale;
}

void QgsSnappingConfig::setMinimumScale( double minScale )
{
  mMinimumScale = minScale;
}

double QgsSnappingConfig::maximumScale() const
{
  return mMaximumScale;
}

void QgsSnappingConfig::setMaximumScale( double maxScale )
{
  mMaximumScale = maxScale;
}

void QgsSnappingConfig::setScaleDependencyMode( QgsSnappingConfig::ScaleDependencyMode mode )
{
  mScaleDependencyMode = mode;
}

QgsSnappingConfig::ScaleDependencyMode QgsSnappingConfig::scaleDependencyMode() const
{
  return mScaleDependencyMode;
}





