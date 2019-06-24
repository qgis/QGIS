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


QgsCoordinateTransformContext::QgsCoordinateTransformContext()
  : d( new QgsCoordinateTransformContextPrivate() )
{}

QgsCoordinateTransformContext::~QgsCoordinateTransformContext() = default;

QgsCoordinateTransformContext::QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs ) //NOLINT
  : d( rhs.d )
{}

QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( const QgsCoordinateTransformContext &rhs ) //NOLINT
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
  bool equal = d->mSourceDestDatumTransforms == rhs.d->mSourceDestDatumTransforms;
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
#if PROJ_VERSION_MAJOR>=6
  return QMap<QPair<QString, QString>, QgsDatumTransform::TransformPair>();
#else
  d->mLock.lockForRead();
  auto res = d->mSourceDestDatumTransforms;
  res.detach();
  d->mLock.unlock();
  return res;
#endif
}

QMap<QPair<QString, QString>, QString> QgsCoordinateTransformContext::coordinateOperations() const
{
#if PROJ_VERSION_MAJOR>=6
  d->mLock.lockForRead();
  auto res = d->mSourceDestDatumTransforms;
  res.detach();
  d->mLock.unlock();
  return res;
#else
  return QMap<QPair<QString, QString>, QString>();
#endif
}

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;
#if PROJ_VERSION_MAJOR>=6
  Q_UNUSED( sourceTransform )
  Q_UNUSED( destinationTransform )
  return false;
#else
  d.detach();
  d->mLock.lockForWrite();
  d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), QgsDatumTransform::TransformPair( sourceTransform, destinationTransform ) );
  d->mLock.unlock();
  return true;
#endif
}

bool QgsCoordinateTransformContext::addCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &coordinateOperationProjString )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;
#if PROJ_VERSION_MAJOR>=6
  d.detach();
  d->mLock.lockForWrite();
  d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), coordinateOperationProjString );
  d->mLock.unlock();
  return true;
#else
  Q_UNUSED( coordinateOperationProjString )
  return false;
#endif
}

void QgsCoordinateTransformContext::removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  removeCoordinateOperation( sourceCrs, destinationCrs );
}

void QgsCoordinateTransformContext::removeCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  d->mSourceDestDatumTransforms.remove( qMakePair( sourceCrs.authid(), destinationCrs.authid() ) );
}

bool QgsCoordinateTransformContext::hasTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
#if PROJ_VERSION_MAJOR>=6
  const QString t = calculateCoordinateOperation( source, destination );
  return !t.isEmpty();
#else
  Q_NOWARN_DEPRECATED_PUSH
  QgsDatumTransform::TransformPair t = calculateDatumTransforms( source, destination );
  Q_NOWARN_DEPRECATED_POP
  // calculateDatumTransforms already takes care of switching source and destination
  return t.sourceTransformId != -1 || t.destinationTransformId != -1;
#endif
}

QgsDatumTransform::TransformPair QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
#if PROJ_VERSION_MAJOR>=6
  Q_UNUSED( source )
  Q_UNUSED( destination )
  return QgsDatumTransform::TransformPair( -1, -1 );
#else
  QString srcKey = source.authid();
  QString destKey = destination.authid();

  d->mLock.lockForRead();
  // highest priority is exact match for source/dest pair
  QgsDatumTransform::TransformPair res = d->mSourceDestDatumTransforms.value( qMakePair( srcKey, destKey ), QgsDatumTransform::TransformPair( -1, -1 ) );
  if ( res.sourceTransformId == -1 && res.destinationTransformId == -1 )
  {
    // try to reverse
    QgsDatumTransform::TransformPair res2 = d->mSourceDestDatumTransforms.value( qMakePair( destKey, srcKey ), QgsDatumTransform::TransformPair( -1, -1 ) );
    res = QgsDatumTransform::TransformPair( res2.destinationTransformId, res2.sourceTransformId );
  }
  d->mLock.unlock();
  return res;
