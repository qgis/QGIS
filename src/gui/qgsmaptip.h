/***************************************************************************
    qgsmaptip.h  -  Query a layer and show a maptip on the canvas
    ---------------------
    begin                : October 2007
    copyright            : (C) 2007 by Gary Sherman
    email                : sherman @ mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTIP_H
#define QGSMAPTIP_H

class QgsMapLayer;
class QgsMapCanvas;
class QPoint;
class QString;
class QgsPointXY;
class QgsVectorLayer;
class QgsWebView;

#include <QWidget>
#include <QUrl>
#include <QTimer>
#include "qgsfeature.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief A maptip is a class to display a tip on a map canvas
 * when a mouse is hovered over a feature.
 *
 * Since QGIS 2.16 a maptip can show full html.
 * QgsMapTip is a QgsWebView, so you can load full HTML/JS/CSS in it.
 *
 * The code found in the map tips tab is inserted in a inline-block div
 * so the frame can be resized based on the content size.
 *
 * If no element in the html has a width attribute, the frame will squeeze down
 * to the widest word. To avoid this you can wrap your HTML in a
 * div style="width:300px" or similar.
 *
 * JS can be included using the script tag as usual, while CSS files must be
 * linked using link rel="stylesheet" href="URL.css" the html specs
 * discourages link rel="stylesheet" in the body, but all browsers allow it.
 * see https://jakearchibald.com/2016/link-in-body/
 */
class GUI_EXPORT QgsMapTip : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Default constructor
     */
    QgsMapTip();

    /**
     * Show a maptip at a given point on the map canvas
     * \param thepLayer a qgis vector map layer pointer that will
     *        be used to provide the attribute data for the map tip.
     * \param mapPosition a reference to the position of the cursor
     *        in map coordinatess.
     * \param pixelPosition a reference to the position of the cursor
     *        in pixel coordinates.
     * \param mpMapCanvas a map canvas on which the tip is drawn
     */
    void showMapTip( QgsMapLayer *thepLayer, QgsPointXY &mapPosition, const QPoint &pixelPosition, QgsMapCanvas *mpMapCanvas );

    /**
     * Clear the current maptip if it exists
     * \param mpMapCanvas the canvas from which the tip should be cleared.
     * \param msDelay optional time in ms to defer clearing the maptip (since QGIS 3.26)
     */
    void clear( QgsMapCanvas *mpMapCanvas = nullptr, int msDelay = 0 );

    /**
     * Returns the html that would be displayed in a maptip for a given layer. If the layer has features, the first feature is used
     * to evaluate the expressions.
     * \since QGIS 3.32
     */
    static QString vectorMapTipPreviewText( QgsMapLayer *layer, QgsMapCanvas *mapCanvas, const QString &mapTemplate, const QString &displayExpression );

    /**
     * Returns the html that would be displayed in a maptip for a given layer. The center pixel of the raster is used to
     * evaluate the expressions.
     * \since QGIS 3.32
     */
    static QString rasterMapTipPreviewText( QgsMapLayer *layer, QgsMapCanvas *mapCanvas, const QString &mapTemplate );

  private slots:
    void onLinkClicked( const QUrl &url );
    void resizeAndMoveToolTip();

  private:
    // Fetch the feature to use for the maptip text.
    // Only the first feature in the search radius is used
    QString fetchFeature( QgsMapLayer *thepLayer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas );

    // Sample the raster and get the maptip text
    QString fetchRaster( QgsMapLayer *layer, QgsPointXY &mapPosition, QgsMapCanvas *mapCanvas );

    // Insert the raw map tip text into an HTML template and return the result
    static QString htmlText( const QString &text, int maxWidth = -1 );

    // Flag to indicate if a maptip is currently being displayed
    bool mMapTipVisible;

    QgsWebView *mWebView = nullptr;

    static const int MARGIN_VALUE = 5;

    QTimer mDelayedClearTimer;

    // Template for the actual HTML content that will be displayed in QgsWebView
    static const QString sMapTipTemplate;

    QPoint mPosition;
    const QgsMapCanvas *mMapCanvas = nullptr;
};
#endif // QGSMAPTIP_H
