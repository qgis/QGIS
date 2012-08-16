/***************************************************************************
    qgslabelpreview.h
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELPREVIEW_H
#define QGSLABELPREVIEW_H

#include <QLabel>

class QgsLabelPreview : public QLabel
{
  public:
    QgsLabelPreview( QWidget* parent = NULL );

    void setTextColor( QColor color );

    void setBuffer( double size, QColor color, Qt::PenJoinStyle joinStyle, bool noFill = false );

    void setFont( QFont f ) { mFont = f; }
    QFont font() { return mFont; }

    void paintEvent( QPaintEvent* e );

  private:
    double mBufferSize;
    QColor mBufferColor;
    Qt::PenJoinStyle mBufferJoinStyle;
    bool mBufferNoFill;
    QColor mTextColor;
    QFont mFont;
};

#endif // LABELPREVIEW_H
