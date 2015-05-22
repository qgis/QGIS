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

#ifndef QGSWEBFRAME_H
#define QGSWEBFRAME_H

#ifdef WITH_QTWEBKIT
#include <QtWebKit/QWebFrame>
#else

#include <QObject>
#include <QPainter>
#include <QUrl>

class CORE_EXPORT QWebFrame : public QObject
{
    Q_OBJECT

  public:
    QWebFrame( QObject* parent = 0 )
        : QObject( parent )
    {

    }

    void setZoomFactor( qreal factor )
    {
      Q_UNUSED( factor );
    }

    void setScrollBarPolicy( Qt::Orientation orientation, Qt::ScrollBarPolicy scrollbarpolicy )
    {
      Q_UNUSED( orientation );
      Q_UNUSED( scrollbarpolicy );
    }

    void setHtml( const QString& html, const QUrl& url = QUrl() )
    {
      Q_UNUSED( html );
      Q_UNUSED( url );
    }

    QSize contentsSize() const
    {
      return QSize();
    }

    void render( QPainter*, const QRegion = QRegion() )
    {

    }

    void addToJavaScriptWindowObject( const QString&, QObject* )
    {

    }

  signals:
    void javaScriptWindowObjectCleared();
};
#endif
#endif // QGSWEBFRAME_H
