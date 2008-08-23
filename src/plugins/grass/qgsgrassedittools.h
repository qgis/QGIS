/***************************************************************************
    qgsgrassedittools.h  -    GRASS Edit tools
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSEDITTOOLS_H
#define QGSGRASSEDITTOOLS_H

#include "qgsmaptool.h"

class QgsGrassEdit;



class QgsGrassEditTool : public QgsMapTool
{
  public:
    QgsGrassEditTool( QgsGrassEdit* edit );

    //! events from canvas
    virtual void canvasPressEvent( QMouseEvent * e );
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! functions to be overridden by grass edit tools
    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button ) { }
    virtual void mouseMove( QgsPoint & point ) { }

  protected:
    QgsGrassEdit* e;
};

// ------------------------------------------------------------------
// NEW POINT + NEW CENTROID
// ------------------------------------------------------------------

class QgsGrassEditNewPoint : public QgsGrassEditTool
{
  public:
    QgsGrassEditNewPoint( QgsGrassEdit* edit, bool newCentroid );
    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );

  private:
    //! true if creating new centroid, false if creating new point
    bool mNewCentroid;
};

// ------------------------------------------------------------------
// NEW LINE + NEW BOUNDARY
// ------------------------------------------------------------------

class QgsGrassEditNewLine : public QgsGrassEditTool
{
  public:
    QgsGrassEditNewLine( QgsGrassEdit* edit, bool newBoundary );

    virtual void activate();
    virtual void deactivate();

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
    virtual void mouseMove( QgsPoint & point );

  private:
    //! true if creating new boundary, false if creating new line
    bool mNewBoundary;
};

// ------------------------------------------------------------------
// MOVE VERTEX
// ------------------------------------------------------------------

class QgsGrassEditMoveVertex : public QgsGrassEditTool
{
  public:
    QgsGrassEditMoveVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
    virtual void mouseMove( QgsPoint & point );
};

// ------------------------------------------------------------------
// ADD VERTEX
// ------------------------------------------------------------------

class QgsGrassEditAddVertex : public QgsGrassEditTool
{
  public:
    QgsGrassEditAddVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
    virtual void mouseMove( QgsPoint & point );
};

// ------------------------------------------------------------------
// DELETE VERTEX
// ------------------------------------------------------------------

class QgsGrassEditDeleteVertex : public QgsGrassEditTool
{
  public:
    QgsGrassEditDeleteVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
};

// ------------------------------------------------------------------
// MOVE LINE
// ------------------------------------------------------------------

class QgsGrassEditMoveLine : public QgsGrassEditTool
{
  public:
    QgsGrassEditMoveLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
    virtual void mouseMove( QgsPoint & point );
};

// ------------------------------------------------------------------
// DELETE LINE
// ------------------------------------------------------------------

class QgsGrassEditDeleteLine : public QgsGrassEditTool
{
  public:
    QgsGrassEditDeleteLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
};

// ------------------------------------------------------------------
// SPLIT LINE
// ------------------------------------------------------------------

class QgsGrassEditSplitLine : public QgsGrassEditTool
{
  public:
    QgsGrassEditSplitLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
};

// ------------------------------------------------------------------
// EDIT ATTRIBUTES
// ------------------------------------------------------------------

class QgsGrassEditAttributes : public QgsGrassEditTool
{
  public:
    QgsGrassEditAttributes( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::ButtonState button );
};

#endif
