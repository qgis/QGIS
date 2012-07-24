/***************************************************************************
                              qgscomposerhtml.h
    ------------------------------------------------------------
    begin                : Julli 2012
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

class QgsComposerHtml: public QgsComposerMultiFrame
{
  public:
    QgsComposerHtml( QgsComposition* c );
    ~QgsComposerHtml();

    void setUrl( const QUrl& url ) { mUrl = url; }
    const QUrl& url() const { return mUrl; }

    QSizeF totalSize() const;

  private:
    QUrl mUrl;
};

#endif // QGSCOMPOSERHTML_H
