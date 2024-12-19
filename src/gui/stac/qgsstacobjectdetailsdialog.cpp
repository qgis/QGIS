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
#include <QDesktopServices>

///@cond PRIVATE

QgsStacObjectDetailsDialog::QgsStacObjectDetailsDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsStacObjectDetailsDialog::setStacObject( QgsStacObject *stacObject )
{
  if ( !stacObject )
    return;

  QStringList thumbnails;
  if ( QgsStacItem *item = dynamic_cast<QgsStacItem *>( stacObject ) )
  {
    const QMap<QString, QgsStacAsset> assets = item->assets();
    for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
    {
      if ( it->roles().contains( QLatin1String( "thumbnail" ) ) )
      {
        thumbnails.append( QStringLiteral( "<img src=\"%1\" border=1><br>" ).arg( it->href() ) );
      }
    }
  }

  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  // inject thumbnails
  QString html = stacObject->toHtml().replace( QLatin1String( "<head>" ), QStringLiteral( "<head>\n%1" ).arg( thumbnails.join( QString() ) ) );
  // inject stylesheet
  html = html.replace( QLatin1String( "<head>" ), QStringLiteral( R"raw(<head><style type="text/css">%1</style>)raw" ) ).arg( myStyle );

  mWebView->page()->setLinkDelegationPolicy( QWebPage::LinkDelegationPolicy::DelegateAllLinks );
  connect( mWebView, &QgsWebView::linkClicked, this, []( const QUrl &url ) {
    QDesktopServices::openUrl( url );
  } );
  mWebView->setHtml( html );
}

///@endcond
