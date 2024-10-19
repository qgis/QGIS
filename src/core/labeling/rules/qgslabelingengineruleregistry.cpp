/***************************************************************************
    qgslabelingengineruleregistry.cpp
    ---------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslabelingengineruleregistry.h"
#include "qgslabelingenginerule.h"
#include "qgslabelingenginerule_impl.h"
#include <geos_c.h>

QgsLabelingEngineRuleRegistry::QgsLabelingEngineRuleRegistry()
{
  addRule( new QgsLabelingEngineRuleMinimumDistanceLabelToFeature() );
  addRule( new QgsLabelingEngineRuleMaximumDistanceLabelToFeature() );
  addRule( new QgsLabelingEngineRuleMinimumDistanceLabelToLabel() );
  addRule( new QgsLabelingEngineRuleAvoidLabelOverlapWithFeature() );
}

QgsLabelingEngineRuleRegistry::~QgsLabelingEngineRuleRegistry() = default;

QStringList QgsLabelingEngineRuleRegistry::ruleIds() const
{
  QStringList res;
  res.reserve( static_cast< int >( mRules.size() ) );
  for ( auto &it : mRules )
  {
    res.append( it.first );
  }
  return res;
}

QString QgsLabelingEngineRuleRegistry::displayType( const QString &id ) const
{
  auto it = mRules.find( id );
  if ( it == mRules.end() )
    return QString();

  return it->second->displayType();
}

bool QgsLabelingEngineRuleRegistry::isAvailable( const QString &id ) const
{
  auto it = mRules.find( id );
  if ( it == mRules.end() )
    return false;

  return it->second->isAvailable();
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRuleRegistry::create( const QString &id ) const
{
  auto it = mRules.find( id );
  if ( it == mRules.end() )
    return nullptr;

  return it->second->clone();
}

bool QgsLabelingEngineRuleRegistry::addRule( QgsAbstractLabelingEngineRule *rule )
{
  if ( !rule )
    return false;

  if ( mRules.find( rule->id() ) != mRules.end() )
  {
    delete rule;
    return false;
  }

  mRules[ rule->id() ] = std::unique_ptr< QgsAbstractLabelingEngineRule >( rule );
  return true;
}

void QgsLabelingEngineRuleRegistry::removeRule( const QString &id )
{
  mRules.erase( id );
}

