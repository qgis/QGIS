/***************************************************************************
                              qgscomposermultiframe.h
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

#ifndef QGSCOMPOSERMULTIFRAME_H
#define QGSCOMPOSERMULTIFRAME_H

#include <QObject>
#include <QSizeF>

class QgsComposerFrame;
class QgsComposition;
class QRectF;
class QPainter;

/**Abstract base class for composer entries with the ability to distribute the content to several frames (items)*/
class QgsComposerMultiFrame: public QObject
{
    Q_OBJECT
  public:

    enum ResizeMode
    {
      UseExistingFrames = 0,
      ExtendToNextPage //duplicates last frame to next page to fit the total size
    };

    QgsComposerMultiFrame( QgsComposition* c );
    virtual ~QgsComposerMultiFrame();
    virtual QSizeF totalSize() const = 0;
    virtual void render( QPainter* p, const QRectF& renderExtent ) = 0;

    void addFrame( QgsComposerFrame* frame );
    void removeFrame( int i );

    void setResizeMode( ResizeMode mode ) { mResizeMode = mode; }
    ResizeMode resizeMode() const { return mResizeMode; }

  protected:
    QgsComposition* mComposition;
    QList<QgsComposerFrame*> mFrameItems;
    ResizeMode mResizeMode;

  protected slots:
    void recalculateFrameSizes();

  private:
    QgsComposerMultiFrame(); //forbidden
};

#endif // QGSCOMPOSERMULTIFRAME_H
