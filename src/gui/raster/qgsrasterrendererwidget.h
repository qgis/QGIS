/***************************************************************************
                         qgsrasterrendererwidget.h
                         ---------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERERWIDGET_H
#define QGSRASTERRENDERERWIDGET_H

#include "qgsrectangle.h"
#include "qgis.h"

#include <QWidget>
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsRasterRenderer;
class QgsMapCanvas;
class QgsRasterMinMaxWidget;

/**
 * \ingroup gui
 * \class QgsRasterRendererWidget
 *
 * \brief Abstract base class for widgets which configure a QgsRasterRenderer.
 */
class GUI_EXPORT QgsRasterRendererWidget: public QWidget
{
    Q_OBJECT

  public:

    //TODO QGIS 4.0 - remove extent parameter, replace with map canvas parameter
    QgsRasterRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
      : mRasterLayer( layer )
      , mExtent( extent )
    {}

    /**
     * Creates a new renderer, using the properties defined in the widget.
     *
     * The caller takes ownership of the returned renderer.
     */
    virtual QgsRasterRenderer *renderer() = 0 SIP_FACTORY;

    /**
     * Sets the raster \a layer associated with the widget.
     *
     * \see rasterLayer()
     */
    void setRasterLayer( QgsRasterLayer *layer ) { mRasterLayer = layer; }

    /**
     * Returns the raster layer associated with the widget.
     *
     * \see setRasterLayer()
     */
    const QgsRasterLayer *rasterLayer() const { return mRasterLayer; }

    /**
     * Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map extent and other properties from the canvas.
     * \param canvas map canvas
     * \see mapCanvas()
     * \since QGIS 2.16
     */
    virtual void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     * \since QGIS 2.16
     */
    QgsMapCanvas *mapCanvas();

    virtual QString min( int index = 0 ) { Q_UNUSED( index ) return QString(); }
    virtual QString max( int index = 0 ) { Q_UNUSED( index ) return QString(); }
    virtual void setMin( const QString &value, int index = 0 ) { Q_UNUSED( index ) Q_UNUSED( value ); }
    virtual void setMax( const QString &value, int index = 0 ) { Q_UNUSED( index ) Q_UNUSED( value ); }
    virtual QString stdDev() { return QString(); }
    virtual void setStdDev( const QString &value ) { Q_UNUSED( value ) }
    virtual int selectedBand( int index = 0 ) { Q_UNUSED( index ) return -1; }

    //! Load programmatically with current values
    virtual void doComputations() { }

    //! Returns min/max widget when it exists.
    virtual QgsRasterMinMaxWidget *minMaxWidget() { return nullptr; }

  signals:

    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

  protected:
    QgsRasterLayer *mRasterLayer = nullptr;

    //! Current extent
    QgsRectangle mExtent;

    //! Associated map canvas
    QgsMapCanvas *mCanvas = nullptr;
};

#endif // QGSRASTERRENDERERWIDGET_H
