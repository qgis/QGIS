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
#include "qgstranslationcontext.h"

#include <QRegularExpression>

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
  layer->setCustomProperty( u"metadata/identifier"_s, mIdentifier );
  layer->setCustomProperty( u"metadata/parentIdentifier"_s, mParentIdentifier );
  layer->setCustomProperty( u"metadata/language"_s, mLanguage );
  layer->setCustomProperty( u"metadata/type"_s, mType );
  layer->setCustomProperty( u"metadata/title"_s, mTitle );
  layer->setCustomProperty( u"metadata/extent"_s, QVariant::fromValue( mExtent ) );
  layer->setCustomProperty( u"metadata/abstract"_s, mAbstract );
  layer->setCustomProperty( u"metadata/fees"_s, mFees );
  layer->setCustomProperty( u"metadata/rights"_s, mRights );
  layer->setCustomProperty( u"metadata/licenses"_s, mLicenses );
  layer->setCustomProperty( u"metadata/history"_s, mHistory );
  layer->setCustomProperty( u"metadata/encoding"_s, mEncoding );
  layer->setCustomProperty( u"metadata/crs"_s, mCrs.authid() );
  layer->setCustomProperty( u"metadata/constraints"_s, QVariant::fromValue( mConstraints ) );
  layer->setCustomProperty( u"metadata/keywords"_s, QVariant::fromValue( mKeywords ) );
  layer->setCustomProperty( u"metadata/contacts"_s, QVariant::fromValue( mContacts ) );
  layer->setCustomProperty( u"metadata/links"_s, QVariant::fromValue( mLinks ) );
}

void QgsLayerMetadata::readFromLayer( const QgsMapLayer *layer )
{
  mIdentifier = layer->customProperty( u"metadata/identifier"_s ).toString();
  mParentIdentifier = layer->customProperty( u"metadata/parentIdentifier"_s ).toString();
  mLanguage = layer->customProperty( u"metadata/language"_s ).toString();
  mType = layer->customProperty( u"metadata/type"_s ).toString();
  mTitle = layer->customProperty( u"metadata/title"_s ).toString();
  mAbstract = layer->customProperty( u"metadata/abstract"_s ).toString();
  mFees = layer->customProperty( u"metadata/fees"_s ).toString();
  mRights = layer->customProperty( u"metadata/rights"_s ).toStringList();
  mLicenses = layer->customProperty( u"metadata/licenses"_s ).toStringList();
  mHistory = layer->customProperty( u"metadata/history"_s ).toStringList();
  mEncoding = layer->customProperty( u"metadata/encoding"_s ).toString();
  const QString crsAuthId = layer->customProperty( u"metadata/crs"_s ).toString();
  mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsAuthId );
  mExtent = layer->customProperty( u"metadata/extent"_s ).value<Extent>();
  mConstraints = layer->customProperty( u"metadata/constraints"_s ).value<ConstraintList>();
  mKeywords = layer->customProperty( u"metadata/keywords"_s ).value<QgsAbstractMetadataBase::KeywordMap>();
  mContacts = layer->customProperty( u"metadata/contacts"_s ).value<QgsAbstractMetadataBase::ContactList>();
  mLinks = layer->customProperty( u"metadata/links"_s ).value<QgsAbstractMetadataBase::LinkList>();
}

