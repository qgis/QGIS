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
class QgsPoint;
class QgsVectorLayer;
class QgsWebView;

#include <QWidget>
#include <QUrl>
#include "qgsfeature.h"

/** \ingroup gui
 * A maptip is a class to display a tip on a map canvas
 * when a mouse is hovered over a feature.
 *
 * Since QGIS 2.16 a maptip can show full html.
 * QgsMapTip is a QgsWebview, so you can load full HTML/JS/CSS in it.
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
class GUI_EXPORT QgsMapTip: public QWidget
{
    Q_OBJECT
  public:
    /** Default constructor
     */
    QgsMapTip();
    /** Destructor
     */
    virtual ~QgsMapTip();
    /** Show a maptip at a given point on the map canvas
     * @param thepLayer a qgis vector map layer pointer that will
     *        be used to provide the attribute data for the map tip.
     * @param theMapPosition a reference to the position of the cursor
     *        in map coordinatess.
     * @param thePixelPosition a reference to the position of the cursor
     *        in pixel coordinates.
     * @param mpMapCanvas a map canvas on which the tip is drawn
     */
    void showMapTip( QgsMapLayer * thepLayer,
                     QgsPoint & theMapPosition,
                     QPoint & thePixelPosition,
                     QgsMapCanvas *mpMapCanvas );
    /** Clear the current maptip if it exists
     * @param mpMapCanvas the canvas from which the tip should be cleared.
     */
    void clear( QgsMapCanvas *mpMapCanvas = nullptr );
  private:
    // Fetch the feature to use for the maptip text.
    // Only the first feature in the search radius is used
    QString fetchFeature( QgsMapLayer * thepLayer,
                          QgsPoint & theMapPosition,
                          QgsMapCanvas *thepMapCanvas );

    QString replaceText(
      QString displayText, QgsVectorLayer *layer, QgsFeature &feat );

    // Flag to indicate if a maptip is currently being displayed
    bool mMapTipVisible;

    QWidget* mWidget;
    QgsWebView* mWebView;

  private slots:
    void onLinkClicked( const QUrl& url );
};
#endif // QGSMAPTIP_H
