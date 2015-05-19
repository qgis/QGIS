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

class QWebFrame : public QObject
{
  Q_OBJECT

  public:
    QWebFrame();

    void setZoomFactor( qreal factor );

    void setScrollBarPolicy( Qt::Orientation, Qt::ScrollBarPolicy );

    void setHtml( const QString&, const QUrl = QUrl() );

    QSize contentsSize() const;

    void render( QPainter*, const QRegion = QRegion() );

    void addToJavaScriptWindowObject( const QString&, QObject* );

  signals:
    void javaScriptWindowObjectCleared();
};
#endif
#endif // QGSWEBFRAME_H
