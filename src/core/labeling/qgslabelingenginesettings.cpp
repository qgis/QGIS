/***************************************************************************
  qgslabelingenginesettings.cpp
  --------------------------------------
  Date                 : April 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingenginesettings.h"

#include "qgsapplication.h"
#include "qgscolorutils.h"
#include "qgslabelingenginerule.h"
#include "qgslabelingengineruleregistry.h"
#include "qgsproject.h"

QgsLabelingEngineSettings::QgsLabelingEngineSettings()
{
}

QgsLabelingEngineSettings::~QgsLabelingEngineSettings() = default;

QgsLabelingEngineSettings::QgsLabelingEngineSettings( const QgsLabelingEngineSettings &other )
//****** IMPORTANT! editing this? make sure you update the move constructor too! *****
  : mFlags( other.mFlags )
  , mSearchMethod( other.mSearchMethod )
  , mMaxLineCandidatesPerCm( other.mMaxLineCandidatesPerCm )
  , mMaxPolygonCandidatesPerCmSquared( other.mMaxPolygonCandidatesPerCmSquared )
  , mUnplacedLabelColor( other.mUnplacedLabelColor )
  , mPlacementVersion( other.mPlacementVersion )
  , mDefaultTextRenderFormat( other.mDefaultTextRenderFormat )
    //****** IMPORTANT! editing this? make sure you update the move constructor too! *****
{
  mEngineRules.reserve( other.mEngineRules.size() );
  for ( const auto &rule : other.mEngineRules )
  {
    mEngineRules.emplace_back( rule->clone() );
  }
}

QgsLabelingEngineSettings::QgsLabelingEngineSettings( QgsLabelingEngineSettings &&other )
  : mFlags( other.mFlags )
  , mSearchMethod( other.mSearchMethod )
  , mMaxLineCandidatesPerCm( other.mMaxLineCandidatesPerCm )
  , mMaxPolygonCandidatesPerCmSquared( other.mMaxPolygonCandidatesPerCmSquared )
  , mUnplacedLabelColor( std::move( other.mUnplacedLabelColor ) )
  , mPlacementVersion( other.mPlacementVersion )
  , mDefaultTextRenderFormat( other.mDefaultTextRenderFormat )
  , mEngineRules( std::move( other.mEngineRules ) )
{
}

QgsLabelingEngineSettings &QgsLabelingEngineSettings::operator=( const QgsLabelingEngineSettings &other )
{
  if ( &other == this )
    return *this;

  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  mFlags = other.mFlags;
  mSearchMethod = other.mSearchMethod;
  mMaxLineCandidatesPerCm = other.mMaxLineCandidatesPerCm;
  mMaxPolygonCandidatesPerCmSquared = other.mMaxPolygonCandidatesPerCmSquared;
  mUnplacedLabelColor = other.mUnplacedLabelColor;
  mPlacementVersion = other.mPlacementVersion;
  mDefaultTextRenderFormat = other.mDefaultTextRenderFormat;
  mEngineRules.clear();
  mEngineRules.reserve( other.mEngineRules.size() );
  for ( const auto &rule : other.mEngineRules )
  {
    mEngineRules.emplace_back( rule->clone() );
  }
  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  return *this;
}

QgsLabelingEngineSettings &QgsLabelingEngineSettings::operator=( QgsLabelingEngineSettings &&other )
{
  if ( &other == this )
    return *this;

  mFlags = other.mFlags;
  mSearchMethod = other.mSearchMethod;
  mMaxLineCandidatesPerCm = other.mMaxLineCandidatesPerCm;
  mMaxPolygonCandidatesPerCmSquared = other.mMaxPolygonCandidatesPerCmSquared;
  mUnplacedLabelColor = std::move( other.mUnplacedLabelColor );
  mPlacementVersion = other.mPlacementVersion;
  mDefaultTextRenderFormat = other.mDefaultTextRenderFormat;
  mEngineRules = std::move( other.mEngineRules );
  return *this;
}

void QgsLabelingEngineSettings::clear()
{
  *this = QgsLabelingEngineSettings();
}

void QgsLabelingEngineSettings::readSettingsFromProject( QgsProject *prj )
{
  bool saved = false;
  mSearchMethod = static_cast< Search >( prj->readNumEntry( u"PAL"_s, u"/SearchMethod"_s, static_cast< int >( Chain ), &saved ) );
  mMaxLineCandidatesPerCm = prj->readDoubleEntry( u"PAL"_s, u"/CandidatesLinePerCM"_s, 5, &saved );
  mMaxPolygonCandidatesPerCmSquared = prj->readDoubleEntry( u"PAL"_s, u"/CandidatesPolygonPerCM"_s, 2.5, &saved );

  mFlags = Qgis::LabelingFlags();
  if ( prj->readBoolEntry( u"PAL"_s, u"/ShowingCandidates"_s, false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawCandidates;
  if ( prj->readBoolEntry( u"PAL"_s, u"/DrawRectOnly"_s, false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawLabelRectOnly;
  if ( prj->readBoolEntry( u"PAL"_s, u"/ShowingAllLabels"_s, false, &saved ) ) mFlags |= Qgis::LabelingFlag::UseAllLabels;
  if ( prj->readBoolEntry( u"PAL"_s, u"/ShowingPartialsLabels"_s, true, &saved ) ) mFlags |= Qgis::LabelingFlag::UsePartialCandidates;
  if ( prj->readBoolEntry( u"PAL"_s, u"/DrawUnplaced"_s, false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawUnplacedLabels;
  if ( prj->readBoolEntry( u"PAL"_s, u"/DrawLabelMetrics"_s, false, &saved ) ) mFlags |= Qgis::LabelingFlag::DrawLabelMetrics;

  mDefaultTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;
  // if users have disabled the older PAL "DrawOutlineLabels" setting, respect that
  if ( !prj->readBoolEntry( u"PAL"_s, u"/DrawOutlineLabels"_s, true ) )
    mDefaultTextRenderFormat = Qgis::TextRenderFormat::AlwaysText;
  // otherwise, prefer the new setting
  const int projectTextFormat = prj->readNumEntry( u"PAL"_s, u"/TextFormat"_s, -1 );
  if ( projectTextFormat >= 0 )
    mDefaultTextRenderFormat = static_cast< Qgis::TextRenderFormat >( projectTextFormat );

  mUnplacedLabelColor = QgsColorUtils::colorFromString( prj->readEntry( u"PAL"_s, u"/UnplacedColor"_s, u"#ff0000"_s ) );

  mPlacementVersion = static_cast< Qgis::LabelPlacementEngineVersion >( prj->readNumEntry( u"PAL"_s, u"/PlacementEngineVersion"_s, static_cast< int >( Qgis::LabelPlacementEngineVersion::Version1 ) ) );
}

void QgsLabelingEngineSettings::writeSettingsToProject( QgsProject *project )
{
  project->writeEntry( u"PAL"_s, u"/SearchMethod"_s, static_cast< int >( mSearchMethod ) );
  project->writeEntry( u"PAL"_s, u"/CandidatesLinePerCM"_s, mMaxLineCandidatesPerCm );
  project->writeEntry( u"PAL"_s, u"/CandidatesPolygonPerCM"_s, mMaxPolygonCandidatesPerCmSquared );

  project->writeEntry( u"PAL"_s, u"/ShowingCandidates"_s, mFlags.testFlag( Qgis::LabelingFlag::DrawCandidates ) );
  project->writeEntry( u"PAL"_s, u"/DrawRectOnly"_s, mFlags.testFlag( Qgis::LabelingFlag::DrawLabelRectOnly ) );
  project->writeEntry( u"PAL"_s, u"/DrawUnplaced"_s, mFlags.testFlag( Qgis::LabelingFlag::DrawUnplacedLabels ) );
  project->writeEntry( u"PAL"_s, u"/ShowingAllLabels"_s, mFlags.testFlag( Qgis::LabelingFlag::UseAllLabels ) );
  project->writeEntry( u"PAL"_s, u"/ShowingPartialsLabels"_s, mFlags.testFlag( Qgis::LabelingFlag::UsePartialCandidates ) );
  project->writeEntry( u"PAL"_s, u"/DrawLabelMetrics"_s, mFlags.testFlag( Qgis::LabelingFlag::DrawLabelMetrics ) );

  project->writeEntry( u"PAL"_s, u"/TextFormat"_s, static_cast< int >( mDefaultTextRenderFormat ) );

  project->writeEntry( u"PAL"_s, u"/UnplacedColor"_s, QgsColorUtils::colorToString( mUnplacedLabelColor ) );

  project->writeEntry( u"PAL"_s, u"/PlacementEngineVersion"_s, static_cast< int >( mPlacementVersion ) );
}

void QgsLabelingEngineSettings::writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const
{
  if ( !mEngineRules.empty() )
  {
    QDomElement rulesElement = doc.createElement( u"rules"_s );
    for ( const auto &rule : mEngineRules )
    {
      QDomElement ruleElement = doc.createElement( u"rule"_s );
      ruleElement.setAttribute( u"id"_s, rule->id() );
      if ( !rule->name().isEmpty() )
        ruleElement.setAttribute( u"name"_s, rule->name() );
      if ( !rule->active() )
        ruleElement.setAttribute( u"active"_s, u"0"_s );
      rule->writeXml( doc, ruleElement, context );
      rulesElement.appendChild( ruleElement );
    }
    element.appendChild( rulesElement );
  }
}

void QgsLabelingEngineSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mEngineRules.clear();
  {
    const QDomElement rulesElement = element.firstChildElement( u"rules"_s );
    const QDomNodeList rules = rulesElement.childNodes();
    for ( int i = 0; i < rules.length(); i++ )
    {
      const QDomElement ruleElement = rules.at( i ).toElement();
      const QString id = ruleElement.attribute( u"id"_s );
      const QString name = ruleElement.attribute( u"name"_s );
      const bool active = ruleElement.attribute( u"active"_s, u"1"_s ).toInt();
      std::unique_ptr< QgsAbstractLabelingEngineRule > rule( QgsApplication::labelingEngineRuleRegistry()->create( id ) );
      if ( rule )
      {
        rule->setName( name );
        rule->setActive( active );
        rule->readXml( ruleElement, context );
        mEngineRules.emplace_back( std::move( rule ) );
      }
    }
  }
}

void QgsLabelingEngineSettings::resolveReferences( const QgsProject *project )
{
  for ( const auto &rule : mEngineRules )
  {
    rule->resolveReferences( project );
  }
}

QColor QgsLabelingEngineSettings::unplacedLabelColor() const
{
  return mUnplacedLabelColor;
}

void QgsLabelingEngineSettings::setUnplacedLabelColor( const QColor &unplacedLabelColor )
{
  mUnplacedLabelColor = unplacedLabelColor;
}

Qgis::LabelPlacementEngineVersion QgsLabelingEngineSettings::placementVersion() const
{
  return mPlacementVersion;
}

void QgsLabelingEngineSettings::setPlacementVersion( Qgis::LabelPlacementEngineVersion placementVersion )
{
  mPlacementVersion = placementVersion;
}

QList<QgsAbstractLabelingEngineRule *> QgsLabelingEngineSettings::rules()
{
  QList<QgsAbstractLabelingEngineRule *> res;
  for ( const auto &it : mEngineRules )
  {
    res << it.get();
  }
  return res;
}

QList<const QgsAbstractLabelingEngineRule *> QgsLabelingEngineSettings::rules() const
{
  QList<const QgsAbstractLabelingEngineRule *> res;
  for ( const auto &it : mEngineRules )
  {
    res << it.get();
  }
  return res;
}

void QgsLabelingEngineSettings::addRule( QgsAbstractLabelingEngineRule *rule )
{
  mEngineRules.emplace_back( rule );
}

void QgsLabelingEngineSettings::setRules( const QList<QgsAbstractLabelingEngineRule *> &rules )
{
  mEngineRules.clear();
  for ( QgsAbstractLabelingEngineRule *rule : rules )
  {
    mEngineRules.emplace_back( rule );
  }
}


