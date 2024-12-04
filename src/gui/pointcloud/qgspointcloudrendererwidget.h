/***************************************************************************
    qgspointcloudrendererwidget.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTCLOUDRENDERERWIDGET_H
#define QGSPOINTCLOUDRENDERERWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QStackedWidget>
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"

class QgsPointCloudLayer;
class QgsStyle;
class QgsPointCloudRenderer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief Base class for point cloud 2D renderer settings widgets.
 *
 * \since QGIS 3.18
*/
class GUI_EXPORT QgsPointCloudRendererWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsPointCloudRendererWidget, associated with the
     * specified \a layer and \a style database.
     */
    QgsPointCloudRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style );

    /**
     * Returns a new instance of a renderer as defined by the settings in the widget.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsPointCloudRenderer *renderer() = 0 SIP_FACTORY;

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
     * Returns the point cloud layer associated with the widget.
     */
    const QgsPointCloudLayer *layer() const { return mLayer; }

  signals:

  protected:
    QgsPointCloudLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};

#endif // QGSPOINTCLOUDRENDERERWIDGET_H
