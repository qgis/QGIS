/***************************************************************************
                              qgscomposerhtml.h
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERHTML_H
#define QGSCOMPOSERHTML_H

#include "qgscomposermultiframe.h"
#include <QUrl>

class QWebPage;

class QgsComposerHtml: public QgsComposerMultiFrame
{
    Q_OBJECT
  public:
    QgsComposerHtml( QgsComposition* c );
    QgsComposerHtml();
    ~QgsComposerHtml();

    void setUrl( const QUrl& url );
    const QUrl& url() const { return mUrl; }

    QSizeF totalSize() const;
    void render( QPainter* p, const QRectF& renderExtent );

  private slots:
    void frameLoaded( bool ok );

  private:
    QUrl mUrl;
    QWebPage* mWebPage;
    bool mLoaded;
};

#endif // QGSCOMPOSERHTML_H
