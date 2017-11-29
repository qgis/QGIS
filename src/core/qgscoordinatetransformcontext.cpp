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
#include "qgssettings.h"

QgsCoordinateTransformContext::QgsCoordinateTransformContext()
  : d( new QgsCoordinateTransformContextPrivate() )
{}

QgsCoordinateTransformContext::QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs ) //NOLINT
  : d( rhs.d )
{}

QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( const QgsCoordinateTransformContext &rhs ) //NOLINT
{
  d = rhs.d;
  return *this;
}

void QgsCoordinateTransformContext::clear()
{
  d.detach();
  // play it safe
  d->mLock.lockForWrite();
  d->mSourceDestDatumTransforms.clear();
#if 0
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();
#endif
  d->mLock.unlock();
}

#ifdef singlesourcedest
QMap<QString, int> QgsCoordinateTransformContext::sourceDatumTransforms() const
{
  d->mLock.lockForRead();
  auto res = d->mSourceDatumTransforms;
  res.detach();
  d->mLock.unlock();
  return res;
}

bool QgsCoordinateTransformContext::addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  d.detach();
  d->mLock.lockForWrite();
  d->mSourceDatumTransforms.insert( crs.authid(), transform );
  d->mLock.unlock();
  return true;
}

void QgsCoordinateTransformContext::removeSourceDatumTransform( const QgsCoordinateReferenceSystem &crs )
{
  d->mSourceDatumTransforms.remove( crs.authid() );
}

QMap<QString, int> QgsCoordinateTransformContext::destinationDatumTransforms() const
{
  d->mLock.lockForRead();
  auto res = d->mDestDatumTransforms;
  res.detach();
  d->mLock.unlock();
  return res;
}

bool QgsCoordinateTransformContext::addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  d.detach();

  d->mLock.lockForWrite();
  d->mDestDatumTransforms.insert( crs.authid(), transform );
  d->mLock.unlock();
  return true;
}

void QgsCoordinateTransformContext::removeDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs )
{
  d->mDestDatumTransforms.remove( crs.authid() );
}

#endif

QMap<QPair<QString, QString>, QPair<int, int> > QgsCoordinateTransformContext::sourceDestinationDatumTransforms() const
{
  d->mLock.lockForRead();
  auto res = d->mSourceDestDatumTransforms;
  res.detach();
  d->mLock.unlock();
  return res;
}

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;

  d.detach();
  d->mLock.lockForWrite();
  d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), qMakePair( sourceTransform, destinationTransform ) );
  d->mLock.unlock();
  return true;
}

void QgsCoordinateTransformContext::removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  d->mSourceDestDatumTransforms.remove( qMakePair( sourceCrs.authid(), destinationCrs.authid() ) );
}

bool QgsCoordinateTransformContext::hasTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  QPair<int, int> t = calculateDatumTransforms( source, destination );
  // calculateDatumTransforms already takes care of switching source and destination
  return t.first != -1 || t.second != -1;
}

QPair<int, int> QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  QString srcKey = source.authid();
  QString destKey = destination.authid();

  d->mLock.lockForRead();
  // highest priority is exact match for source/dest pair
  QPair< int, int > res = d->mSourceDestDatumTransforms.value( qMakePair( srcKey, destKey ), qMakePair( -1, -1 ) );
  if ( res == qMakePair( -1, -1 ) )
  {
    // try to reverse
    QPair< int, int > res2 = d->mSourceDestDatumTransforms.value( qMakePair( destKey, srcKey ), qMakePair( -1, -1 ) );
    res = qMakePair( res2.second, res2.first );
  }
  d->mLock.unlock();
  return res;

#ifdef singlesourcedest
  // fallback to checking src and dest separately
  int srcTransform = d->mSourceDatumTransforms.value( srcKey, -1 );
  int destTransform = d->mDestDatumTransforms.value( destKey, -1 );
  d->mLock.unlock();
  return qMakePair( srcTransform, destTransform );
#endif
}

void QgsCoordinateTransformContext::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  d.detach();
  d->mLock.lockForWrite();

  d->mSourceDestDatumTransforms.clear();
#if 0
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();
#endif

  const QDomNodeList contextNodes = element.elementsByTagName( QStringLiteral( "transformContext" ) );
  if ( contextNodes.count() < 1 )
  {
    d->mLock.unlock();
    return;
  }

  const QDomElement contextElem = contextNodes.at( 0 ).toElement();

  // src/dest transforms
  const QDomNodeList srcDestNodes = contextElem.elementsByTagName( QStringLiteral( "srcDest" ) );
  for ( int i = 0; i < srcDestNodes.size(); ++i )
  {
    const QDomElement transformElem = srcDestNodes.at( i ).toElement();
    QString key1 = transformElem.attribute( QStringLiteral( "source" ) );
    QString key2 = transformElem.attribute( QStringLiteral( "dest" ) );
    bool ok = false;
    int value1 = transformElem.attribute( QStringLiteral( "sourceTransform" ) ).toInt( &ok );
    bool ok2 = false;
    int value2 = transformElem.attribute( QStringLiteral( "destTransform" ) ).toInt( &ok2 );
    if ( ok && ok2 )
    {
      d->mSourceDestDatumTransforms.insert( qMakePair( key1, key2 ), qMakePair( value1, value2 ) );
    }
  }