bool QgsLayerMetadata::readMetadataXml( const QDomElement &metadataElement, const QgsReadWriteContext &context )
{
  QgsAbstractMetadataBase::readMetadataXml( metadataElement, context );

  QDomNode mnl;
  QDomElement mne;

  // set fees
  mnl = metadataElement.namedItem( u"fees"_s );
  mFees = mnl.toElement().text();

  // constraints
  const QDomNodeList constraintsList = metadataElement.elementsByTagName( u"constraints"_s );
  mConstraints.clear();
  for ( int i = 0; i < constraintsList.size(); i++ )
  {
    mnl = constraintsList.at( i );
    mne = mnl.toElement();
    addConstraint( QgsLayerMetadata::Constraint( mne.text(), mne.attribute( u"type"_s ) ) );
  }

  // rights
  const QDomNodeList rightsNodeList = metadataElement.elementsByTagName( u"rights"_s );
  QStringList rightsList;
  for ( int i = 0; i < rightsNodeList.size(); i++ )
  {
    mnl = rightsNodeList.at( i );
    mne = mnl.toElement();
    const QString right = context.projectTranslator()->translate( "metadata", mne.text() );
    rightsList.append( right );
  }
  setRights( rightsList );

  // licenses
  const QDomNodeList licensesNodeList = metadataElement.elementsByTagName( u"license"_s );
  QStringList licensesList;
  for ( int i = 0; i < licensesNodeList.size(); i++ )
  {
    mnl = licensesNodeList.at( i );
    mne = mnl.toElement();
    licensesList.append( mne.text() );
  }
  setLicenses( licensesList );

  // encoding
  mnl = metadataElement.namedItem( u"encoding"_s );
  mEncoding = mnl.toElement().text();

  // crs
  mnl = metadataElement.namedItem( u"crs"_s );
  if ( !mCrs.readXml( mnl ) )
    mCrs = QgsCoordinateReferenceSystem();

  // extent
  mnl = metadataElement.namedItem( u"extent"_s );
  QgsLayerMetadata::Extent metadataExtent;

  // spatial extent
  const QDomNodeList spatialList = mnl.toElement().elementsByTagName( u"spatial"_s );
  QList< QgsLayerMetadata::SpatialExtent > metadataSpatialExtents;
  for ( int i = 0; i < spatialList.size(); i++ )
  {
    mnl = spatialList.at( i );
    mne = mnl.toElement();
    QgsLayerMetadata::SpatialExtent se = QgsLayerMetadata::SpatialExtent();
    se.extentCrs = QgsCoordinateReferenceSystem( mne.attribute( u"crs"_s ) );
    se.bounds = QgsBox3D();
    se.bounds.setXMinimum( mne.attribute( u"minx"_s ).toDouble() );
    se.bounds.setYMinimum( mne.attribute( u"miny"_s ).toDouble() );
    se.bounds.setZMinimum( mne.attribute( u"minz"_s ).toDouble() );
    se.bounds.setXMaximum( mne.attribute( u"maxx"_s ).toDouble() );
    se.bounds.setYMaximum( mne.attribute( u"maxy"_s ).toDouble() );
    se.bounds.setZMaximum( mne.attribute( u"maxz"_s ).toDouble() );
    metadataSpatialExtents.append( se );
  }
  metadataExtent.setSpatialExtents( metadataSpatialExtents );

  // temporal extent
  mnl = metadataElement.namedItem( u"extent"_s );
  const QDomNodeList temporalList = mnl.toElement().elementsByTagName( u"temporal"_s );
  QList<QgsDateTimeRange> metadataDates;
  for ( int j = 0; j < temporalList.size(); j++ )
  {
    mnl = temporalList.at( j );
    const QDomNodeList instantList = mnl.toElement().elementsByTagName( u"instant"_s );
    for ( int i = 0; i < instantList.size(); i++ )
    {
      mnl = instantList.at( i );
      const QDateTime d = QDateTime::fromString( mnl.toElement().text(), Qt::ISODate );
      const QgsDateTimeRange date = QgsDateTimeRange( d, d );
      metadataDates << date;
    }
    const QDomNodeList periodList = mnl.toElement().elementsByTagName( u"period"_s );
    for ( int i = 0; i < periodList.size(); i++ )
    {
      const QDomNode begin = periodList.at( i ).namedItem( u"start"_s );
      const QDomNode end = periodList.at( i ).namedItem( u"end"_s );
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

bool QgsLayerMetadata::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QgsAbstractMetadataBase::writeMetadataXml( metadataElement, document, context );

  // fees
  QDomElement fees = document.createElement( u"fees"_s );
  const QDomText feesText = document.createTextNode( mFees );
  fees.appendChild( feesText );
  metadataElement.appendChild( fees );

  // constraints
  for ( const QgsLayerMetadata::Constraint &constraint : mConstraints )
  {
    QDomElement constraintElement = document.createElement( u"constraints"_s );
    constraintElement.setAttribute( u"type"_s, constraint.type );
    const QDomText constraintText = document.createTextNode( constraint.constraint );
    constraintElement.appendChild( constraintText );
    metadataElement.appendChild( constraintElement );
  }

  // rights
  for ( const QString &right : mRights )
  {
    QDomElement rightElement = document.createElement( u"rights"_s );
    const QDomText rightText = document.createTextNode( right );
    rightElement.appendChild( rightText );
    metadataElement.appendChild( rightElement );
  }

  // license
  for ( const QString &license : mLicenses )
  {
    QDomElement licenseElement = document.createElement( u"license"_s );
    const QDomText licenseText = document.createTextNode( license );
    licenseElement.appendChild( licenseText );
    metadataElement.appendChild( licenseElement );
  }

  // encoding
  QDomElement encoding = document.createElement( u"encoding"_s );
  const QDomText encodingText = document.createTextNode( mEncoding );
  encoding.appendChild( encodingText );
  metadataElement.appendChild( encoding );

  // crs
  QDomElement crsElement = document.createElement( u"crs"_s );
  mCrs.writeXml( crsElement, document );
  metadataElement.appendChild( crsElement );

  // extent
  QDomElement extentElement = document.createElement( u"extent"_s );

  // spatial extents
  const QList< QgsLayerMetadata::SpatialExtent > sExtents = extent().spatialExtents();
  for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : sExtents )
  {
    QDomElement spatialElement = document.createElement( u"spatial"_s );
    // Dimensions fixed in the XSD
    spatialElement.setAttribute( u"dimensions"_s, u"2"_s );
    spatialElement.setAttribute( u"crs"_s, spatialExtent.extentCrs.authid() );
    spatialElement.setAttribute( u"minx"_s, qgsDoubleToString( spatialExtent.bounds.xMinimum() ) );
    spatialElement.setAttribute( u"miny"_s, qgsDoubleToString( spatialExtent.bounds.yMinimum() ) );
    spatialElement.setAttribute( u"minz"_s, qgsDoubleToString( spatialExtent.bounds.zMinimum() ) );
    spatialElement.setAttribute( u"maxx"_s, qgsDoubleToString( spatialExtent.bounds.xMaximum() ) );
    spatialElement.setAttribute( u"maxy"_s, qgsDoubleToString( spatialExtent.bounds.yMaximum() ) );
    spatialElement.setAttribute( u"maxz"_s, qgsDoubleToString( spatialExtent.bounds.zMaximum() ) );
    extentElement.appendChild( spatialElement );
  }

  // temporal extents
  const QList< QgsDateTimeRange > tExtents = extent().temporalExtents();
  for ( const QgsDateTimeRange &temporalExtent : tExtents )
  {
    QDomElement temporalElement = document.createElement( u"temporal"_s );
    if ( temporalExtent.isInstant() )
    {
      QDomElement instantElement = document.createElement( u"instant"_s );
      const QDomText instantText = document.createTextNode( temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      instantElement.appendChild( instantText );
      temporalElement.appendChild( instantElement );
    }
    else
    {
      QDomElement periodElement = document.createElement( u"period"_s );
      QDomElement startElement = document.createElement( u"start"_s );
      QDomElement endElement = document.createElement( u"end"_s );
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

void QgsLayerMetadata::registerTranslations( QgsTranslationContext *translationContext ) const
{
  QgsAbstractMetadataBase::registerTranslations( translationContext );

  for ( const QString &right : std::as_const( mRights ) )
  {
    translationContext->registerTranslation( u"metadata"_s, right );
  }
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

bool QgsLayerMetadata::contains( const QString &searchString ) const
{

  if ( searchString.trimmed().isEmpty() )
  {
    return false;
  }

  if ( title().contains( searchString, Qt::CaseInsensitive ) ||
       identifier().contains( searchString, Qt::CaseInsensitive ) ||
       abstract().contains( searchString, Qt::CaseInsensitive ) )
  {
    return true;
  }

  const QList<QStringList> keyVals { keywords().values() };
  for ( const QStringList &kws : std::as_const( keyVals ) )
  {
    for ( const QString &kw : std::as_const( kws ) )
    {
      if ( kw.contains( searchString, Qt::CaseSensitivity::CaseInsensitive ) )
      {
        return true;
      }
    }
  }

  const QStringList constCat { categories() };
  for ( const QString &cat : std::as_const( constCat ) )
  {
    if ( cat.contains( searchString, Qt::CaseSensitivity::CaseInsensitive ) )
    {
      return true;
    }
  }

  return false;
}

bool QgsLayerMetadata::matches( const QVector<QRegularExpression> &searchReList ) const
{
  for ( const QRegularExpression &re : std::as_const( searchReList ) )
  {
    if ( re.match( title() ).hasMatch() ||
         re.match( identifier() ).hasMatch() ||
         re.match( abstract() ).hasMatch() )
    {
      return true;
    }

    const QList<QStringList> keyVals { keywords().values() };
    for ( const QStringList &kws : std::as_const( keyVals ) )
    {
      for ( const QString &kw : std::as_const( kws ) )
      {
        if ( re.match( kw ).hasMatch() )
        {
          return true;
        }
      }
    }

    const QStringList constCat { categories() };
    for ( const QString &cat : std::as_const( constCat ) )
    {
      if ( re.match( cat ).hasMatch() )
      {
        return true;
      }
    }

  }

  return false;
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
