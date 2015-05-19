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

class QgsWebView : public QWebView
{
    Q_OBJECT

  public:
    explicit QgsWebView(QWidget* parent = 0)
      : QWebView( parent )
    {}
};
#else
#include "qgswebpage.h"

class QgsWebView : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsWebView(QWidget *parent = 0) : QWidget(parent )
    {
    }

    void setUrl( const QUrl& url )
    {

    }

    void load( const QUrl& url )
    {

    }

    QWebPage* page() const
    {

    }

    QWebSettings* settings() const
    {

    }

    void setHtml( const QString& )
    {

    }

    virtual QgsWebView* createWindow(QWebPage::WebWindowType)
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
};
#endif

#endif // QGSWEBVIEW_H
