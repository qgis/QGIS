/***************************************************************************
    qgslayermetadataformatter.cpp
    ---------------------
    begin                : September 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QStringBuilder>
#include <QDateTime>

#include "qgslayermetadataformatter.h"
#include "qgslayermetadata.h"


QgsLayerMetadataFormatter::QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata )
  : mMetadata( metadata )
{
}

QString QgsLayerMetadataFormatter::accessSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Fees" ) + QStringLiteral( "</td><td>" ) + mMetadata.fees() + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Licenses" ) + QStringLiteral( "</td><td>" ) + mMetadata.licenses().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Rights" ) + QStringLiteral( "</td><td>" ) + mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Constraints" ) + QStringLiteral( "</td><td>" );
  const QList<QgsLayerMetadata::Constraint> &constraints = mMetadata.constraints();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::Constraint &constraint : constraints )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />" );
    }
    myMetadata += QStringLiteral( "<strong>" ) + constraint.type + QStringLiteral( ": </strong>" ) + constraint.constraint;
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QLatin1String( "</table>\n" );
  return myMetadata;
}

QString QgsLayerMetadataFormatter::contactsSectionHtml() const
{
  const QList<QgsAbstractMetadataBase::Contact> &contacts = mMetadata.contacts();
  QString myMetadata;
  if ( contacts.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + tr( "No contact yet." ) + QStringLiteral( "</p>" );
  }
  else
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += QLatin1String( "<tr><th>" ) + tr( "ID" ) + QLatin1String( "</th><th>" ) + tr( "Name" ) + QLatin1String( "</th><th>" ) + tr( "Position" ) + QLatin1String( "</th><th>" ) + tr( "Organization" ) + QLatin1String( "</th><th>" ) + tr( "Role" ) + QLatin1String( "</th><th>" ) + tr( "Email" ) + QLatin1String( "</th><th>" ) + tr( "Voice" ) + QLatin1String( "</th><th>" ) + tr( "Fax" ) + QLatin1String( "</th><th>" ) + tr( "Addresses" ) + QLatin1String( "</th></tr>\n" );
    int i = 1;
    for ( const QgsAbstractMetadataBase::Contact &contact : contacts )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + QString::number( i ) + QLatin1String( "</td><td>" ) + contact.name + QLatin1String( "</td><td>" ) + contact.position + QLatin1String( "</td><td>" ) + contact.organization + QLatin1String( "</td><td>" ) + contact.role + QLatin1String( "</td><td>" ) + contact.email + QLatin1String( "</td><td>" ) + contact.voice + QLatin1String( "</td><td>" ) + contact.fax + QLatin1String( "</td><td>" );
      bool notFirstRow = false;
      for ( const QgsAbstractMetadataBase::Address &oneAddress : contact.addresses )
      {
        if ( notFirstRow )
        {
          myMetadata += QLatin1String( "<br />\n" );
        }
        if ( ! oneAddress.type.isEmpty() )
        {
          myMetadata += oneAddress.type + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.address.isEmpty() )
        {
          myMetadata += oneAddress.address + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.postalCode.isEmpty() )
        {
          myMetadata += oneAddress.postalCode + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.city.isEmpty() )
        {
          myMetadata += oneAddress.city + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.administrativeArea.isEmpty() )
        {
          myMetadata += oneAddress.administrativeArea + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.country.isEmpty() )
        {
          myMetadata += oneAddress.country;
        }
        notFirstRow = true;
      }
      myMetadata += QLatin1String( "</td></tr>\n" );
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::extentSectionHtml( const bool showSpatialExtent ) const
{
  const QgsLayerMetadata::Extent extent = mMetadata.extent();
  bool notFirstRow = false;
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  if ( showSpatialExtent )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "CRS" ) + QStringLiteral( "</td><td>" );
    if ( mMetadata.crs().isValid() )
    {
      myMetadata += mMetadata.crs().authid() + QStringLiteral( " - " );
      myMetadata += mMetadata.crs().description() + QStringLiteral( " - " );
      if ( mMetadata.crs().isGeographic() )
        myMetadata += tr( "Geographic" );
      else
        myMetadata += tr( "Projected" );
    }
    myMetadata += QLatin1String( "</td></tr>\n" );

    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Spatial Extent" ) + QStringLiteral( "</td><td>" );
    const QList< QgsLayerMetadata::SpatialExtent > spatialExtents = extent.spatialExtents();
    for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : spatialExtents )
    {
      if ( notFirstRow )
      {
        myMetadata += QLatin1String( "<br />\n" );
      }
      myMetadata += QStringLiteral( "<strong>" ) + tr( "CRS" ) + QStringLiteral( ": </strong>" ) + spatialExtent.extentCrs.authid() + QStringLiteral( " - " );
      myMetadata += spatialExtent.extentCrs.description() + QStringLiteral( " - " );
      if ( spatialExtent.extentCrs.isGeographic() )
        myMetadata += tr( "Geographic" );
      else
        myMetadata += tr( "Projected" );
      myMetadata += QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + tr( "X Minimum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMinimum() ) + QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + tr( "Y Minimum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMinimum() ) + QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + tr( "X Maximum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMaximum() ) + QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + tr( "Y Maximum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMaximum() ) + QStringLiteral( "<br />" );
      if ( spatialExtent.bounds.zMinimum() || spatialExtent.bounds.zMaximum() )
      {
        myMetadata += QStringLiteral( "<strong>" ) + tr( "Z Minimum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMinimum() ) + QStringLiteral( "<br />" );
        myMetadata += QStringLiteral( "<strong>" ) + tr( "Z Maximum:" ) + QStringLiteral( " </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMaximum() );
      }
      notFirstRow = true;
    }
    myMetadata += QLatin1String( "</td></tr>\n" );
  }
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Temporal Extent" ) + QStringLiteral( "</td><td>" );
  const QList< QgsDateTimeRange > temporalExtents = extent.temporalExtents();
  notFirstRow = false;
  for ( const QgsDateTimeRange &temporalExtent : temporalExtents )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />\n" );
    }
    if ( temporalExtent.isInstant() )
    {
      myMetadata += QStringLiteral( "<strong>" ) + tr( "Instant:" ) + QStringLiteral( " </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    else
    {
      myMetadata += QStringLiteral( "<strong>" ) + tr( "Start:" ) + QStringLiteral( " </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) + QStringLiteral( "<br />\n" );
      myMetadata += QStringLiteral( "<strong>" ) + tr( "End:" ) + QStringLiteral( " </strong>" ) + temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "</table>\n" );
  return myMetadata;
}

QString QgsLayerMetadataFormatter::identificationSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );

  // Identifier
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Identifier" ) + QStringLiteral( "</td><td>" ) + mMetadata.identifier() + QStringLiteral( "</td></tr>\n" );

  // Parent Identifier
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Parent Identifier" ) + QStringLiteral( "</td><td>" ) + mMetadata.parentIdentifier() + QStringLiteral( "</td></tr>\n" );

  // Title
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Title" ) + QStringLiteral( "</td><td>" ) + mMetadata.title() + QStringLiteral( "</td></tr>\n" );

  // Type
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Type" ) + QStringLiteral( "</td><td>" ) + mMetadata.type() + QStringLiteral( "</td></tr>\n" );

  // Language
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Language" ) + QStringLiteral( "</td><td>" ) + mMetadata.language() + QStringLiteral( "</td></tr>\n" );

  // Abstract
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Abstract" ) + QStringLiteral( "</td><td>" ) + mMetadata.abstract() + QStringLiteral( "</td></tr>\n" );

  // Categories
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Categories" ) + QStringLiteral( "</td><td>" ) + mMetadata.categories().join( QStringLiteral( ", " ) ) + QStringLiteral( "</td></tr>\n" );

  // Keywords
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Keywords" ) + QStringLiteral( "</td><td>\n" );
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  if ( i.hasNext() )
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += QLatin1String( "<tr><th>" ) + tr( "Vocabulary" ) + QLatin1String( "</th><th>" ) + tr( "Items" ) + QLatin1String( "</th></tr>\n" );
    int j = 1;
    while ( i.hasNext() )
    {
      i.next();
      QString rowClass;
      if ( j % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + i.key() + QLatin1String( "</td><td>" ) + i.value().join( QStringLiteral( ", " ) ) + QLatin1String( "</td></tr>\n" );
      j++;
    }
    myMetadata += QLatin1String( "</table>\n" ); // End keywords table
  }
  myMetadata += QLatin1String( "</td></tr>\n" ); // End of keywords row
  myMetadata += QLatin1String( "</table>\n" ); // End identification table
  return myMetadata;
}

QString QgsLayerMetadataFormatter::historySectionHtml() const
{
  QString myMetadata;
  const QStringList historyItems = mMetadata.history();
  if ( historyItems.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + tr( "No history yet." ) + QStringLiteral( "</p>\n" );
  }
  else
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += QLatin1String( "<tr><th>" ) + tr( "ID" ) + QLatin1String( "</th><th>" ) + tr( "Action" ) + QLatin1String( "</th></tr>\n" );
    int i = 1;
    for ( const QString &history : historyItems )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td width=\"5%\">" ) + QString::number( i ) + QLatin1String( "</td><td>" ) + history + QLatin1String( "</td></tr>\n" );
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::linksSectionHtml() const
{
  QString myMetadata;
  const QList<QgsAbstractMetadataBase::Link> &links = mMetadata.links();
  if ( links.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + tr( "No links yet." ) + QStringLiteral( "</p>\n" );
  }
  else
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += QLatin1String( "<tr><th>" ) + tr( "ID" ) + QLatin1String( "</th><th>" ) + tr( "Name" ) + QLatin1String( "</th><th>" ) + tr( "Type" ) + QLatin1String( "</th><th>" ) + tr( "URL" ) + QLatin1String( "</th><th>" ) + tr( "Description" ) + QLatin1String( "</th><th>" ) + tr( "Format" ) + QLatin1String( "</th><th>" ) + tr( "MIME Type" ) + QLatin1String( "</th><th>" ) + tr( "Size" ) + QLatin1String( "</th></tr>\n" );
    int i = 1;
    for ( const QgsAbstractMetadataBase::Link &link : links )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + QString::number( i ) + QLatin1String( "</td><td>" ) + link.name + QLatin1String( "</td><td>" ) + link.type + QLatin1String( "</td><td>" ) + link.url + QLatin1String( "</td><td>" ) + link.description + QLatin1String( "</td><td>" ) + link.format + QLatin1String( "</td><td>" ) + link.mimeType + QLatin1String( "</td><td>" ) + link.size + QLatin1String( "</td></tr>\n" );
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}
