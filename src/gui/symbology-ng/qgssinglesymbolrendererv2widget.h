/***************************************************************************
    qgssinglesymbolrendererv2widget.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSINGLESYMBOLRENDERERV2WIDGET_H
#define QGSSINGLESYMBOLRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

class QgsSingleSymbolRendererV2;
class QgsSymbolV2SelectorDialog;

class QMenu;

class GUI_EXPORT QgsSingleSymbolRendererV2Widget : public QgsRendererV2Widget
{
    Q_OBJECT

  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsSingleSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsSingleSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer() override;

  public slots:
    void changeSingleSymbol();

    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

  protected:

    QgsSingleSymbolRendererV2* mRenderer;
    QgsSymbolV2SelectorDialog* mSelector;
    QgsSymbolV2* mSingleSymbol;
};


#endif // QGSSINGLESYMBOLRENDERERV2WIDGET_H
