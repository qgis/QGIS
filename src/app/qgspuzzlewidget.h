/***************************************************************************
  qgspuzzlewidget.h
  --------------------------------------
  Date                 : 1st of April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPUZZLEWIDGET_H
#define QGSPUZZLEWIDGET_H

#include <QGraphicsView>

class QgsMapCanvas;


class QgsPuzzleWidget : public QGraphicsView
{
    Q_OBJECT
  public:
    explicit QgsPuzzleWidget( QgsMapCanvas *canvas = nullptr );

    bool letsGetThePartyStarted();

  protected:
    void mousePressEvent( QMouseEvent *event ) override;

  signals:
    void done();

  private:
    void updateTilePositions();

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QGraphicsScene *mScene = nullptr;
    int mSize = 4;  //!< Number of items in one row/column
    int mTileWidth = 0, mTileHeight = 0;
    QVector<QGraphicsItem *> mItems;
    QVector<QGraphicsItem *> mTextItems;
    QVector<int> mPositions;  //!< Indices of items (-1 where the piece is missing)
};

#endif // QGSPUZZLEWIDGET_H
