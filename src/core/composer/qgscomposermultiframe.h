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

#include "qgscomposerobject.h"
#include <QObject>
#include <QSizeF>
#include <QPointF>

class QgsComposerFrame;
class QgsComposerItem;
class QgsComposition;
class QDomDocument;
class QDomElement;
class QRectF;
class QPainter;

/**Abstract base class for composer entries with the ability to distribute the content to several frames (items)*/
class CORE_EXPORT QgsComposerMultiFrame: public QgsComposerObject
{
    Q_OBJECT
  public:

    enum ResizeMode
    {
      UseExistingFrames = 0,
      ExtendToNextPage, //uses the next page(s) until the content has been printed
      RepeatOnEveryPage, //repeats the same frame on every page
      RepeatUntilFinished //duplicates last frame to next page to fit the total size
    };

    QgsComposerMultiFrame( QgsComposition* c, bool createUndoCommands );
    virtual ~QgsComposerMultiFrame();
    virtual QSizeF totalSize() const = 0;
    virtual void render( QPainter* p, const QRectF& renderExtent ) = 0;

    virtual void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true ) = 0;

    /**Finds the optimal position to break a frame at.
     * @param yPos maximum vertical position for break
     * @returns the optimal breakable position which occurs in the multi frame close
     * to and before the specified yPos
     * @note added in version 2.3*/
    virtual double findNearbyPageBreak( double yPos ) { return yPos; }

    void removeFrame( int i );

    void update();

    void setResizeMode( ResizeMode mode );
    ResizeMode resizeMode() const { return mResizeMode; }

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const = 0;
    bool _writeXML( QDomElement& elem, QDomDocument& doc, bool ignoreFrames = false ) const;

    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false ) = 0;
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    QgsComposition* composition() { return mComposition; }

    bool createUndoCommands() const { return mCreateUndoCommands; }
    void setCreateUndoCommands( bool enabled ) { mCreateUndoCommands = enabled; }

    /**Removes and deletes all frames from mComposition*/
    void deleteFrames();

    /** Return the number of frames associated with this multiframeset.
    @note added in 2.0, replaces nFrames
    **/
    int frameCount() const { return mFrameItems.size(); }
    QgsComposerFrame* frame( int i ) const;

    /**Creates a new frame and adds it to the multi frame and composition.
     * @param currentFrame an existing QgsComposerFrame from which to copy the size
     * and general frame properties (eg frame style, background, rendering settings).
     * @param pos position of top-left corner of the new frame
     * @param size size of the new frame
     * @returns new QgsComposerFrame
     * @note added in version 2.3
     */
    QgsComposerFrame* createNewFrame( QgsComposerFrame* currentFrame, QPointF pos, QSizeF size );

    /**Get multiframe display name.
     * @returns display name for item
     * @note added in version 2.5
    */
    virtual QString displayName() const;

  public slots:

    /**Recalculates the portion of the multiframe item which is shown in each of it's
     * component frames. If the resize mode is set to anything but UseExistingFrames then
     * this may cause new frames to be added or frames to be removed, in order to fit
     * the current size of the multiframe's content.
     */
    void recalculateFrameSizes();

  protected:
    QList<QgsComposerFrame*> mFrameItems;
    ResizeMode mResizeMode;
    /**True: creates QgsMultiFrameCommands on internal changes (e.g. changing frames )*/
    bool mCreateUndoCommands;

  protected slots:
    /**Called before a frame is going to be removed (update frame list)*/
    void handleFrameRemoval( QgsComposerItem* item );
    /**Adapts to changed number of pages if resize type is RepeatOnEveryPage*/
    void handlePageChange();

  private:
    QgsComposerMultiFrame(); //forbidden

    bool mIsRecalculatingSize;

  signals:
    void changed();

    /**Emitted when the contents of the multi frame have changed and the frames
     * must be redrawn.
    */
    void contentsChanged();
};

#endif // QGSCOMPOSERMULTIFRAME_H
