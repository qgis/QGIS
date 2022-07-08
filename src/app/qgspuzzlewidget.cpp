/***************************************************************************
  qgspuzzlewidget.cpp
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

#include "qgspuzzlewidget.h"

#include "qgsmapcanvas.h"

#include <QGraphicsPixmapItem>
#include <QMessageBox>


static bool testSolved( QVector<int> &positions )
{
  for ( int i = 0; i < positions.count() - 1; ++i )
  {
    if ( positions[i] != i )
      return false;
  }
  return true;
}

static int findEmpty( QVector<int> &positions )
{
  for ( int i = 0; i < positions.count(); ++i )
  {
    if ( positions[i] == -1 )
      return i;
  }
  Q_ASSERT( false );
  return -1;
}

// half of the possible permutations lead to an unsolvable puzzle, so we initialize the puzzle by
// doing valid moves
// https://en.wikipedia.org/wiki/15_puzzle
static void shuffle( QVector<int> &positions, int count )
{
  const int size = sqrt( positions.count() );
  int idxEmpty = findEmpty( positions );
  int cEmpty = idxEmpty % size, rEmpty = idxEmpty / size;
  const int moveX[] = { 0, 0, 1, -1 };
  const int moveY[] = { 1, -1, 0, 0 };
  int cOther, rOther;
  for ( int i = 0; i < count; ++i )
  {
    do
    {
      const int move = qrand() % 4;
      cOther = cEmpty + moveX[move];
      rOther = rEmpty + moveY[move];
    }
    while ( cOther < 0 || cOther >= size || rOther < 0 || rOther >= size );

    const int idxOther = rOther * size + cOther;
    std::swap( positions[idxEmpty], positions[idxOther] );
    idxEmpty = idxOther;
    cEmpty = idxEmpty % size;
    rEmpty = idxEmpty / size;
  }
}

QgsPuzzleWidget::QgsPuzzleWidget( QgsMapCanvas *canvas )
  : mCanvas( canvas )
{
  setInteractive( false );
  setBackgroundBrush( QBrush( Qt::lightGray ) );
}

void QgsPuzzleWidget::mousePressEvent( QMouseEvent *event )
{
  if ( mTileWidth == 0 || mTileHeight == 0 )
    return;  // not initialized

  const int idxEmpty = findEmpty( mPositions );
  const int rEmpty = idxEmpty / mSize;
  const int cEmpty = idxEmpty % mSize;
  const int cMouse = event->pos().x() / mTileWidth;
  const int rMouse = event->pos().y() / mTileHeight;
  const int idxMouse = rMouse * mSize + cMouse;
  const int dx = cMouse - cEmpty;
  const int dy = rMouse - rEmpty;

  if ( ( dx == 0 && std::abs( dy ) == 1 ) || ( dy == 0 && std::abs( dx ) == 1 ) )
  {
    std::swap( mPositions[idxEmpty], mPositions[idxMouse] );
    updateTilePositions();
    if ( testSolved( mPositions ) )
    {
      QMessageBox::information( nullptr, tr( "QGIS" ), tr( "Well done!\n\nNow let's get back to work, shall we?" ) );
      emit done();
    }
  }
  else if ( dx == 0 && dy == 0 )
  {
    // toggle text help when clicked on empty tile
    for ( int i = 0; i < mTextItems.count(); ++i )
      mTextItems[i]->setVisible( !mTextItems[i]->isVisible() );
  }
}

bool QgsPuzzleWidget::letsGetThePartyStarted()
{
  mPositions.clear();
  delete mScene;
  mItems.clear();
  mTextItems.clear();
  mTileWidth = 0;
  mTileHeight = 0;

  if ( !mCanvas->layerCount() )
    return false;

  mScene = new QGraphicsScene;

  QTemporaryFile f;
  f.open();
  f.close();

  const QString filename( f.fileName() );
  mCanvas->saveAsImage( filename );

  QPixmap pixmap;
  pixmap.load( filename );
  pixmap = pixmap.scaled( mCanvas->width() - 2, mCanvas->height() - 2 );

  const int tileWidth = pixmap.width() / mSize;
  const int tileHeight = pixmap.height() / mSize;
  mTileWidth = tileWidth;
  mTileHeight = tileHeight;

  for ( int row = 0; row < mSize; ++row )
    for ( int col = 0; col < mSize; ++col )
    {
      QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
      item->setPixmap( pixmap.copy( col * tileWidth, row * tileHeight, tileWidth, tileHeight ) );
      mScene->addItem( item ); // takes ownership
      mItems.append( item );

      QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( QString::number( row * mSize + col + 1 ), item );
      const QRectF textRect = textItem->boundingRect();
      textItem->setPos( ( tileWidth - textRect.width() ) / 2, ( tileHeight - textRect.height() ) / 2 );
      textItem->setVisible( false );
      mTextItems.append( textItem );
    }

  // extra lines to show tile split points
  for ( int i = 1; i < mSize; ++i )
  {
    QGraphicsLineItem *lineHorz = new QGraphicsLineItem( 0, i * tileHeight, tileWidth * mSize, i * tileHeight );
    QGraphicsLineItem *lineVert = new QGraphicsLineItem( i * tileWidth, 0, i * tileWidth, tileHeight * mSize );
    lineHorz->setZValue( 10 );
    lineVert->setZValue( 10 );
    mScene->addItem( lineHorz );
    mScene->addItem( lineVert );
  }

  // initialize positions
  for ( int i = 0; i < mSize * mSize; ++i )
    mPositions << i;
  mPositions[mSize * mSize - 1] = -1;
  shuffle( mPositions, 1000 );

  mItems[mSize * mSize - 1]->setVisible( false ); // hide item for the missing piece

  updateTilePositions();

  setScene( mScene );
  return true;
}

void QgsPuzzleWidget::updateTilePositions()
{
  for ( int i = 0; i < mSize * mSize; ++i )
  {
    const int itemIndex = mPositions[i];
    if ( itemIndex == -1 )
      continue;  // empty tile

    int r = i / mSize, c = i % mSize;
    int x = c * mTileWidth, y = r * mTileHeight;
    mItems[itemIndex]->setPos( x, y );
  }
}
