/***************************************************************************
                          qgsplottoolpan.h
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

#ifndef QGSPLOTTOOLPAN_H
#define QGSPLOTTOOLPAN_H

#include "qgsplottool.h"
#include "qgis_gui.h"
#include "qgis_sip.h"


/**
 * \ingroup gui
 * \brief Plot tool for panning/zoom/navigating plots.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotToolPan : public QgsPlotTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotToolPan, with the associated \a canvas.
     */
    QgsPlotToolPan( QgsPlotCanvas *canvas );

    Qgis::PlotToolFlags flags() const override;
    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void plotPressEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;
    void deactivate() override;

  private:

    bool mIsPanning = false;
    QPoint mLastMousePos;
    //! Start position for mouse press
    QPoint mMousePressStartPos;

};

#endif // QGSPLOTTOOLPAN_H
