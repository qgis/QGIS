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

#define SIP_NO_FILE

#include "qgis_core.h"

#ifdef WITH_QTWEBKIT
#include <QWebFrame>
#else

#include <QObject>
#include <QPainter>
#include <QUrl>
#include <QVariant>

/**
 * \ingroup core
 * \brief A collection of stubs to mimic the API of a QWebFrame on systems
 * where QtWebkit is not available.
 */
class CORE_EXPORT QWebFrame : public QObject
{
/// @cond NOT_STABLE_API
    Q_OBJECT

  public:
    QWebFrame( QObject *parent = nullptr )
      : QObject( parent )
    {

    }

    void setZoomFactor( qreal factor )
    {
      Q_UNUSED( factor )
    }

    void setScrollBarPolicy( Qt::Orientation orientation, Qt::ScrollBarPolicy scrollbarpolicy )
    {
      Q_UNUSED( orientation )
      Q_UNUSED( scrollbarpolicy )
    }

    void setHtml( const QString &html, const QUrl &url = QUrl() )
    {
      Q_UNUSED( html )
      Q_UNUSED( url )
      emit loadFinished( true );
    }

    QSize contentsSize() const
    {
      return QSize();
    }

    void render( QPainter *, const QRegion = QRegion() )
    {

    }

    void addToJavaScriptWindowObject( const QString &, QObject * )
    {

    }

    QVariant evaluateJavaScript( const QString & )
    {
      return QVariant();
    }

  signals:
    void loadFinished( bool ok );

    void javaScriptWindowObjectCleared();
/// @endcond
};
#endif
#endif // QGSWEBFRAME_H
