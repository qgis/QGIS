/***************************************************************************

               ----------------------------------------------------
              date                 : 19.5.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBVIEW_H
#define QGSWEBVIEW_H


#define SIP_NO_FILE

#include <QWidget>

#ifdef WITH_QTWEBKIT
#include <QWebView>
#include <QDesktopWidget>

#include "qgis_core.h"


/**
 * \ingroup core
 */
class CORE_EXPORT QgsWebView : public QWebView
{
    Q_OBJECT

  public:
    explicit QgsWebView( QWidget *parent = nullptr )
      : QWebView( parent )
    {
      QDesktopWidget desktop;
      // Apply zoom factor for HiDPI screens
      if ( desktop.logicalDpiX() > 96 )
      {
        setZoomFactor( desktop.logicalDpiX() / 96 );
      }
    }
};
#else
#include "qgswebpage.h"
#include <QTextBrowser>

class QPrinter;

/**
 * \ingroup core
 * \brief The QgsWebView class is a collection of stubs to mimic the API of QWebView on systems where the real
 * library is not available. It should be used instead of QWebView inside QGIS.
 *
 * If QGIS is compiled WITH_QTWEBKIT This will simply be a subclass of QWebView. If it is compiled with
 * WITH_QTWEBKIT=OFF then this will be an empty QWidget. If you miss methods in here that you would like to use,
 * please add additional stubs.
 */
class CORE_EXPORT QgsWebView : public QTextBrowser
{

/// @cond NOT_STABLE_API
    Q_OBJECT
  public:
    explicit QgsWebView( QWidget *parent = nullptr )
      : QTextBrowser( parent )
      , mSettings( new QWebSettings() )
      , mPage( new QWebPage( this ) )
    {
      connect( this, &QTextBrowser::anchorClicked, this, &QgsWebView::linkClicked );
      connect( this, &QgsWebView::pageLoadFinished, mPage, &QWebPage::loadFinished );
    }

    ~QgsWebView()
    {
      delete mSettings;
      delete mPage;
    }

    void setUrl( const QUrl &url )
    {
      setSource( url );
    }

    void load( const QUrl &url )
    {
      setSource( url );
    }

    QWebPage *page() const
    {
      return mPage;
    }

    QWebSettings *settings() const
    {
      return mSettings;
    }

    virtual QgsWebView *createWindow( QWebPage::WebWindowType )
    {
      return new QgsWebView();
    }

    void setContent( const QByteArray &data, const QString &contentType, const QUrl & )
    {
      QString text = QString::fromUtf8( data );
      if ( contentType == "text/html" )
        setHtml( text );
      else
        setPlainText( text );

      emit pageLoadFinished( true );
    }

    void print( QPrinter * )
    {
    }

  signals:
    void linkClicked( const QUrl &link );

    void pageLoadFinished( bool ok );

  private:
    QWebSettings *mSettings = nullptr;
    QWebPage *mPage = nullptr;

/// @endcond
};
#endif

#endif // QGSWEBVIEW_H
