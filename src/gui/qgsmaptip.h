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
#ifndef _QGSMAPTIP_H_
#define _QGSMAPTIP_H_

class QgsMapLayer;
class QgsMapCanvas;
class QPoint;
class QString;

#include "qgsfeature.h"

/** \ingroup gui
 * A maptip is a class to display a tip on a map canvas
 * when a mouse is hovered over a feature.
 */
class GUI_EXPORT QgsMapTip
{
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
    void clear( QgsMapCanvas *mpMapCanvas );
  private:
    // Fetch the feature to use for the maptip text. Only the first feature in the
    // search radius is used
    QString fetchFeature( QgsMapLayer * thepLayer,
                          QgsPoint & theMapPosition,
                          QgsMapCanvas *thepMapCanvas );

    QString replaceText( QString displayText, QgsVectorLayer *layer, QgsFeature &feat );

    // Flag to indicate if a maptip is currently being displayed
    bool mMapTipVisible;
    // Last point on the map canvas when the maptip timer fired. This point is in widget pixel
    // coordinates
    QPoint mLastPosition;

};
#endif // _QGSMAPTIP_H_
