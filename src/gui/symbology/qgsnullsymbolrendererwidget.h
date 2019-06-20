/***************************************************************************
    qgsnullsymbolrendererwidget.h
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNULLSYMBOLRENDERERWIDGET_H
#define QGSNULLSYMBOLRENDERERWIDGET_H

#include "qgsrendererwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsNullSymbolRenderer;

class QMenu;

/**
 * \ingroup gui
 * \class QgsNullSymbolRendererWidget
 * \brief Blank widget for customizing QgsNullSymbolRenderer.
 * \since QGIS 2.16
 */

class GUI_EXPORT QgsNullSymbolRendererWidget : public QgsRendererWidget
{
    Q_OBJECT

  public:

    //! Creates a new QgsNullSymbolRendererWidget object
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    //! Constructor for QgsNullSymbolRendererWidget
    QgsNullSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsNullSymbolRendererWidget() override;

    //! Returns a pointer to the configured renderer
    QgsFeatureRenderer *renderer() override;

  protected:

    //! Renderer being configured by the widget
    QgsNullSymbolRenderer *mRenderer = nullptr;

};


#endif // QGSNULLSYMBOLRENDERERWIDGET_H
