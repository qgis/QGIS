/***************************************************************************
    qgssinglesymbolrendererwidget.h
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
#ifndef QGSSINGLESYMBOLRENDERERWIDGET_H
#define QGSSINGLESYMBOLRENDERERWIDGET_H

#include "qgsrendererwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsSingleSymbolRenderer;
class QgsSymbolSelectorWidget;

class QMenu;

/**
 * \ingroup gui
 * \class QgsSingleSymbolRendererWidget
 */
class GUI_EXPORT QgsSingleSymbolRendererWidget : public QgsRendererWidget
{
    Q_OBJECT

  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    QgsSingleSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsSingleSymbolRendererWidget() override;

    QgsFeatureRenderer *renderer() override;

    void setContext( const QgsSymbolWidgetContext &context ) override;
    void setDockMode( bool dockMode ) override;
    void disableSymbolLevels() override SIP_SKIP;

  protected:
    void setSymbolLevels( const QList<QgsLegendSymbolItem> &levels, bool enabled ) override;

  private slots:
    void changeSingleSymbol();

    void showSymbolLevels();

    void dataDefinedSizeLegend();

  private:
    std::unique_ptr<QgsSingleSymbolRenderer> mRenderer;
    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr<QgsSymbol> mSingleSymbol;
    QAction *mActionLevels = nullptr;
};


#endif // QGSSINGLESYMBOLRENDERERWIDGET_H
