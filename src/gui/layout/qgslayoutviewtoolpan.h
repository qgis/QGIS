/***************************************************************************
                             qgslayoutviewtoolpan.h
                             ----------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVIEWTOOLPAN_H
#define QGSLAYOUTVIEWTOOLPAN_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

/**
 * \ingroup gui
 * Layout view tool for panning the layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolPan : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolPan.
     */
    QgsLayoutViewToolPan( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void deactivate() override;

  private:

    bool mIsPanning = false;
    QPoint mLastMousePos;
    //! Start position for mouse press
    QPoint mMousePressStartPos;

};

#endif // QGSLAYOUTVIEWTOOLPAN_H