#endif
}

QString QgsCoordinateTransformContext::calculateCoordinateOperation( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
#if PROJ_VERSION_MAJOR>=6
  const QString srcKey = source.authid();
  const QString destKey = destination.authid();

  d->mLock.lockForRead();
  QString res = d->mSourceDestDatumTransforms.value( qMakePair( srcKey, destKey ), QString() );
  if ( res.isEmpty() )
  {
    // try to reverse
    res = d->mSourceDestDatumTransforms.value( qMakePair( destKey, srcKey ), QString() );
  }
  d->mLock.unlock();
  return res;
#else
  Q_UNUSED( source )
  Q_UNUSED( destination )
  return QString();
#endif
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
    const QString key1 = transformElem.attribute( QStringLiteral( "source" ) );
    const QString key2 = transformElem.attribute( QStringLiteral( "dest" ) );

#if PROJ_VERSION_MAJOR>=6
    const QString coordinateOp = transformElem.attribute( QStringLiteral( "coordinateOp" ) );

    // try to instantiate operation, and check for missing grids
    if ( !QgsProjUtils::coordinateOperationIsAvailable( coordinateOp ) )
    {
      // not possible in current Proj 6 api!
      // QgsCoordinateTransform will alert users to this, we don't need to use missingTransforms here
      result = false;
    }

    d->mSourceDestDatumTransforms.insert( qMakePair( key1, key2 ), coordinateOp );
#else
    QString value1 = transformElem.attribute( QStringLiteral( "sourceTransform" ) );
    QString value2 = transformElem.attribute( QStringLiteral( "destTransform" ) );

    Q_NOWARN_DEPRECATED_PUSH
    int datumId1 = -1;
    int datumId2 = -1;
    //warn if value1 or value2 is non-empty, yet no matching transform was found
    if ( !value1.isEmpty() )
    {
      datumId1 = QgsDatumTransform::projStringToDatumTransformId( value1 );
      if ( datumId1 < 0 )
      {
        result = false;
        missingTransforms << value1;
      }
    }
    if ( !value2.isEmpty() )
    {
      datumId2 = QgsDatumTransform::projStringToDatumTransformId( value2 );
      if ( datumId2 < 0 )
      {
        result = false;
        missingTransforms << value2;
      }
    }
    Q_NOWARN_DEPRECATED_POP

    d->mSourceDestDatumTransforms.insert( qMakePair( key1, key2 ), QgsDatumTransform::TransformPair( datumId1, datumId2 ) );
#endif
  }

  d->mLock.unlock();
  return result;
}

