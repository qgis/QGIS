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

#include <QWidget>
#include <QPrinter>

#ifdef WITH_QTWEBKIT
#include <QtWebKit/QWebView>

class CORE_EXPORT QgsWebView : public QWebView
{
    Q_OBJECT

  public:
    explicit QgsWebView( QWidget* parent = 0 )
        : QWebView( parent )
    {}
};
#else
#include "qgswebpage.h"

/**
 * @brief The QgsWebView class is a collection of stubs to mimic the API of QWebView on systems where the real
 * library is not available. It should be used instead of QWebView inside QGIS.
 *
 * If QGIS is compiled WITH_QTWEBKIT This will simply be a subclass of QWebView. If it is compiled with
 * WITH_QTWEBKIT=OFF then this will be an empty QWidget. If you miss methods in here that you would like to use,
 * please add additional stubs.
 */
class CORE_EXPORT QgsWebView : public QWidget
{

/// @cond
    Q_OBJECT
  public:
    explicit QgsWebView( QWidget *parent = 0 )
        : QWidget( parent )
        , mSettings( new QWebSettings() )
        , mPage( new QWebPage() )
    {
    }

    ~QgsWebView()
    {
      delete mSettings;
      delete mPage;
    }

    void setUrl( const QUrl& url )
    {
      Q_UNUSED( url );

    }

    void load( const QUrl& url )
    {
      Q_UNUSED( url );
    }

    QWebPage* page() const
    {
      return mPage;
    }

    QWebSettings* settings() const
    {
      return mSettings;
    }

    void setHtml( const QString& html )
    {
      Q_UNUSED( html );
    }

    virtual QgsWebView* createWindow( QWebPage::WebWindowType )
    {
      return new QgsWebView();
    }

    void setContent( const QByteArray&, const QString&, const QUrl& )
    {

    }

    void print( QPrinter* )
    {

    }

  signals:

  public slots:

  private:
    QWebSettings* mSettings;
    QWebPage* mPage;

/// @endcond
};
#endif

#endif // QGSWEBVIEW_H
