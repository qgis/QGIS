/***************************************************************************
                             qgslayermetadata.cpp
                             --------------------
    begin                : April 2017
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

#include "qgslayermetadata.h"
#include "qgsmaplayer.h"

QgsLayerMetadata *QgsLayerMetadata::clone() const
{
  return new QgsLayerMetadata( *this );
}

QString QgsLayerMetadata::fees() const
{
  return mFees;
}

void QgsLayerMetadata::setFees( const QString &fees )
{
  mFees = fees;
}

void QgsLayerMetadata::addConstraint( const QgsLayerMetadata::Constraint &constraint )
{
  mConstraints << constraint;
}

QList<QgsLayerMetadata::Constraint> QgsLayerMetadata::constraints() const
{
  return mConstraints;
}

void QgsLayerMetadata::setConstraints( const QList<Constraint> &constraints )
{
  mConstraints = constraints;
}

QStringList QgsLayerMetadata::rights() const
{
  return mRights;
}

void QgsLayerMetadata::setRights( const QStringList &rights )
{
  mRights = rights;
}

QStringList QgsLayerMetadata::licenses() const
{
  return mLicenses;
}

void QgsLayerMetadata::setLicenses( const QStringList &licenses )
{
  mLicenses = licenses;
}

QString QgsLayerMetadata::encoding() const
{
  return mEncoding;
}

void QgsLayerMetadata::setEncoding( const QString &encoding )
{
  mEncoding = encoding;
}

QgsCoordinateReferenceSystem QgsLayerMetadata::crs() const
{
  return mCrs;
}

void QgsLayerMetadata::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

void QgsLayerMetadata::saveToLayer( QgsMapLayer *layer ) const
{
  layer->setCustomProperty( QStringLiteral( "metadata/identifier" ), mIdentifier );
  layer->setCustomProperty( QStringLiteral( "metadata/parentIdentifier" ), mParentIdentifier );
  layer->setCustomProperty( QStringLiteral( "metadata/language" ), mLanguage );
  layer->setCustomProperty( QStringLiteral( "metadata/type" ), mType );
  layer->setCustomProperty( QStringLiteral( "metadata/title" ), mTitle );
  layer->setCustomProperty( QStringLiteral( "metadata/extent" ), QVariant::fromValue( mExtent ) );
  layer->setCustomProperty( QStringLiteral( "metadata/abstract" ), mAbstract );
  layer->setCustomProperty( QStringLiteral( "metadata/fees" ), mFees );
  layer->setCustomProperty( QStringLiteral( "metadata/rights" ), mRights );
  layer->setCustomProperty( QStringLiteral( "metadata/licenses" ), mLicenses );
  layer->setCustomProperty( QStringLiteral( "metadata/history" ), mHistory );
  layer->setCustomProperty( QStringLiteral( "metadata/encoding" ), mEncoding );
  layer->setCustomProperty( QStringLiteral( "metadata/crs" ), mCrs.authid() );
  layer->setCustomProperty( QStringLiteral( "metadata/constraints" ), QVariant::fromValue( mConstraints ) );
  layer->setCustomProperty( QStringLiteral( "metadata/keywords" ), QVariant::fromValue( mKeywords ) );
  layer->setCustomProperty( QStringLiteral( "metadata/contacts" ), QVariant::fromValue( mContacts ) );
  layer->setCustomProperty( QStringLiteral( "metadata/links" ), QVariant::fromValue( mLinks ) );
}

void QgsLayerMetadata::readFromLayer( const QgsMapLayer *layer )
{
  mIdentifier = layer->customProperty( QStringLiteral( "metadata/identifier" ) ).toString();
  mParentIdentifier = layer->customProperty( QStringLiteral( "metadata/parentIdentifier" ) ).toString();
  mLanguage = layer->customProperty( QStringLiteral( "metadata/language" ) ).toString();
  mType = layer->customProperty( QStringLiteral( "metadata/type" ) ).toString();
  mTitle = layer->customProperty( QStringLiteral( "metadata/title" ) ).toString();
  mAbstract = layer->customProperty( QStringLiteral( "metadata/abstract" ) ).toString();
  mFees = layer->customProperty( QStringLiteral( "metadata/fees" ) ).toString();
  mRights = layer->customProperty( QStringLiteral( "metadata/rights" ) ).toStringList();
  mLicenses = layer->customProperty( QStringLiteral( "metadata/licenses" ) ).toStringList();
  mHistory = layer->customProperty( QStringLiteral( "metadata/history" ) ).toStringList();
  mEncoding = layer->customProperty( QStringLiteral( "metadata/encoding" ) ).toString();
  const QString crsAuthId = layer->customProperty( QStringLiteral( "metadata/crs" ) ).toString();
  mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsAuthId );
  mExtent = layer->customProperty( QStringLiteral( "metadata/extent" ) ).value<Extent>();
  mConstraints = layer->customProperty( QStringLiteral( "metadata/constraints" ) ).value<ConstraintList>();
  mKeywords = layer->customProperty( QStringLiteral( "metadata/keywords" ) ).value<QgsAbstractMetadataBase::KeywordMap>();
  mContacts = layer->customProperty( QStringLiteral( "metadata/contacts" ) ).value<QgsAbstractMetadataBase::ContactList>();
  mLinks = layer->customProperty( QStringLiteral( "metadata/links" ) ).value<QgsAbstractMetadataBase::LinkList>();
}

bool QgsLayerMetadata::readMetadataXml( const QDomElement &metadataElement )
{
  QgsAbstractMetadataBase::readMetadataXml( metadataElement );

  QDomNode mnl;
  QDomElement mne;

  // set fees
  mnl = metadataElement.namedItem( QStringLiteral( "fees" ) );
  mFees = mnl.toElement().text();

  // constraints
  const QDomNodeList constraintsList = metadataElement.elementsByTagName( QStringLiteral( "constraints" ) );
  mConstraints.clear();
  for ( int i = 0; i < constraintsList.size(); i++ )
  {
    mnl = constraintsList.at( i );
    mne = mnl.toElement();
    addConstraint( QgsLayerMetadata::Constraint( mne.text(), mne.attribute( QStringLiteral( "type" ) ) ) );
  }

  // rights
  const QDomNodeList rightsNodeList = metadataElement.elementsByTagName( QStringLiteral( "rights" ) );
  QStringList rightsList;
  for ( int i = 0; i < rightsNodeList.size(); i++ )
  {
    mnl = rightsNodeList.at( i );
    mne = mnl.toElement();
    rightsList.append( mne.text() );
  }
  setRights( rightsList );

  // licenses
  const QDomNodeList licensesNodeList = metadataElement.elementsByTagName( QStringLiteral( "license" ) );
  QStringList licensesList;
  for ( int i = 0; i < licensesNodeList.size(); i++ )
  {
    mnl = licensesNodeList.at( i );
    mne = mnl.toElement();
    licensesList.append( mne.text() );
  }
  setLicenses( licensesList );

  // encoding
  mnl = metadataElement.namedItem( QStringLiteral( "encoding" ) );
  mEncoding = mnl.toElement().text();

  // crs
  mnl = metadataElement.namedItem( QStringLiteral( "crs" ) );
  if ( !mCrs.readXml( mnl ) )
    mCrs = QgsCoordinateReferenceSystem();

  // extent
  mnl = metadataElement.namedItem( QStringLiteral( "extent" ) );
  QgsLayerMetadata::Extent metadataExtent;

  // spatial extent
  const QDomNodeList spatialList = mnl.toElement().elementsByTagName( QStringLiteral( "spatial" ) );
  QList< QgsLayerMetadata::SpatialExtent > metadataSpatialExtents;
  for ( int i = 0; i < spatialList.size(); i++ )
  {
    mnl = spatialList.at( i );
    mne = mnl.toElement();
    QgsLayerMetadata::SpatialExtent se = QgsLayerMetadata::SpatialExtent();
    se.extentCrs = QgsCoordinateReferenceSystem( mne.attribute( QStringLiteral( "crs" ) ) );
    se.bounds = QgsBox3d();
    se.bounds.setXMinimum( mne.attribute( QStringLiteral( "minx" ) ).toDouble() );
    se.bounds.setYMinimum( mne.attribute( QStringLiteral( "miny" ) ).toDouble() );
    se.bounds.setZMinimum( mne.attribute( QStringLiteral( "minz" ) ).toDouble() );
    se.bounds.setXMaximum( mne.attribute( QStringLiteral( "maxx" ) ).toDouble() );
    se.bounds.setYMaximum( mne.attribute( QStringLiteral( "maxy" ) ).toDouble() );
    se.bounds.setZMaximum( mne.attribute( QStringLiteral( "maxz" ) ).toDouble() );
    metadataSpatialExtents.append( se );
  }
  metadataExtent.setSpatialExtents( metadataSpatialExtents );

  // temporal extent
  mnl = metadataElement.namedItem( QStringLiteral( "extent" ) );
  const QDomNodeList temporalList = mnl.toElement().elementsByTagName( QStringLiteral( "temporal" ) );
  QList<QgsDateTimeRange> metadataDates;
  for ( int j = 0; j < temporalList.size(); j++ )
  {
    mnl = temporalList.at( j );
    const QDomNodeList instantList = mnl.toElement().elementsByTagName( QStringLiteral( "instant" ) );
    for ( int i = 0; i < instantList.size(); i++ )
    {
      mnl = instantList.at( i );
      const QDateTime d = QDateTime::fromString( mnl.toElement().text(), Qt::ISODate );
      const QgsDateTimeRange date = QgsDateTimeRange( d, d );
      metadataDates << date;
    }
    const QDomNodeList periodList = mnl.toElement().elementsByTagName( QStringLiteral( "period" ) );
    for ( int i = 0; i < periodList.size(); i++ )
    {
      const QDomNode begin = periodList.at( i ).namedItem( QStringLiteral( "start" ) );
      const QDomNode end = periodList.at( i ).namedItem( QStringLiteral( "end" ) );
      const QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
      const QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );
      const QgsDateTimeRange date = QgsDateTimeRange( beginDate, endDate );
      metadataDates << date;
    }
  }
  metadataExtent.setTemporalExtents( metadataDates );
  setExtent( metadataExtent );

  return true;
}

bool QgsLayerMetadata::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const
{
  QgsAbstractMetadataBase::writeMetadataXml( metadataElement, document );

  // fees
  QDomElement fees = document.createElement( QStringLiteral( "fees" ) );
  const QDomText feesText = document.createTextNode( mFees );
  fees.appendChild( feesText );
  metadataElement.appendChild( fees );

  // constraints
  for ( const QgsLayerMetadata::Constraint &constraint : mConstraints )
  {
    QDomElement constraintElement = document.createElement( QStringLiteral( "constraints" ) );
    constraintElement.setAttribute( QStringLiteral( "type" ), constraint.type );
    const QDomText constraintText = document.createTextNode( constraint.constraint );
    constraintElement.appendChild( constraintText );
    metadataElement.appendChild( constraintElement );
  }

  // rights
  for ( const QString &right : mRights )
  {
    QDomElement rightElement = document.createElement( QStringLiteral( "rights" ) );
    const QDomText rightText = document.createTextNode( right );
    rightElement.appendChild( rightText );
    metadataElement.appendChild( rightElement );
  }

  // license
  for ( const QString &license : mLicenses )
  {
    QDomElement licenseElement = document.createElement( QStringLiteral( "license" ) );
    const QDomText licenseText = document.createTextNode( license );
    licenseElement.appendChild( licenseText );
    metadataElement.appendChild( licenseElement );
  }

  // encoding
  QDomElement encoding = document.createElement( QStringLiteral( "encoding" ) );
  const QDomText encodingText = document.createTextNode( mEncoding );
  encoding.appendChild( encodingText );
  metadataElement.appendChild( encoding );

  // crs
  QDomElement crsElement = document.createElement( QStringLiteral( "crs" ) );
  mCrs.writeXml( crsElement, document );
  metadataElement.appendChild( crsElement );

  // extent
  QDomElement extentElement = document.createElement( QStringLiteral( "extent" ) );

  // spatial extents
  const QList< QgsLayerMetadata::SpatialExtent > sExtents = extent().spatialExtents();
  for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : sExtents )
  {
    QDomElement spatialElement = document.createElement( QStringLiteral( "spatial" ) );
    // Dimensions fixed in the XSD
    spatialElement.setAttribute( QStringLiteral( "dimensions" ), QStringLiteral( "2" ) );
    spatialElement.setAttribute( QStringLiteral( "crs" ), spatialExtent.extentCrs.authid() );
    spatialElement.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( spatialExtent.bounds.xMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( spatialExtent.bounds.yMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "minz" ), qgsDoubleToString( spatialExtent.bounds.zMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( spatialExtent.bounds.xMaximum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( spatialExtent.bounds.yMaximum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxz" ), qgsDoubleToString( spatialExtent.bounds.zMaximum() ) );
    extentElement.appendChild( spatialElement );
  }

  // temporal extents
  const QList< QgsDateTimeRange > tExtents = extent().temporalExtents();
  for ( const QgsDateTimeRange &temporalExtent : tExtents )
  {
    QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
    if ( temporalExtent.isInstant() )
    {
      QDomElement instantElement = document.createElement( QStringLiteral( "instant" ) );
      const QDomText instantText = document.createTextNode( temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      instantElement.appendChild( instantText );
      temporalElement.appendChild( instantElement );
    }
    else
    {
      QDomElement periodElement = document.createElement( QStringLiteral( "period" ) );
      QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
      QDomElement endElement = document.createElement( QStringLiteral( "end" ) );
      const QDomText startText = document.createTextNode( temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      const QDomText endText = document.createTextNode( temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      startElement.appendChild( startText );
      endElement.appendChild( endText );
      periodElement.appendChild( startElement );
      periodElement.appendChild( endElement );
      temporalElement.appendChild( periodElement );
    }
    extentElement.appendChild( temporalElement );
  }

  metadataElement.appendChild( extentElement );

  return true;
}

void QgsLayerMetadata::combine( const QgsAbstractMetadataBase *other )
{
  QgsAbstractMetadataBase::combine( other );

  if ( const QgsLayerMetadata *otherLayerMetadata = dynamic_cast< const QgsLayerMetadata * >( other ) )
  {
    if ( !otherLayerMetadata->fees().isEmpty() )
      mFees = otherLayerMetadata->fees();

    if ( !otherLayerMetadata->constraints().isEmpty() )
      mConstraints = otherLayerMetadata->constraints();

    if ( !otherLayerMetadata->rights().isEmpty() )
      mRights = otherLayerMetadata->rights();

    if ( !otherLayerMetadata->licenses().isEmpty() )
      mLicenses = otherLayerMetadata->licenses();

    if ( !otherLayerMetadata->encoding().isEmpty() )
      mEncoding = otherLayerMetadata->encoding();

    if ( otherLayerMetadata->crs().isValid() )
      mCrs = otherLayerMetadata->crs();

    if ( !otherLayerMetadata->extent().spatialExtents().isEmpty() )
      mExtent.setSpatialExtents( otherLayerMetadata->extent().spatialExtents() );

    if ( !otherLayerMetadata->extent().temporalExtents().isEmpty() )
      mExtent.setTemporalExtents( otherLayerMetadata->extent().temporalExtents() );
  }
}

const QgsLayerMetadata::Extent &QgsLayerMetadata::extent() const
{
  return mExtent;
}

QgsLayerMetadata::Extent &QgsLayerMetadata::extent()
{
  return mExtent;
}

void QgsLayerMetadata::setExtent( const Extent &extent )
{
  mExtent = extent;
}

QList<QgsLayerMetadata::SpatialExtent> QgsLayerMetadata::Extent::spatialExtents() const
{
  return mSpatialExtents;
}

void QgsLayerMetadata::Extent::setSpatialExtents( const QList<QgsLayerMetadata::SpatialExtent> &spatialExtents )
{
  mSpatialExtents = spatialExtents;
}

QList<QgsDateTimeRange> QgsLayerMetadata::Extent::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsLayerMetadata::Extent::setTemporalExtents( const QList<QgsDateTimeRange> &temporalExtents )
{
  mTemporalExtents = temporalExtents;
}

bool QgsLayerMetadata::Extent::operator==( const QgsLayerMetadata::Extent &other ) const
{
  return mSpatialExtents == other.mSpatialExtents && mTemporalExtents == other.mTemporalExtents;
}

bool QgsLayerMetadata::operator==( const QgsLayerMetadata &other )  const
{
  return equals( other ) &&
         mFees == other.mFees &&
         mConstraints == other.mConstraints &&
         mRights == other.mRights &&
         mLicenses == other.mLicenses &&
         mEncoding == other.mEncoding &&
         mCrs == other.mCrs &&
         mExtent == other.mExtent;
}

bool QgsLayerMetadata::SpatialExtent::operator==( const QgsLayerMetadata::SpatialExtent &other ) const
{
  return extentCrs == other.extentCrs &&
         bounds == other.bounds;
}

bool QgsLayerMetadata::Constraint::operator==( const QgsLayerMetadata::Constraint &other ) const
{
  return type == other.type && constraint == other.constraint;
}
