#include <QStringBuilder>

#include "qgslayermetadataformatter.h"
#include "qgslayermetadata.h"


QgsLayerMetadataFormatter::QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata )
  : mMetadata( metadata )
{
}

QString QgsLayerMetadataFormatter::accessSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Fees" ) + QLatin1String( "</td><td>" ) + mMetadata.fees() + QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Licenses" ) + QLatin1String( "</td><td>" ) + mMetadata.licenses().join( QStringLiteral( "<br />" ) ) + QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Rights" ) + QLatin1String( "</td><td>" ) + mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Constraints" ) + QLatin1String( "</td><td>" );
  const QList<QgsLayerMetadata::Constraint> &constraints = mMetadata.constraints();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::Constraint &constraint : constraints )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />" );
    }
    myMetadata += QLatin1String( "<strong>" ) + constraint.type + QLatin1String( ": </strong>" ) + constraint.constraint;
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "</table>\n" );
  return myMetadata;
}

QString QgsLayerMetadataFormatter::contactsSectionHtml() const
{
  const QList<QgsLayerMetadata::Contact> &contacts = mMetadata.contacts();
  QString myMetadata;
  if ( contacts.isEmpty() )
  {
    myMetadata += QLatin1String( "<p>" ) + QObject::tr( "No contact yet." ) + QLatin1String( "</p>" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Name" ) + "</th><th>" + QObject::tr( "Position" ) + "</th><th>" + QObject::tr( "Organization" ) + "</th><th>" + QObject::tr( "Role" ) + "</th><th>" + QObject::tr( "Email" ) + "</th><th>" + QObject::tr( "Voice" ) + "</th><th>" + QObject::tr( "Fax" ) + "</th><th>" + QObject::tr( "Addresses" ) + "</th></tr>\n";
    int i = 1;
    for ( const QgsLayerMetadata::Contact &contact : contacts )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QString( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + QString::number( i ) + "</td><td>" + contact.name + "</td><td>" + contact.position + "</td><td>" + contact.organization + "</td><td>" + contact.role + "</td><td>" + contact.email + "</td><td>" + contact.voice + "</td><td>" + contact.fax + "</td><td>";
      bool notFirstRow = false;
      for ( const QgsLayerMetadata::Address &oneAddress : contact.addresses )
      {
        if ( notFirstRow )
        {
          myMetadata += QLatin1String( "<br />\n" );
        }
        if ( ! oneAddress.type.isEmpty() )
        {
          myMetadata += oneAddress.type + QLatin1String( "<br />" );
        }
        if ( ! oneAddress.address.isEmpty() )
        {
          myMetadata += oneAddress.address + QLatin1String( "<br />" );
        }
        if ( ! oneAddress.postalCode.isEmpty() )
        {
          myMetadata += oneAddress.postalCode + QLatin1String( "<br />" );
        }
        if ( ! oneAddress.city.isEmpty() )
        {
          myMetadata += oneAddress.city + QLatin1String( "<br />" );
        }
        if ( ! oneAddress.administrativeArea.isEmpty() )
        {
          myMetadata += oneAddress.administrativeArea + QLatin1String( "<br />" );
        }
        if ( ! oneAddress.country.isEmpty() )
        {
          myMetadata += oneAddress.country;
        }
        notFirstRow = true;
      }
      myMetadata += "</td></tr>\n";
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::extentSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "CRS" ) + QLatin1String( "</td><td>" ) + mMetadata.crs().authid() + QLatin1String( " - " );
  myMetadata += mMetadata.crs().description() + QLatin1String( " - " );
  if ( mMetadata.crs().isGeographic() )
    myMetadata += QObject::tr( "Geographic" );
  else
    myMetadata += QObject::tr( "Projected" );
  myMetadata += QLatin1String( "</td></tr>\n" );

  const QgsLayerMetadata::Extent extent = mMetadata.extent();
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Spatial Extent" ) + QLatin1String( "</td><td>" );
  const QList< QgsLayerMetadata::SpatialExtent > spatialExtents = extent.spatialExtents();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : spatialExtents )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />\n" );
    }
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "CRS" ) + QStringLiteral( ": </strong>" ) + spatialExtent.extentCrs.authid() + QLatin1String( " - " );
    myMetadata += spatialExtent.extentCrs.description() + QLatin1String( " - " );
    if ( spatialExtent.extentCrs.isGeographic() )
      myMetadata += QObject::tr( "Geographic" );
    else
      myMetadata += QObject::tr( "Projected" );
    myMetadata += QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "X Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMinimum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Y Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMinimum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "X Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMaximum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Y Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMaximum() ) + QStringLiteral( "<br />" );
    if ( spatialExtent.bounds.zMinimum() || spatialExtent.bounds.zMinimum() )
    {
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Z Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMinimum() ) + QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Z Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMaximum() );
    }
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Temporal Extent" ) + QLatin1String( "</td><td>" );
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
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Instant" ) + QStringLiteral( ": </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    else
    {
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Start" ) + QStringLiteral( ": </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) + QStringLiteral( "<br />\n" );
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "End" ) + QStringLiteral( ": </strong>" ) + temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
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
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Identifier" ) + QLatin1String( "</td><td>" ) + mMetadata.identifier() + QLatin1String( "</td></tr>\n" );

  // Parent Identifier
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Parent Identifier" ) + QLatin1String( "</td><td>" ) + mMetadata.parentIdentifier() + QLatin1String( "</td></tr>\n" );

  // Title
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Title" ) + QLatin1String( "</td><td>" ) + mMetadata.title() + QLatin1String( "</td></tr>\n" );

  // Type
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Type" ) + QLatin1String( "</td><td>" ) + mMetadata.type() + QLatin1String( "</td></tr>\n" );

  // Language
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Language" ) + QLatin1String( "</td><td>" ) + mMetadata.language() + QLatin1String( "</td></tr>\n" );

  // Abstract
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Abstract" ) + QLatin1String( "</td><td>" ) + mMetadata.abstract() + QLatin1String( "</td></tr>\n" );

  // Categories
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Categories" ) + QLatin1String( "</td><td>" ) + mMetadata.categories().join( QStringLiteral( ", " ) ) + QLatin1String( "</td></tr>\n" );

  // Keywords
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) + QObject::tr( "Keywords" ) + QLatin1String( "</td><td>\n" );
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  if ( i.hasNext() )
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "Vocabulary" ) + "</th><th>" + QObject::tr( "Items" ) + "</th></tr>\n";
    int j = 1;
    while ( i.hasNext() )
    {
      i.next();
      QString rowClass;
      if ( j % 2 )
        rowClass = QString( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + i.key() + "</td><td>" + i.value().join( QStringLiteral( ", " ) ) + "</td></tr>\n";
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
    myMetadata += QLatin1String( "<p>" ) + QObject::tr( "No history yet." ) + QLatin1String( "</p>\n" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Action" ) + "</th></tr>\n";
    int i = 1;
    for ( const QString &history : historyItems )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QString( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td width=\"5%\">" + QString::number( i ) + "</td><td>" + history + "</td></tr>\n";
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::linksSectionHtml() const
{
  QString myMetadata;
  const QList<QgsLayerMetadata::Link> &links = mMetadata.links();
  if ( links.isEmpty() )
  {
    myMetadata += QLatin1String( "<p>" ) + QObject::tr( "No links yet." ) + QLatin1String( "</p>\n" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Name" ) + "</th><th>" + QObject::tr( "Type" ) + "</th><th>" + QObject::tr( "URL" ) + "</th><th>" + QObject::tr( "Description" ) + "</th><th>" + QObject::tr( "Format" ) + "</th><th>" + QObject::tr( "MIME Type" ) + "</th><th>" + QObject::tr( "Size" ) + "</th></tr>\n";
    int i = 1;
    for ( const QgsLayerMetadata::Link &link : links )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QString( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + QString::number( i ) + "</td><td>" + link.name + "</td><td>" + link.type + "</td><td>" + link.url + "</td><td>" + link.description + "</td><td>" + link.format + "</td><td>" + link.mimeType + "</td><td>" + link.size + "</td></tr>\n";
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}
