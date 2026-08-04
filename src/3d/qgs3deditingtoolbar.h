/***************************************************************************
    qgs3deditingtoolbar.h
    -------------------
    begin                : November 2025
    copyright            : (C) 2025 Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DEDITINGTOOLBAR_H
#define QGS3DEDITINGTOOLBAR_H

#include "qgis_3d.h"
#include "qgsmaplayer.h"

#include <QToolBar>

class QAction;

/**
 * Base class to create 3D editing toolbar.
 *
 * Inherited classes:
 *
 * - will be sub widget of the main 3D edtion toolbar Qgs3DMapCanvasWidget::mEditingToolBar
 * - will be visible only when the QGIS active layer is compatible (see accept() function)
 *
 * \since QGIS 3.44
 */
class _3D_EXPORT Qgs3DEditingToolBar : public QToolBar
{
    Q_OBJECT

  public:
    /**
     * Default constructor
     * \param title toolbar title
     * \param parent parent widget
     */
    Qgs3DEditingToolBar( const QString &title, QWidget *parent );

    /**
     * \param layer current active layer
     * \return true if this toolbar is usable with the \a layer
     */
    virtual bool accept( QgsMapLayer *layer ) const = 0;

    /**
     * Called when the current active layer changes and is accepted
     * \param layer current active layer
     */
    virtual void activate( QgsMapLayer *layer ) = 0;

    /**
     * Called when the current active layer changes and is no more accepted
     */
    virtual void deactivate() = 0;

    /**
     * \return the list of actions produced by the toolbar
     */
    virtual QList<QAction *> groupActions() const = 0;

  protected:
    QWidget *mParentWidget;
};

#endif // QGS3DEDITINGTOOLBAR_H
