/***************************************************************************
    qgstiledscenerendererwidget.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTILEDSCENERENDERERWIDGET_H
#define QGSTILEDSCENERENDERERWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QStackedWidget>
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"

class QgsTiledSceneLayer;
class QgsStyle;
class QgsTiledSceneRenderer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief Base class for tiled scene 2D renderer settings widgets.
 *
 * \since QGIS 3.34
*/
class GUI_EXPORT QgsTiledSceneRendererWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsTiledSceneRendererWidget, associated with the
     * specified \a layer and \a style database.
     */
    QgsTiledSceneRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style );

    /**
     * Returns a new instance of a renderer as defined by the settings in the widget.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsTiledSceneRenderer *renderer() = 0 SIP_FACTORY;

    /**
     * Sets the \a context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.
     * \see context()
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the tiled scene layer associated with the widget.
     */
    const QgsTiledSceneLayer *layer() const { return mLayer; }

  signals:

  protected:
    QgsTiledSceneLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};

#endif // QGSTILEDSCENERENDERERWIDGET_H
