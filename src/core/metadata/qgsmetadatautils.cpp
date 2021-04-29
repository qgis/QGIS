/***************************************************************************
                             qgsmetadatautils.cpp
                             -------------------
    begin                : April 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsmetadatautils.h"
#include "qgslayermetadata.h"

#include <QDomDocument>
#include <QTextDocumentFragment>

QgsLayerMetadata QgsMetadataUtils::convertFromEsri( const QDomDocument &document )
{
  QgsLayerMetadata metadata;
  const QDomElement metadataElem = document.firstChildElement( QStringLiteral( "metadata" ) );

  const QDomElement esri = metadataElem.firstChildElement( QStringLiteral( "Esri" ) );
  const QDomElement dataProperties = esri.firstChildElement( QStringLiteral( "DataProperties" ) );
  const QDomElement itemProps = dataProperties.firstChildElement( QStringLiteral( "itemProps" ) );
  metadata.setIdentifier( itemProps.firstChildElement( QStringLiteral( "itemName" ) ).text() );

  const QDomElement dataIdInfo = metadataElem.firstChildElement( QStringLiteral( "dataIdInfo" ) );

  // title
  const QDomElement idCitation = dataIdInfo.firstChildElement( QStringLiteral( "idCitation" ) );
  const QString title = idCitation.firstChildElement( QStringLiteral( "resTitle" ) ).text();
  metadata.setTitle( title );

  // abstract
  const QDomElement idAbs = dataIdInfo.firstChildElement( QStringLiteral( "idAbs" ) );
  const QString abstractPlainText = QTextDocumentFragment::fromHtml( idAbs.text() ).toPlainText();
  metadata.setAbstract( abstractPlainText );

  // purpose
  const QDomElement idPurp = dataIdInfo.firstChildElement( QStringLiteral( "idPurp" ) );
  const QString purposePlainText = QTextDocumentFragment::fromHtml( idPurp.text() ).toPlainText();
  if ( !metadata.abstract().isEmpty() )
    metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + purposePlainText );
  else
    metadata.setAbstract( purposePlainText );

  // supplementary info
  const QDomElement suppInfo = dataIdInfo.firstChildElement( QStringLiteral( "suppInfo" ) );
  const QString suppInfoPlainText = QTextDocumentFragment::fromHtml( suppInfo.text() ).toPlainText();
  if ( !metadata.abstract().isEmpty() )
    metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + suppInfoPlainText );
  else
    metadata.setAbstract( suppInfoPlainText );

  // language
  const QDomElement dataLang = dataIdInfo.firstChildElement( QStringLiteral( "dataLang" ) );
  const QDomElement languageCode = dataLang.firstChildElement( QStringLiteral( "languageCode" ) );
  const QString language = languageCode.attribute( QStringLiteral( "value" ) ).toUpper();
  metadata.setLanguage( language );

  // keywords
  QDomElement searchKeys = dataIdInfo.firstChildElement( QStringLiteral( "searchKeys" ) );
  QStringList keywords;
  while ( !searchKeys.isNull() )
  {
    QDomElement keyword = searchKeys.firstChildElement( QStringLiteral( "keyword" ) );
    while ( !keyword.isNull() )
    {
      keywords << keyword.text();
      keyword = keyword.nextSiblingElement( QStringLiteral( "keyword" ) );
    }

    searchKeys = searchKeys.nextSiblingElement( QStringLiteral( "searchKeys" ) );
  }
  if ( !keywords.empty() )
    metadata.addKeywords( QObject::tr( "Search keys" ), keywords );

  // categories
  QDomElement themeKeys = dataIdInfo.firstChildElement( QStringLiteral( "themeKeys" ) );
  QStringList categories;
  while ( !themeKeys.isNull() )
  {
    QDomElement themeKeyword = themeKeys.firstChildElement( QStringLiteral( "keyword" ) );
    while ( !themeKeyword.isNull() )
    {
      categories << themeKeyword.text();
      themeKeyword = themeKeyword.nextSiblingElement( QStringLiteral( "keyword" ) );
    }
    themeKeys = themeKeys.nextSiblingElement( QStringLiteral( "themeKeys" ) );
  }
  if ( !categories.isEmpty() )
    metadata.setCategories( categories );

  QgsLayerMetadata::Extent extent;

  // pubDate
  const QDomElement date = idCitation.firstChildElement( QStringLiteral( "date" ) );
  const QString pubDate = date.firstChildElement( QStringLiteral( "pubDate" ) ).text();
  const QDateTime publicationDate = QDateTime::fromString( pubDate, Qt::ISODate );
  if ( publicationDate.isValid() )
  {
    extent.setTemporalExtents( { publicationDate, QDateTime() } );
  }

  //crs
  QgsCoordinateReferenceSystem crs;
  const QDomElement refSysInfo = metadataElem.firstChildElement( QStringLiteral( "refSysInfo" ) );
  if ( !refSysInfo.isNull() )
  {
    const QDomElement refSystem = refSysInfo.firstChildElement( QStringLiteral( "RefSystem" ) );
    const QDomElement refSysID = refSystem.firstChildElement( QStringLiteral( "refSysID" ) );
    const QString code = refSysID.firstChildElement( QStringLiteral( "identCode" ) ).attribute( QStringLiteral( "code" ) );
    const QString auth = refSysID.firstChildElement( QStringLiteral( "idCodeSpace" ) ).text();

    crs = QgsCoordinateReferenceSystem( QStringLiteral( "%1:%2" ).arg( auth, code ) );
    if ( crs.isValid() )
      metadata.setCrs( crs );
  }

  // extent
  QDomElement dataExt = dataIdInfo.firstChildElement( QStringLiteral( "dataExt" ) );
  while ( !dataExt.isNull() )
  {
    const QDomElement geoEle = dataExt.firstChildElement( QStringLiteral( "geoEle" ) );
    if ( !geoEle.isNull() )
    {
      const QDomElement geoBndBox = geoEle.firstChildElement( QStringLiteral( "GeoBndBox" ) );
      const double west = geoBndBox.firstChildElement( QStringLiteral( "westBL" ) ).text().toDouble();
      const double east = geoBndBox.firstChildElement( QStringLiteral( "eastBL" ) ).text().toDouble();
      const double south = geoBndBox.firstChildElement( QStringLiteral( "northBL" ) ).text().toDouble();
      const double north = geoBndBox.firstChildElement( QStringLiteral( "southBL" ) ).text().toDouble();

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.extentCrs = crs.isValid() ? crs : QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      spatialExtent.bounds = QgsBox3d( west, south, 0, east, north, 0 );

      extent.setSpatialExtents( { spatialExtent } );
      break;
    }
    dataExt = dataExt.nextSiblingElement( QStringLiteral( "dataExt" ) );
  }

  metadata.setExtent( extent );

  // licenses, constraints
  QStringList licenses;
  QStringList rights;
  QgsLayerMetadata::ConstraintList constraints;
  QDomElement resConst = dataIdInfo.firstChildElement( QStringLiteral( "resConst" ) );
  while ( !resConst.isNull() )
  {
    QDomElement legConsts = resConst.firstChildElement( QStringLiteral( "LegConsts" ) );
    while ( !legConsts.isNull() )
    {
      const QString restrictCd = legConsts.firstChildElement( QStringLiteral( "useConsts" ) ).firstChildElement( QStringLiteral( "RestrictCd" ) ).attribute( QStringLiteral( "value" ) );

      if ( restrictCd.compare( QLatin1String( "005" ) ) == 0 )
      {
        licenses << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      }
      else if ( restrictCd.compare( QLatin1String( "006" ) ) == 0 )
      {
        rights << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      }
      legConsts = legConsts.nextSiblingElement( QStringLiteral( "LegConsts" ) );
    }

    QDomElement secConsts = resConst.firstChildElement( QStringLiteral( "SecConsts" ) );
    while ( !secConsts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Security constraints" );
      constraint.constraint = QTextDocumentFragment::fromHtml( secConsts.firstChildElement( QStringLiteral( "userNote" ) ).text() ).toPlainText();
      constraints << constraint;
      secConsts = secConsts.nextSiblingElement( QStringLiteral( "SecConsts" ) );
    }

    QDomElement consts = resConst.firstChildElement( QStringLiteral( "Consts" ) );
    while ( !consts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Limitations of use" );
      constraint.constraint = QTextDocumentFragment::fromHtml( consts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      constraints << constraint;
      consts = consts.nextSiblingElement( QStringLiteral( "Consts" ) );
    }

    resConst = resConst.nextSiblingElement( QStringLiteral( "resConst" ) );
  }

  const QDomElement idCredit = dataIdInfo.firstChildElement( QStringLiteral( "idCredit" ) );
  const QString credit = idCredit.text();
  if ( !credit.isEmpty() )
    rights << credit;

  metadata.setLicenses( licenses );
  metadata.setRights( rights );
  metadata.setConstraints( constraints );

  // links
  const QDomElement distInfo = metadataElem.firstChildElement( QStringLiteral( "distInfo" ) );
  const QDomElement distributor = distInfo.firstChildElement( QStringLiteral( "distributor" ) );

  QDomElement distorTran = distributor.firstChildElement( QStringLiteral( "distorTran" ) );
  while ( !distorTran.isNull() )
  {
    const QDomElement onLineSrc = distorTran.firstChildElement( QStringLiteral( "onLineSrc" ) );
    if ( !onLineSrc.isNull() )
    {
      QgsAbstractMetadataBase::Link link;
      link.url = onLineSrc.firstChildElement( QStringLiteral( "linkage" ) ).text();

      const QDomElement distorFormat = distributor.firstChildElement( QStringLiteral( "distorFormat" ) );
      link.name = distorFormat.firstChildElement( QStringLiteral( "formatName" ) ).text();
      link.type = distorFormat.firstChildElement( QStringLiteral( "formatSpec" ) ).text();
      metadata.addLink( link );
    }

    distorTran = distorTran.nextSiblingElement( QStringLiteral( "distorTran" ) );
  }

  // lineage
  const QDomElement dqInfo = metadataElem.firstChildElement( QStringLiteral( "dqInfo" ) );
  const QDomElement dataLineage = dqInfo.firstChildElement( QStringLiteral( "dataLineage" ) );
  const QString statement = QTextDocumentFragment::fromHtml( dataLineage.firstChildElement( QStringLiteral( "statement" ) ).text() ).toPlainText();
  if ( !statement.isEmpty() )
    metadata.addHistoryItem( statement );

  QDomElement dataSource = dataLineage.firstChildElement( QStringLiteral( "dataSource" ) );
  while ( !dataSource.isNull() )
  {
    metadata.addHistoryItem( QObject::tr( "Data source: %1" ).arg( QTextDocumentFragment::fromHtml( dataSource.firstChildElement( QStringLiteral( "srcDesc" ) ).text() ).toPlainText() ) );
    dataSource = dataSource.nextSiblingElement( QStringLiteral( "dataSource" ) );
  }

  // contacts
  const QDomElement mdContact = metadataElem.firstChildElement( QStringLiteral( "mdContact" ) );
  if ( !mdContact.isNull() )
  {
    QgsAbstractMetadataBase::Contact contact;
    contact.name = mdContact.firstChildElement( QStringLiteral( "rpIndName" ) ).text();
    contact.organization = mdContact.firstChildElement( QStringLiteral( "rpOrgName" ) ).text();
    contact.position = mdContact.firstChildElement( QStringLiteral( "rpPosName" ) ).text();

    const QString role = mdContact.firstChildElement( QStringLiteral( "role" ) ).firstChildElement( QStringLiteral( "RoleCd" ) ).attribute( QStringLiteral( "value" ) );
    if ( role == QLatin1String( "007" ) )
      contact.role = QObject::tr( "Point of contact" );

    const QDomElement rpCntInfo = mdContact.firstChildElement( QStringLiteral( "rpCntInfo" ) );
    contact.email = rpCntInfo.firstChildElement( QStringLiteral( "cntAddress" ) ).firstChildElement( QStringLiteral( "eMailAdd" ) ).text();
    contact.voice = rpCntInfo.firstChildElement( QStringLiteral( "cntPhone" ) ).firstChildElement( QStringLiteral( "voiceNum" ) ).text();

    metadata.addContact( contact );
  }

  return metadata;
}
