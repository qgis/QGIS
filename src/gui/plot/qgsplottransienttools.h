/***************************************************************************
                          qgsplottransienttools.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#ifndef QGSPLOTTRANSIENTTOOLS_H
#define QGSPLOTTRANSIENTTOOLS_H

#include "qgsplottool.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsplottoolzoom.h"

/**
 * \ingroup gui
 * \brief Plot tool for temporarily panning a plot while a key is depressed.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolTemporaryKeyPan : public QgsPlotTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolTemporaryKeyPan.
     */
    QgsPlotToolTemporaryKeyPan( QgsPlotCanvas *canvas SIP_TRANSFERTHIS );

    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void activate() override;

  private:

    QPoint mLastMousePos;
    QPointer< QgsPlotTool > mPreviousTool;

};

/**
 * \ingroup gui
 * \brief Plot tool for temporarily panning a plot while a mouse button is depressed.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolTemporaryMousePan : public QgsPlotTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolTemporaryMousePan.
     */
    QgsPlotToolTemporaryMousePan( QgsPlotCanvas *canvas SIP_TRANSFERTHIS );

    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;
    void activate() override;

  private:

    QPoint mLastMousePos;
    QPointer< QgsPlotTool > mPreviousTool;

};

/**
 * \ingroup gui
 * \brief Plot tool for temporarily zooming a plot while a key is depressed.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolTemporaryKeyZoom : public QgsPlotToolZoom
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolTemporaryKeyZoom.
     */
    QgsPlotToolTemporaryKeyZoom( QgsPlotCanvas *canvas SIP_TRANSFERTHIS );

    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void activate() override;

  private:

    QPointer< QgsPlotTool > mPreviousViewTool;

    bool mDeactivateOnMouseRelease = false;

    void updateCursor( Qt::KeyboardModifiers modifiers );
};

#endif // QGSPLOTTRANSIENTTOOLS_H
