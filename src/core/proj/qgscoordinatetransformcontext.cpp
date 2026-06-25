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
#include "qgsprojutils.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsSettingsTreeNamedListNode *QgsCoordinateTransformContext::sTreeCoordinateOperationsSource = QgsSettingsTree::sTreeCrs->createChildNode( u"coordinate-operations"_s )->createNamedListNode( u"source"_s );
QgsSettingsTreeNamedListNode *QgsCoordinateTransformContext::sTreeCoordinateOperationsDestination = sTreeCoordinateOperationsSource->createNamedListNode( u"destination"_s );
const QgsSettingsEntryString *QgsCoordinateTransformContext::settingsCoordinateOperation
  = new QgsSettingsEntryString( u"operation"_s, sTreeCoordinateOperationsDestination, QString(), u"PROJ coordinate operation string used when transforming between the source and destination CRS pair."_s );
const QgsSettingsEntryBool *QgsCoordinateTransformContext::settingsAllowFallback
  = new QgsSettingsEntryBool( u"allow-fallback"_s, sTreeCoordinateOperationsDestination, true, u"If true, transformations between the source and destination CRS pair are allowed to fall back to a less accurate operation when the preferred coordinate operation fails."_s );

QString crsToKey( const QgsCoordinateReferenceSystem &crs )
{
  return crs.authid().isEmpty() ? crs.toWkt( Qgis::CrsWktVariant::Preferred ) : crs.authid();
}

QgsCoordinateTransformContext::QgsCoordinateTransformContext()
  : d( new QgsCoordinateTransformContextPrivate() )
{}

QgsCoordinateTransformContext::~QgsCoordinateTransformContext() = default;

QgsCoordinateTransformContext::QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs ) //NOLINT
  : d( rhs.d )
{}

QgsCoordinateTransformContext::QgsCoordinateTransformContext( QgsCoordinateTransformContext &&rhs ) //NOLINT
  : d( std::move( rhs.d ) )
{}


QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( const QgsCoordinateTransformContext &rhs ) //NOLINT
{
  if ( &rhs == this )
    return *this;

  d = rhs.d;
  return *this;
}

QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( QgsCoordinateTransformContext &&rhs ) //NOLINT
{
  if ( &rhs == this )
    return *this;

  d = std::move( rhs.d );
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

bool QgsCoordinateTransformContext::operator!=( const QgsCoordinateTransformContext &rhs ) const
{
  return !( *this == rhs );
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

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform(
  const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform
)
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;
  Q_UNUSED( sourceTransform )
  Q_UNUSED( destinationTransform )
  return false;
}

bool QgsCoordinateTransformContext::addCoordinateOperation(
  const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &coordinateOperationProjString, bool allowFallback
)
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

  auto it = d->mSourceDestDatumTransforms.constFind( qMakePair( source, destination ) );
  if ( it == d->mSourceDestDatumTransforms.constEnd() )
  {
    // try to reverse
    it = d->mSourceDestDatumTransforms.constFind( qMakePair( destination, source ) );
  }

  const QString result = it == d->mSourceDestDatumTransforms.constEnd() ? QString() : it.value().operation;
  d->mLock.unlock();
  return result;
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

  const QDomNodeList contextNodes = element.elementsByTagName( u"transformContext"_s );
  if ( contextNodes.count() < 1 )
  {
    d->mLock.unlock();
    return true;
  }

  missingTransforms.clear();
  bool result = true;

  const QDomElement contextElem = contextNodes.at( 0 ).toElement();

  // src/dest transforms
  const QDomNodeList srcDestNodes = contextElem.elementsByTagName( u"srcDest"_s );
  for ( int i = 0; i < srcDestNodes.size(); ++i )
  {
    const QDomElement transformElem = srcDestNodes.at( i ).toElement();

    const QDomElement srcElem = transformElem.firstChildElement( u"src"_s );
    const QDomElement destElem = transformElem.firstChildElement( u"dest"_s );

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
      const QString key1 = transformElem.attribute( u"source"_s );
      const QString key2 = transformElem.attribute( u"dest"_s );
      srcCrs = QgsCoordinateReferenceSystem( key1 );
      destCrs = QgsCoordinateReferenceSystem( key2 );
    }

    if ( !srcCrs.isValid() || !destCrs.isValid() )
      continue;

    const QString coordinateOp = transformElem.attribute( u"coordinateOp"_s );
    const bool allowFallback = transformElem.attribute( u"allowFallback"_s, u"1"_s ).toInt();

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

  QDomElement contextElem = doc.createElement( u"transformContext"_s );

  //src/dest transforms
  for ( auto it = d->mSourceDestDatumTransforms.constBegin(); it != d->mSourceDestDatumTransforms.constEnd(); ++it )
  {
    QDomElement transformElem = doc.createElement( u"srcDest"_s );
    QDomElement srcElem = doc.createElement( u"src"_s );
    QDomElement destElem = doc.createElement( u"dest"_s );

    it.key().first.writeXml( srcElem, doc );
    it.key().second.writeXml( destElem, doc );

    transformElem.appendChild( srcElem );
    transformElem.appendChild( destElem );

    transformElem.setAttribute( u"coordinateOp"_s, it.value().operation );
    transformElem.setAttribute( u"allowFallback"_s, it.value().allowFallback ? u"1"_s : u"0"_s );
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

  QMap<QPair<QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem>, QgsCoordinateTransformContextPrivate::OperationDetails> transforms;
  const QStringList srcAuthIds = sTreeCoordinateOperationsSource->items();
  for ( const QString &srcAuthId : srcAuthIds )
  {
    const QStringList destAuthIds = sTreeCoordinateOperationsDestination->items( { srcAuthId } );
    for ( const QString &destAuthId : destAuthIds )
    {
      QgsCoordinateTransformContextPrivate::OperationDetails deets;
      deets.operation = settingsCoordinateOperation->value( { srcAuthId, destAuthId } );
      deets.allowFallback = settingsAllowFallback->value( { srcAuthId, destAuthId } );
      transforms[qMakePair( QgsCoordinateReferenceSystem( srcAuthId ), QgsCoordinateReferenceSystem( destAuthId ) )] = deets;
    }
  }

  // add transforms to context
  auto transformIt = transforms.constBegin();
  for ( ; transformIt != transforms.constEnd(); ++transformIt )
  {
    d->mSourceDestDatumTransforms.insert( transformIt.key(), transformIt.value() );
  }

  d->mLock.unlock();
}

void QgsCoordinateTransformContext::writeSettings()
{
  sTreeCoordinateOperationsSource->deleteAllItems();

  for ( auto transformIt = d->mSourceDestDatumTransforms.constBegin(); transformIt != d->mSourceDestDatumTransforms.constEnd(); ++transformIt )
  {
    const QString srcAuthId = transformIt.key().first.authid();
    const QString destAuthId = transformIt.key().second.authid();

    if ( srcAuthId.isEmpty() || destAuthId.isEmpty() )
      continue; // not so nice, but alternative would be to shove whole CRS wkt into the settings values...

    settingsCoordinateOperation->setValue( transformIt.value().operation, { srcAuthId, destAuthId } );
    settingsAllowFallback->setValue( transformIt.value().allowFallback, { srcAuthId, destAuthId } );
  }
}
