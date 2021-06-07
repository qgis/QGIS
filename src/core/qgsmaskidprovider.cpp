/***************************************************************************
 qgsmaskidprovider.cpp
 ---------------------
 begin                : August 2019
 copyright            : (C) 2019 by Hugo Mercier / Oslandia
 email                : infos at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaskidprovider.h"

int QgsMaskIdProvider::insertLabelLayer( const QString &layerId, const QString &ruleId, const QSet<QgsSymbolLayerReference> &maskedSymbolLayers )
{
  QString strId = layerId + ruleId;
  // look for an existing symbol layer set
  int maskId = mLabelLayers.indexOf( maskedSymbolLayers );
  if ( maskId != -1 )
  {
    // add the layer id / rule id to the existing mask id
    mMaskIds[maskId].insert( strId );
    return maskId;
  }
  //  else
  mLabelLayers.push_back( maskedSymbolLayers );
  mMaskIds.push_back( QSet<QString>() << strId );
  return mMaskIds.size() - 1;
}

int QgsMaskIdProvider::maskId( const QString &labelLayerId, const QString &labelRuleId ) const
{
  if ( labelLayerId.isEmpty() )
    return -1;

  QString id = labelLayerId + labelRuleId;
  for ( int i = 0; i < mMaskIds.size(); i++ )
  {
    if ( mMaskIds[i].contains( id ) )
      return i;
  }
  return -1;
}

int QgsMaskIdProvider::size() const
{
  return mMaskIds.size();
}
