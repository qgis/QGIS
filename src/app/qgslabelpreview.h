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

#include "qgspallabeling.h"

#include <QLabel>

class QgsRenderContext;

class APP_EXPORT QgsLabelPreview : public QLabel
{
  public:
    QgsLabelPreview( QWidget* parent = NULL );
    ~QgsLabelPreview();

    void setTextColor( QColor color );

    void setBuffer( double size, QColor color, Qt::PenJoinStyle joinStyle, bool noFill = false );

    void setFont( QFont f ) { mFont = f; }
    QFont font() { return mFont; }

    void paintEvent( QPaintEvent* e ) override;

  private:
    QgsPalLayerSettings* mTmpLyr;
    QColor mTextColor;
    QFont mFont;

    // device-based render context
    QgsRenderContext* mContext;
};

#endif // LABELPREVIEW_H