void QgsCoordinateTransformContext::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  d->mLock.lockForRead();

  QDomElement contextElem = element.ownerDocument().createElement( QStringLiteral( "transformContext" ) );

  //src/dest transforms
  for ( auto it = d->mSourceDestDatumTransforms.constBegin(); it != d->mSourceDestDatumTransforms.constEnd(); ++ it )
  {
    QDomElement transformElem = element.ownerDocument().createElement( QStringLiteral( "srcDest" ) );
    transformElem.setAttribute( QStringLiteral( "source" ), it.key().first );
    transformElem.setAttribute( QStringLiteral( "dest" ), it.key().second );
#if PROJ_VERSION_MAJOR>=6
    transformElem.setAttribute( QStringLiteral( "coordinateOp" ), it.value() );
#else
    Q_NOWARN_DEPRECATED_PUSH
    transformElem.setAttribute( QStringLiteral( "sourceTransform" ), it.value().sourceTransformId < 0 ? QString() : QgsDatumTransform::datumTransformToProj( it.value().sourceTransformId ) );
    transformElem.setAttribute( QStringLiteral( "destTransform" ), it.value().destinationTransformId < 0 ? QString() : QgsDatumTransform::datumTransformToProj( it.value().destinationTransformId ) );
    Q_NOWARN_DEPRECATED_POP
#endif
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
  QStringList projectionKeys = settings.allKeys();

  //collect src and dest entries that belong together
#if PROJ_VERSION_MAJOR>=6
  QMap< QPair< QString, QString >, QString > transforms;
#else
  QMap< QPair< QString, QString >, QPair< int, int > > transforms;
#endif
  QStringList::const_iterator pkeyIt = projectionKeys.constBegin();
  for ( ; pkeyIt != projectionKeys.constEnd(); ++pkeyIt )
  {
#if PROJ_VERSION_MAJOR>=6
    if ( pkeyIt->contains( QLatin1String( "coordinateOp" ) ) )
    {
      QStringList split = pkeyIt->split( '/' );
      QString srcAuthId, destAuthId;
      if ( ! split.isEmpty() )
      {
        srcAuthId = split.at( 0 );
      }
      if ( split.size() > 1 )
      {
        destAuthId = split.at( 1 ).split( '_' ).at( 0 );
      }

      const QString proj = settings.value( *pkeyIt ).toString();
      transforms[ qMakePair( srcAuthId, destAuthId )] = proj;
    }
#else
    if ( pkeyIt->contains( QLatin1String( "srcTransform" ) ) || pkeyIt->contains( QLatin1String( "destTransform" ) ) )
    {
      QStringList split = pkeyIt->split( '/' );
      QString srcAuthId, destAuthId;
      if ( ! split.isEmpty() )
      {
        srcAuthId = split.at( 0 );
      }
      if ( split.size() > 1 )
      {
        destAuthId = split.at( 1 ).split( '_' ).at( 0 );
      }

      QString proj = settings.value( *pkeyIt ).toString();
      Q_NOWARN_DEPRECATED_PUSH
      int datumId = QgsDatumTransform::projStringToDatumTransformId( proj );
      Q_NOWARN_DEPRECATED_POP
      if ( pkeyIt->contains( QLatin1String( "srcTransform" ) ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].first = datumId;
      }
      else if ( pkeyIt->contains( QLatin1String( "destTransform" ) ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].second = datumId;
      }
    }
#endif
  }

  // add transforms to context
  auto transformIt = transforms.constBegin();
  for ( ; transformIt != transforms.constEnd(); ++transformIt )
  {
#if PROJ_VERSION_MAJOR>=6
    d->mSourceDestDatumTransforms.insert( transformIt.key(), transformIt.value() );
#else
    d->mSourceDestDatumTransforms.insert( transformIt.key(), QgsDatumTransform::TransformPair( transformIt.value().first, transformIt.value().second ) );
#endif
  }

  d->mLock.unlock();
  settings.endGroup();
}

void QgsCoordinateTransformContext::writeSettings()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/Projections" ) );
  QStringList groupKeys = settings.allKeys();
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
    const QString srcAuthId = transformIt.key().first;
    const QString destAuthId = transformIt.key().second;

#if PROJ_VERSION_MAJOR>=6
    const QString proj = transformIt.value();
    settings.setValue( srcAuthId + "//" + destAuthId + "_coordinateOp", proj );
#else
    int sourceDatumTransform = transformIt.value().sourceTransformId;
    QString sourceDatumProj;
    Q_NOWARN_DEPRECATED_PUSH
    if ( sourceDatumTransform >= 0 )
      sourceDatumProj = QgsDatumTransform::datumTransformToProj( sourceDatumTransform );
    int destinationDatumTransform = transformIt.value().destinationTransformId;
    QString destinationDatumProj;
    if ( destinationDatumTransform >= 0 )
      destinationDatumProj = QgsDatumTransform::datumTransformToProj( destinationDatumTransform );
    Q_NOWARN_DEPRECATED_POP

    settings.setValue( srcAuthId + "//" + destAuthId + "_srcTransform", sourceDatumProj );
    settings.setValue( srcAuthId + "//" + destAuthId + "_destTransform", destinationDatumProj );
#endif
  }

  settings.endGroup();
}
