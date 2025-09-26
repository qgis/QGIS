/***************************************************************************
    qgsstacobjectdetailsdialog.cpp
    ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacobjectdetailsdialog.h"
#include "moc_qgsstacobjectdetailsdialog.cpp"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsstacitem.h"
#include "qgsauthmanager.h"

#include <QDesktopServices>

///@cond PRIVATE

QgsStacObjectDetailsDialog::QgsStacObjectDetailsDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
}


void QgsStacObjectDetailsDialog::setContentFromStacObject( QgsStacObject *stacObject )
{
  if ( !stacObject )
    return;

  QStringList thumbnails;
  if ( QgsStacItem *item = dynamic_cast<QgsStacItem *>( stacObject ) )
  {
    const QMap<QString, QgsStacAsset> assets = item->assets();
    for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
    {
      if ( isThumbnailAsset( &it.value() ) )
      {
        thumbnails.append( thumbnailHtmlContent( &it.value() ) );
      }
    }
  }

  QString thumbnailHtml = thumbnails.join( QString() );
  QString bodyHtml = stacObject->toHtml();
  setContent( bodyHtml, thumbnailHtml );
}


void QgsStacObjectDetailsDialog::setContentFromStacAsset( const QgsStacAsset *stacAsset )
{
  QString thumbnailHtml = QString( "" );
  if ( isThumbnailAsset( stacAsset ) )
  {
    thumbnailHtml = thumbnailHtmlContent( stacAsset );
  }
  QString bodyHtml = stacAsset->toHtml();
  setContent( bodyHtml, thumbnailHtml );
}


void QgsStacObjectDetailsDialog::setContent( QString bodyHtml, QString thumbnailHtml )
{
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  QString html = QStringLiteral( "<html>\n<head>\n" );
  html += QStringLiteral( "<style type=\"text/css\">%1</style>\n" ).arg( myStyle );
  html += QStringLiteral( "%1\n" ).arg( thumbnailHtml );
  html += QStringLiteral( "</head>\n<body>\n" );
  html += QStringLiteral( "%1\n" ).arg( bodyHtml );
  html += QLatin1String( "</body>\n</html>\n" );
  mWebView->page()->setLinkDelegationPolicy( QWebPage::LinkDelegationPolicy::DelegateAllLinks );
  connect( mWebView, &QgsWebView::linkClicked, this, []( const QUrl &url ) {
    QDesktopServices::openUrl( url );
  } );
  mWebView->setHtml( html );
}

void QgsStacObjectDetailsDialog::setAuthcfg( const QString &authcfg )
{
  mAuthcfg = authcfg;
}

bool QgsStacObjectDetailsDialog::isThumbnailAsset( const QgsStacAsset *stacAsset )
{
  return stacAsset->roles().contains( QLatin1String( "thumbnail" ) );
}

QString QgsStacObjectDetailsDialog::thumbnailHtmlContent( const QgsStacAsset *stacAsset )
{
  QString uri = stacAsset->href();
  if ( !mAuthcfg.isEmpty() )
  {
    QStringList connectionItems;
    connectionItems << uri;
    if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, mAuthcfg ) )
    {
      uri = connectionItems.first();
    }
  }
  return QStringLiteral( "<img src=\"%1\" border=1><br>" ).arg( uri );
}

///@endcond
