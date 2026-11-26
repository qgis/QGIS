/***************************************************************************
  qgsfieldpropertycollectortask.cpp
  --------------------------------------
  Date                 : November 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldpropertycollectortask.h"

#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "moc_qgsfieldpropertycollectortask.cpp"

using namespace Qt::StringLiterals;

QgsFieldPropertyCollectorTask::QgsFieldPropertyCollectorTask( QgsVectorLayer *layer )
  : QgsTask( tr( "Calculating maximum field lengths" ), QgsTask::CanCancel | QgsTask::Silent | QgsTask::CancelWithoutPrompt )
  , mSource( std::make_unique<QgsVectorLayerFeatureSource>( layer ) )
  , mFields( layer->fields() )
{}

QgsFieldPropertyCollectorTask::~QgsFieldPropertyCollectorTask() = default;

bool QgsFieldPropertyCollectorTask::run()
{
  QgsAttributeList stringFieldIndexes;
  mMaxFieldLengths.resize( mFields.count() );
  for ( int i = 0; i < mFields.count(); ++i )
  {
    mMaxFieldLengths[i] = 0;
    if ( mFields.at( i ).type() == QMetaType::Type::QString )
    {
      stringFieldIndexes << i;
    }
  }

  if ( stringFieldIndexes.isEmpty() ) // nothing to do
    return true;

  QgsFeatureRequest req;
  req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  req.setSubsetOfAttributes( stringFieldIndexes );

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures( req );
  while ( it.nextFeature( f ) )
  {
    if ( isCanceled() )
      return false;

    for ( int idx : std::as_const( stringFieldIndexes ) )
    {
      const QVariant val = f.attribute( idx );
      if ( !QgsVariantUtils::isNull( val ) )
      {
        const qsizetype len = val.toString().length();
        mMaxFieldLengths[idx] = std::max( mMaxFieldLengths[idx], len );
      }
    }
  }
  return true;
}
