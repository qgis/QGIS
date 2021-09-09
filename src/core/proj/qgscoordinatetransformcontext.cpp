/***************************************************************************
                         qgscoordinatetransformcontext.cpp
                         ---------------------------------
    begin                : November 2017
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

#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransformcontext_p.h"
#include "qgscoordinatetransform.h"
#include "qgssettings.h"
#include "qgsprojutils.h"

QString crsToKey( const QgsCoordinateReferenceSystem &crs )
{
  return crs.authid().isEmpty() ? crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) : crs.authid();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template<>
bool qMapLessThanKey<QPair<QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem>>( const QPair<QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem> &key1,
    const QPair<QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem> &key2 )
{
  const QPair< QString, QString > key1String = qMakePair( crsToKey( key1.first ), crsToKey( key1.second ) );
  const QPair< QString, QString > key2String = qMakePair( crsToKey( key2.first ), crsToKey( key2.second ) );
  return key1String < key2String;
}
#endif

QgsCoordinateTransformContext::QgsCoordinateTransformContext()
  : d( new QgsCoordinateTransformContextPrivate() )
{}

QgsCoordinateTransformContext::~QgsCoordinateTransformContext() = default;

QgsCoordinateTransformContext::QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs )  //NOLINT
  : d( rhs.d )
{}

QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( const QgsCoordinateTransformContext &rhs )  //NOLINT
{
  d = rhs.d;
  return *this;
}

bool QgsCoordinateTransformContext::operator==( const QgsCoordinateTransformContext &rhs ) const
{
  if ( d == rhs.d )
    return true;

  d->mLock.lockForRead();
  rhs.d->mLock.lockForRead();
  const bool equal = d->mSourceDestDatumTransforms == rhs.d->mSourceDestDatumTransforms;
  d->mLock.unlock();
  rhs.d->mLock.unlock();
  return equal;
}

void QgsCoordinateTransformContext::clear()
{
  d.detach();
  // play it safe
  d->mLock.lockForWrite();
  d->mSourceDestDatumTransforms.clear();
  d->mLock.unlock();
}

QMap<QPair<QString, QString>, QgsDatumTransform::TransformPair> QgsCoordinateTransformContext::sourceDestinationDatumTransforms() const
{
  return QMap<QPair<QString, QString>, QgsDatumTransform::TransformPair>();
}

QMap<QPair<QString, QString>, QString> QgsCoordinateTransformContext::coordinateOperations() const
{
  d->mLock.lockForRead();
  auto res = d->mSourceDestDatumTransforms;
  res.detach();
  d->mLock.unlock();
  QMap<QPair<QString, QString>, QString> results;
  for ( auto it = res.constBegin(); it != res.constEnd(); ++it )
    results.insert( qMakePair( it.key().first.authid(), it.key().second.authid() ), it.value().operation );

  return results;
}

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;
  Q_UNUSED( sourceTransform )
  Q_UNUSED( destinationTransform )
  return false;
}

bool QgsCoordinateTransformContext::addCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &coordinateOperationProjString, bool allowFallback )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;
  d.detach();
  d->mLock.lockForWrite();
  QgsCoordinateTransformContextPrivate::OperationDetails details;
  details.operation = coordinateOperationProjString;
  details.allowFallback = allowFallback;
  d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs, destinationCrs ), details );
  d->mLock.unlock();
  return true;
}

void QgsCoordinateTransformContext::removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  removeCoordinateOperation( sourceCrs, destinationCrs );
}

void QgsCoordinateTransformContext::removeCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  d->mSourceDestDatumTransforms.remove( qMakePair( sourceCrs, destinationCrs ) );
}

bool QgsCoordinateTransformContext::hasTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  const QString t = calculateCoordinateOperation( source, destination );
  return !t.isEmpty();
}

QgsDatumTransform::TransformPair QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  Q_UNUSED( source )
  Q_UNUSED( destination )
  return QgsDatumTransform::TransformPair( -1, -1 );
}

QString QgsCoordinateTransformContext::calculateCoordinateOperation( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  if ( !source.isValid() || !destination.isValid() )
    return QString();

  d->mLock.lockForRead();
  QgsCoordinateTransformContextPrivate::OperationDetails res = d->mSourceDestDatumTransforms.value( qMakePair( source, destination ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  if ( res.operation.isEmpty() )
  {
    // try to reverse
    res = d->mSourceDestDatumTransforms.value( qMakePair( destination, source ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  }
  d->mLock.unlock();
  return res.operation;
}

bool QgsCoordinateTransformContext::allowFallbackTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  if ( !source.isValid() || !destination.isValid() )
    return false;

  d->mLock.lockForRead();
  QgsCoordinateTransformContextPrivate::OperationDetails res = d->mSourceDestDatumTransforms.value( qMakePair( source, destination ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  if ( res.operation.isEmpty() )
  {
    // try to reverse
    res = d->mSourceDestDatumTransforms.value( qMakePair( destination, source ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  }
  d->mLock.unlock();
  return res.allowFallback;
}

bool QgsCoordinateTransformContext::mustReverseCoordinateOperation( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  if ( !source.isValid() || !destination.isValid() )
    return false;

  d->mLock.lockForRead();
  QgsCoordinateTransformContextPrivate::OperationDetails res = d->mSourceDestDatumTransforms.value( qMakePair( source, destination ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  if ( !res.operation.isEmpty() )
  {
    d->mLock.unlock();
    return false;
  }
  // see if the reverse operation is present
  res = d->mSourceDestDatumTransforms.value( qMakePair( destination, source ), QgsCoordinateTransformContextPrivate::OperationDetails() );
  if ( !res.operation.isEmpty() )
  {
    d->mLock.unlock();
    return true;
  }

  d->mLock.unlock();
  return false;
}

bool QgsCoordinateTransformContext::readXml( const QDomElement &element, const QgsReadWriteContext &, QStringList &missingTransforms )
{
  d.detach();
  d->mLock.lockForWrite();

  d->mSourceDestDatumTransforms.clear();

  const QDomNodeList contextNodes = element.elementsByTagName( QStringLiteral( "transformContext" ) );
  if ( contextNodes.count() < 1 )
  {
    d->mLock.unlock();
    return true;
  }

  missingTransforms.clear();
  bool result = true;

  const QDomElement contextElem = contextNodes.at( 0 ).toElement();

  // src/dest transforms
  const QDomNodeList srcDestNodes = contextElem.elementsByTagName( QStringLiteral( "srcDest" ) );
  for ( int i = 0; i < srcDestNodes.size(); ++i )
  {
    const QDomElement transformElem = srcDestNodes.at( i ).toElement();

    const QDomElement srcElem = transformElem.firstChildElement( QStringLiteral( "src" ) );
    const QDomElement destElem = transformElem.firstChildElement( QStringLiteral( "dest" ) );

    QgsCoordinateReferenceSystem srcCrs;
    QgsCoordinateReferenceSystem destCrs;
    if ( !srcElem.isNull() && !destElem.isNull() )
    {
      srcCrs.readXml( srcElem );
      destCrs.readXml( destElem );
    }
    else
    {
      // for older project compatibility
      const QString key1 = transformElem.attribute( QStringLiteral( "source" ) );
      const QString key2 = transformElem.attribute( QStringLiteral( "dest" ) );
      srcCrs = QgsCoordinateReferenceSystem( key1 );
      destCrs = QgsCoordinateReferenceSystem( key2 );
    }

    if ( !srcCrs.isValid() || !destCrs.isValid() )
      continue;

    const QString coordinateOp = transformElem.attribute( QStringLiteral( "coordinateOp" ) );
    const bool allowFallback = transformElem.attribute( QStringLiteral( "allowFallback" ), QStringLiteral( "1" ) ).toInt();

    // try to instantiate operation, and check for missing grids
    if ( !QgsProjUtils::coordinateOperationIsAvailable( coordinateOp ) )
    {
      // not possible in current Proj 6 api!
      // QgsCoordinateTransform will alert users to this, we don't need to use missingTransforms here
      result = false;
    }

    QgsCoordinateTransformContextPrivate::OperationDetails deets;
    deets.operation = coordinateOp;
    deets.allowFallback = allowFallback;
    d->mSourceDestDatumTransforms.insert( qMakePair( srcCrs, destCrs ), deets );
  }

  d->mLock.unlock();
  return result;
}

void QgsCoordinateTransformContext::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  d->mLock.lockForRead();

  QDomDocument doc = element.ownerDocument();

  QDomElement contextElem = doc.createElement( QStringLiteral( "transformContext" ) );

  //src/dest transforms
  for ( auto it = d->mSourceDestDatumTransforms.constBegin(); it != d->mSourceDestDatumTransforms.constEnd(); ++ it )
  {
    QDomElement transformElem = doc.createElement( QStringLiteral( "srcDest" ) );
    QDomElement srcElem = doc.createElement( QStringLiteral( "src" ) );
    QDomElement destElem = doc.createElement( QStringLiteral( "dest" ) );

    it.key().first.writeXml( srcElem, doc );
    it.key().second.writeXml( destElem, doc );

    transformElem.appendChild( srcElem );
    transformElem.appendChild( destElem );

    transformElem.setAttribute( QStringLiteral( "coordinateOp" ), it.value().operation );
    transformElem.setAttribute( QStringLiteral( "allowFallback" ), it.value().allowFallback ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    contextElem.appendChild( transformElem );
  }

  element.appendChild( contextElem );
  d->mLock.unlock();
}

void QgsCoordinateTransformContext::readSettings()
{
  d.detach();
  d->mLock.lockForWrite();

  d->mSourceDestDatumTransforms.clear();

  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/Projections" ) );
  const QStringList projectionKeys = settings.allKeys();

  //collect src and dest entries that belong together
  QMap< QPair< QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem >, QgsCoordinateTransformContextPrivate::OperationDetails > transforms;
  QStringList::const_iterator pkeyIt = projectionKeys.constBegin();
  for ( ; pkeyIt != projectionKeys.constEnd(); ++pkeyIt )
  {
    if ( pkeyIt->contains( QLatin1String( "coordinateOp" ) ) )
    {
      const QStringList split = pkeyIt->split( '/' );
      QString srcAuthId, destAuthId;
      if ( ! split.isEmpty() )
      {
        srcAuthId = split.at( 0 );
      }
      if ( split.size() > 1 )
      {
        destAuthId = split.at( 1 ).split( '_' ).at( 0 );
      }

      if ( srcAuthId.isEmpty() || destAuthId.isEmpty() )
        continue;

      const QString proj = settings.value( *pkeyIt ).toString();
      const bool allowFallback = settings.value( QStringLiteral( "%1//%2_allowFallback" ).arg( srcAuthId, destAuthId ) ).toBool();
      QgsCoordinateTransformContextPrivate::OperationDetails deets;
      deets.operation = proj;
      deets.allowFallback = allowFallback;
      transforms[ qMakePair( QgsCoordinateReferenceSystem( srcAuthId ), QgsCoordinateReferenceSystem( destAuthId ) )] = deets;
    }
  }

  // add transforms to context
  auto transformIt = transforms.constBegin();
  for ( ; transformIt != transforms.constEnd(); ++transformIt )
  {
    d->mSourceDestDatumTransforms.insert( transformIt.key(), transformIt.value() );
  }

  d->mLock.unlock();
  settings.endGroup();
}

void QgsCoordinateTransformContext::writeSettings()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/Projections" ) );
  const QStringList groupKeys = settings.allKeys();
  QStringList::const_iterator groupKeyIt = groupKeys.constBegin();
  for ( ; groupKeyIt != groupKeys.constEnd(); ++groupKeyIt )
  {
    if ( groupKeyIt->contains( QLatin1String( "srcTransform" ) ) || groupKeyIt->contains( QLatin1String( "destTransform" ) ) || groupKeyIt->contains( QLatin1String( "coordinateOp" ) ) )
    {
      settings.remove( *groupKeyIt );
    }
  }

  for ( auto transformIt = d->mSourceDestDatumTransforms.constBegin(); transformIt != d->mSourceDestDatumTransforms.constEnd(); ++transformIt )
  {
    const QString srcAuthId = transformIt.key().first.authid();
    const QString destAuthId = transformIt.key().second.authid();

    if ( srcAuthId.isEmpty() || destAuthId.isEmpty() )
      continue; // not so nice, but alternative would be to shove whole CRS wkt into the settings values...

    const QString proj = transformIt.value().operation;
    const bool allowFallback = transformIt.value().allowFallback;
    settings.setValue( srcAuthId + "//" + destAuthId + "_coordinateOp", proj );
    settings.setValue( srcAuthId + "//" + destAuthId + "_allowFallback", allowFallback );
  }

  settings.endGroup();
}
