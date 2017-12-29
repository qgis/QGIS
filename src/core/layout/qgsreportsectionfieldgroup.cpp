/***************************************************************************
                             qgsreportsectionfieldgroup.cpp
                             --------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsreportsectionfieldgroup.h"
#include "qgslayout.h"

///@cond NOT_STABLE

QgsReportSectionFieldGroup::QgsReportSectionFieldGroup( QgsAbstractReportSection *parent )
  : QgsAbstractReportSection( parent )
{

}

QgsReportSectionFieldGroup *QgsReportSectionFieldGroup::clone() const
{
  std::unique_ptr< QgsReportSectionFieldGroup > copy = qgis::make_unique< QgsReportSectionFieldGroup >( nullptr );
  copyCommonProperties( copy.get() );

  if ( mBody )
  {
    copy->mBody.reset( mBody->clone() );
  }
  else
    copy->mBody.reset();

  copy->setLayer( mCoverageLayer.get() );
  copy->setField( mField );

  return copy.release();
}

bool QgsReportSectionFieldGroup::beginRender()
{
  if ( !mCoverageLayer.get() )
    return false;

  if ( !mField.isEmpty() )
  {
    mFieldIndex = mCoverageLayer->fields().lookupField( mField );
    if ( mFieldIndex < 0 )
      return false;

    if ( mBody )
      mBody->reportContext().setLayer( mCoverageLayer.get() );

    mFeatures = QgsFeatureIterator();
  }
  return QgsAbstractReportSection::beginRender();
}

QgsLayout *QgsReportSectionFieldGroup::nextBody( bool &ok )
{
  if ( !mFeatures.isValid() )
  {
    mFeatures = mCoverageLayer->getFeatures( buildFeatureRequest() );
  }

  QgsFeature f = getNextFeature();
  if ( !f.isValid() )
  {
    // no features left for this iteration
    mFeatures = QgsFeatureIterator();
    ok = false;
    return nullptr;
  }

  updateChildContexts( f );

  ok = true;
  if ( mBody )
  {
    mBody->reportContext().blockSignals( true );
    mBody->reportContext().setLayer( mCoverageLayer.get() );
    mBody->reportContext().blockSignals( false );
    mBody->reportContext().setFeature( f );
  }

  return mBody.get();
}

void QgsReportSectionFieldGroup::reset()
{
  QgsAbstractReportSection::reset();
  mEncounteredValues.clear();
}

QgsFeatureRequest QgsReportSectionFieldGroup::buildFeatureRequest() const
{
  QgsFeatureRequest request;
  QString filter = context().layerFilters.value( mCoverageLayer.get() );
  if ( !filter.isEmpty() )
    request.setFilterExpression( filter );
  request.addOrderBy( mField, true );
  return request;
}

QgsFeature QgsReportSectionFieldGroup::getNextFeature()
{
  QgsFeature f;
  QVariant currentValue;
  bool first = true;
  while ( first || ( !mBody && mEncounteredValues.contains( currentValue ) ) )
  {
    if ( !mFeatures.nextFeature( f ) )
    {
      return QgsFeature();
    }

    first = false;
    currentValue = f.attribute( mFieldIndex );
  }

  mEncounteredValues.insert( currentValue );
  return f;
}

void QgsReportSectionFieldGroup::updateChildContexts( const QgsFeature &feature )
{
  QgsReportSectionContext c = context();
  QString currentFilter = c.layerFilters.value( mCoverageLayer.get() );
  QString thisFilter = QgsExpression::createFieldEqualityExpression( mField, feature.attribute( mFieldIndex ) );
  QString newFilter = currentFilter.isEmpty() ? thisFilter : QStringLiteral( "(%1) AND (%2)" ).arg( currentFilter, thisFilter );
  c.layerFilters[ mCoverageLayer.get() ] = newFilter;

  const QList< QgsAbstractReportSection * > sections = children();
  for ( QgsAbstractReportSection *section : qgis::as_const( sections ) )
  {
    section->setContext( c );
  }
}

///@endcond

