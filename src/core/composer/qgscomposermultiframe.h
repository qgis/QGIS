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
class QgsComposerItem;
class QgsComposition;
class QDomDocument;
class QDomElement;
class QRectF;
class QPainter;

/**Abstract base class for composer entries with the ability to distribute the content to several frames (items)*/
class CORE_EXPORT QgsComposerMultiFrame: public QObject
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

  protected:
    QgsComposition* mComposition;
    QList<QgsComposerFrame*> mFrameItems;
    ResizeMode mResizeMode;
    /**True: creates QgsMultiFrameCommands on internal changes (e.g. changing frames )*/
    bool mCreateUndoCommands;

  protected slots:
    void recalculateFrameSizes();
    /**Called before a frame is going to be removed (update frame list)*/
    void handleFrameRemoval( QgsComposerItem* item );
    /**Adapts to changed number of pages if resize type is RepeatOnEveryPage*/
    void handlePageChange();

  private:
    QgsComposerMultiFrame(); //forbidden

  signals:
    void changed();
};

#endif // QGSCOMPOSERMULTIFRAME_H
