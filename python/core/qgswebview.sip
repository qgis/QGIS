/***************************************************************************
  qgswebview.sip - QgsWebView

 ---------------------
 begin                : May 2016
 copyright            : (C) 2016 by Jürgen Fischer
 email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%If (WebKit)
class QgsWebView : QWebView
{
%TypeHeaderCode
#include "qgswebview.h"
%End
   public:
     QgsWebView( QWidget* parent = 0 );
 };
%End

%If (!WebKit)
class QgsWebView : QWidget
{
%TypeHeaderCode
#include "qgswebview.h"
%End
  public:
    explicit QgsWebView( QWidget* parent = 0 );

    void setUrl( const QUrl& url );
    void load( const QUrl& url );
    QWebPage* page() const;
    QWebSettings* settings() const;
    void setHtml( const QString& html );
    virtual QgsWebView* createWindow( QWebPage::WebWindowType );
    void setContent( const QByteArray&, const QString&, const QUrl& );
    void print( QPrinter* );
};
%End
