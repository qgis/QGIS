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
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();
  d->mLock.unlock();
}

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
  if ( transform == -1 )
  {
    d->mSourceDatumTransforms.remove( crs.authid() );
  }
  else
  {
    d->mSourceDatumTransforms.insert( crs.authid(), transform );
  }
  d->mLock.unlock();
  return true;
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
  if ( transform == -1 )
  {
    d->mDestDatumTransforms.remove( crs.authid() );
  }
  else
  {
    d->mDestDatumTransforms.insert( crs.authid(), transform );
  }
  d->mLock.unlock();
  return true;
}

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
  if ( sourceTransform == -1 && destinationTransform == -1 )
  {
    d->mSourceDestDatumTransforms.remove( qMakePair( sourceCrs.authid(), destinationCrs.authid() ) );
  }
  else
  {
    d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), qMakePair( sourceTransform, destinationTransform ) );
  }
  d->mLock.unlock();
  return true;
}

QPair<int, int> QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{
  QString srcKey = source.authid();
  QString destKey = destination.authid();

  d->mLock.lockForRead();
  // highest priority is exact match for source/dest pair
  QPair< int, int > res = d->mSourceDestDatumTransforms.value( qMakePair( srcKey, destKey ), qMakePair( -1, -1 ) );
  if ( res.first != -1 && res.second != -1 )
  {
    d->mLock.unlock();
    return res;
  }

  // fallback to checking src and dest separately
  int srcTransform = d->mSourceDatumTransforms.value( srcKey, -1 );
  int destTransform = d->mDestDatumTransforms.value( destKey, -1 );
  d->mLock.unlock();
  return qMakePair( srcTransform, destTransform );
}

void QgsCoordinateTransformContext::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  d.detach();
  d->mLock.lockForWrite();

  d->mSourceDestDatumTransforms.clear();
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();

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

  element.appendChild( contextElem );
  d->mLock.unlock();
}
