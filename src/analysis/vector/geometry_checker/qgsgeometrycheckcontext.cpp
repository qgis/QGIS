/***************************************************************************
    qgsgeometrycheckcontext.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include <QThread>

QgsGeometryCheckContext::QgsGeometryCheckContext( int precision, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &transformContext, const QgsProject *project )
  : tolerance( std::pow( 10, -precision ) )
  , reducedTolerance( std::pow( 10, -precision / 2 ) )
  , mapCrs( mapCrs )
  , transformContext( transformContext )
  , mProject( project )
{
}

const QgsProject *QgsGeometryCheckContext::project() const
{
  Q_ASSERT( qApp->thread() == QThread::currentThread() );
  return mProject;
}
