/***************************************************************************
    qgsgeometrycheckregistry.cpp
     --------------------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckregistry.h"
#include "qgsgeometrycheckfactory.h"
#include "qgis.h"


QgsGeometryCheckRegistry::~QgsGeometryCheckRegistry()
{
  qDeleteAll( mGeometryCheckFactories );
}

QgsGeometryCheck *QgsGeometryCheckRegistry::geometryCheck( const QString &checkId, QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfig )
{
  QgsGeometryCheckFactory *factory = mGeometryCheckFactories.value( checkId );
  if ( factory )
    return factory->createGeometryCheck( context, geometryCheckConfig );
  else
    return nullptr;
}

QList<QgsGeometryCheckFactory *> QgsGeometryCheckRegistry::geometryCheckFactories( QgsVectorLayer *layer, QgsGeometryCheck::CheckType type, QgsGeometryCheck::Flags flags ) const
{
  QList<QgsGeometryCheckFactory *> factories;
  for ( QgsGeometryCheckFactory *factory : mGeometryCheckFactories )
  {
    if ( factory->checkType() == type && ( factory->flags() & flags ) == flags && factory->isCompatible( layer ) )
      factories << factory;
  }
  return factories;
}

void QgsGeometryCheckRegistry::registerGeometryCheck( QgsGeometryCheckFactory *checkFactory )
{
  mGeometryCheckFactories.insert( checkFactory->id(), checkFactory );
}