#if 0
  // src transforms
  const QDomNodeList srcNodes = contextElem.elementsByTagName( QStringLiteral( "source" ) );
  for ( int i = 0; i < srcNodes .size(); ++i )
  {
    const QDomElement transformElem = srcNodes.at( i ).toElement();
    QString key = transformElem.attribute( QStringLiteral( "crs" ) );
    bool ok = false;
    int value = transformElem.attribute( QStringLiteral( "transform" ) ).toInt( &ok );
    if ( ok )
    {
      d->mSourceDatumTransforms.insert( key, value );
    }
  }

  // dest transforms
  const QDomNodeList destNodes = contextElem.elementsByTagName( QStringLiteral( "dest" ) );
  for ( int i = 0; i < destNodes.size(); ++i )
  {
    const QDomElement transformElem = destNodes.at( i ).toElement();
    QString key = transformElem.attribute( QStringLiteral( "crs" ) );
    bool ok = false;
    int value = transformElem.attribute( QStringLiteral( "transform" ) ).toInt( &ok );
    if ( ok )
    {
      d->mDestDatumTransforms.insert( key, value );
    }
  }
#endif

  d->mLock.unlock();
}

void QgsCoordinateTransformContext::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  d->mLock.lockForRead();

  QDomElement contextElem = document.createElement( QStringLiteral( "transformContext" ) );

  //src/dest transforms
  for ( auto it = d->mSourceDestDatumTransforms.constBegin(); it != d->mSourceDestDatumTransforms.constEnd(); ++ it )
  {
    QDomElement transformElem = document.createElement( QStringLiteral( "srcDest" ) );
    transformElem.setAttribute( QStringLiteral( "source" ), it.key().first );
    transformElem.setAttribute( QStringLiteral( "dest" ), it.key().second );
    transformElem.setAttribute( QStringLiteral( "sourceTransform" ), it.value().first );
    transformElem.setAttribute( QStringLiteral( "destTransform" ), it.value().second );
    contextElem.appendChild( transformElem );
  }

#if 0
  // src transforms
  for ( auto it = d->mSourceDatumTransforms.constBegin(); it != d->mSourceDatumTransforms.constEnd(); ++ it )
  {
    QDomElement transformElem = document.createElement( QStringLiteral( "source" ) );
    transformElem.setAttribute( QStringLiteral( "crs" ), it.key() );
    transformElem.setAttribute( QStringLiteral( "transform" ), it.value() );
    contextElem.appendChild( transformElem );
  }

  // dest transforms
  for ( auto it = d->mDestDatumTransforms.constBegin(); it != d->mDestDatumTransforms.constEnd(); ++ it )
  {
    QDomElement transformElem = document.createElement( QStringLiteral( "dest" ) );
    transformElem.setAttribute( QStringLiteral( "crs" ), it.key() );
    transformElem.setAttribute( QStringLiteral( "transform" ), it.value() );
    contextElem.appendChild( transformElem );
  }
#endif

  element.appendChild( contextElem );
  d->mLock.unlock();
}

void QgsCoordinateTransformContext::readSettings()
{
  d.detach();
  d->mLock.lockForWrite();

  d->mSourceDestDatumTransforms.clear();
#if 0
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();
#endif

  QgsSettings *settings = new QgsSettings();
  settings->beginGroup( QStringLiteral( "/Projections" ) );
  QStringList projectionKeys = settings->allKeys();

  //collect src and dest entries that belong together
  QMap< QPair< QString, QString >, QPair< int, int > > transforms;
  QStringList::const_iterator pkeyIt = projectionKeys.constBegin();
  for ( ; pkeyIt != projectionKeys.constEnd(); ++pkeyIt )
  {
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

      if ( pkeyIt->contains( QLatin1String( "srcTransform" ) ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].first = settings->value( *pkeyIt ).toInt();
      }
      else if ( pkeyIt->contains( QLatin1String( "destTransform" ) ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].second = settings->value( *pkeyIt ).toInt();
      }
    }
  }

  // add transforms to context
  QMap< QPair< QString, QString >, QPair< int, int > >::const_iterator transformIt = transforms.constBegin();
  for ( ; transformIt != transforms.constEnd(); ++transformIt )
  {
    d->mSourceDestDatumTransforms.insert( transformIt.key(), transformIt.value() );
  }

  d->mLock.unlock();
  settings->endGroup();
}

void QgsCoordinateTransformContext::writeSettings()
{
  QgsSettings *settings = new QgsSettings();
  settings->beginGroup( QStringLiteral( "/Projections" ) );
  QStringList groupKeys = settings->allKeys();
  QStringList::const_iterator groupKeyIt = groupKeys.constBegin();
  for ( ; groupKeyIt != groupKeys.constEnd(); ++groupKeyIt )
  {
    if ( groupKeyIt->contains( QLatin1String( "srcTransform" ) ) || groupKeyIt->contains( QLatin1String( "destTransform" ) ) )
    {
      settings->remove( *groupKeyIt );
    }
  }

  QMap< QPair< QString, QString >, QPair< int, int > >::const_iterator transformIt = d->mSourceDestDatumTransforms.constBegin();
  for ( ; transformIt != d->mSourceDestDatumTransforms.constEnd(); ++transformIt )
  {
    QString srcAuthId = transformIt.key().first;
    QString destAuthId = transformIt.key().second;
    int sourceDatumTransform = transformIt.value().first;
    int destinationDatumTransform = transformIt.value().second;

    settings->setValue( srcAuthId + "//" + destAuthId + "_srcTransform", sourceDatumTransform );
    settings->setValue( srcAuthId + "//" + destAuthId + "_destTransform", destinationDatumTransform );
  }

  settings->endGroup();
}
