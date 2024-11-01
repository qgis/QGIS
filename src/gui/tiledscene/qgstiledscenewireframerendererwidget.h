/***************************************************************************
    qgstiledscenewireframerendererwidget.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENEWIREFRAMERENDERERWIDGET_H
#define QGSTILEDSCENEWIREFRAMERENDERERWIDGET_H

#include "qgstiledscenerendererwidget.h"
#include "ui_qgstiledscenewireframerendererwidgetbase.h"
#include "qgis_gui.h"

class QgsTiledSceneLayer;
class QgsStyle;
class QgsTiledSceneRenderer;

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsTiledSceneWireframeRendererWidget : public QgsTiledSceneRendererWidget, private Ui::QgsTiledSceneWireframeRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsTiledSceneWireframeRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style );
    static QgsTiledSceneRendererWidget *create( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer * );

    QgsTiledSceneRenderer *renderer() override;

  private slots:

    void emitWidgetChanged();

  private:
    void setFromRenderer( const QgsTiledSceneRenderer *r );

    bool mBlockChangedSignal = false;
};

///@endcond

#endif // QGSTILEDSCENEWIREFRAMERENDERERWIDGET_H
