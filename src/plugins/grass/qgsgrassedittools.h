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
#include "qgspoint.h"

class QgsGrassEdit;



class QgsGrassEditTool : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsGrassEditTool( QgsGrassEdit* edit );

    //! events from canvas
    virtual void canvasPressEvent( QMouseEvent * e ) override;
    virtual void canvasMoveEvent( QMouseEvent * e ) override;

    //! functions to be overridden by grass edit tools
    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button )
    { Q_UNUSED( point ); Q_UNUSED( button ); }
    virtual void mouseMove( QgsPoint & point )
    { Q_UNUSED( point ); }

  protected:
    QgsGrassEdit* e;
};

// ------------------------------------------------------------------
// NEW POINT + NEW CENTROID
// ------------------------------------------------------------------

class QgsGrassEditNewPoint : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditNewPoint( QgsGrassEdit* edit, bool newCentroid );
    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;

  private:
    //! true if creating new centroid, false if creating new point
    bool mNewCentroid;
};

// ------------------------------------------------------------------
// NEW LINE + NEW BOUNDARY
// ------------------------------------------------------------------

class QgsGrassEditNewLine : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditNewLine( QgsGrassEdit* edit, bool newBoundary );

    virtual void activate() override;
    virtual void deactivate() override;

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
    virtual void mouseMove( QgsPoint & point ) override;

  private:
    //! true if creating new boundary, false if creating new line
    bool mNewBoundary;
};

// ------------------------------------------------------------------
// MOVE VERTEX
// ------------------------------------------------------------------

class QgsGrassEditMoveVertex : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditMoveVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
    virtual void mouseMove( QgsPoint & point ) override;
};

// ------------------------------------------------------------------
// ADD VERTEX
// ------------------------------------------------------------------

class QgsGrassEditAddVertex : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditAddVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
    virtual void mouseMove( QgsPoint & point ) override;
};

// ------------------------------------------------------------------
// DELETE VERTEX
// ------------------------------------------------------------------

class QgsGrassEditDeleteVertex : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditDeleteVertex( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
};

// ------------------------------------------------------------------
// MOVE LINE
// ------------------------------------------------------------------

class QgsGrassEditMoveLine : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditMoveLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
    virtual void mouseMove( QgsPoint & point ) override;
};

// ------------------------------------------------------------------
// DELETE LINE
// ------------------------------------------------------------------

class QgsGrassEditDeleteLine : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditDeleteLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
};

// ------------------------------------------------------------------
// SPLIT LINE
// ------------------------------------------------------------------

class QgsGrassEditSplitLine : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditSplitLine( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
};

// ------------------------------------------------------------------
// EDIT ATTRIBUTES
// ------------------------------------------------------------------

class QgsGrassEditAttributes : public QgsGrassEditTool
{
    Q_OBJECT
  public:
    QgsGrassEditAttributes( QgsGrassEdit* edit );

    virtual void mouseClick( QgsPoint & point, Qt::MouseButton button ) override;
};

#endif
