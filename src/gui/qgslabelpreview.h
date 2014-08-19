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

#include "qgstextrenderer.h"

#include <QLabel>

class QgsRenderContext;

class GUI_EXPORT QgsLabelPreview : public QLabel
{
  public:
    QgsLabelPreview( QWidget* parent = NULL );
    ~QgsLabelPreview();

    void paintEvent( QPaintEvent* e );

    void setTextRendererSettings( const QgsTextRendererSettings &textSettings );

    void setMapUnitScale( const double scale );

  private:
    QgsTextRendererSettings mTextSettings;

    // device-based render context
    QgsRenderContext* mContext;
};

#endif // LABELPREVIEW_H
