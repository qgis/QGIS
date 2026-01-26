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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsgui.h"
#include "qgsstacitem.h"

#include <QDesktopServices>

#include "moc_qgsstacobjectdetailsdialog.cpp"

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

  const QString thumbnailHtml = thumbnails.join( QString() );
  const QString bodyHtml = stacObject->toHtml();
  setContent( bodyHtml, thumbnailHtml );
}


void QgsStacObjectDetailsDialog::setContentFromStacAsset( const QString &assetId, const QgsStacAsset *stacAsset )
{
  const QString thumbnailHtml = isThumbnailAsset( stacAsset ) ? thumbnailHtmlContent( stacAsset ) : QString();
  const QString bodyHtml = stacAsset->toHtml( assetId );
  setContent( bodyHtml, thumbnailHtml );
}


void QgsStacObjectDetailsDialog::setContent( QString bodyHtml, QString thumbnailHtml )
{
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  QString html = u"<html>\n<head>\n"_s;
  html += u"<style type=\"text/css\">%1</style>\n"_s.arg( myStyle );
  html += u"%1\n"_s.arg( thumbnailHtml );
  html += "</head>\n<body>\n"_L1;
  html += u"%1\n"_s.arg( bodyHtml );
  html += "</body>\n</html>\n"_L1;
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
  return stacAsset->roles().contains( "thumbnail"_L1 );
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
  return u"<img src=\"%1\" border=1><br>"_s.arg( uri );
}

///@endcond
