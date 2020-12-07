/***************************************************************************
                             qgslayoutviewtooltemporarymousepan.h
                             ------------------------------------
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

#ifndef QGSLAYOUTVIEWTOOLTEMPORARYMOUSEPAN_H
#define QGSLAYOUTVIEWTOOLTEMPORARYMOUSEPAN_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

/**
 * \ingroup gui
 * Layout view tool for temporarily panning a layout while a mouse button is depressed.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolTemporaryMousePan : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolTemporaryMousePan.
     */
    QgsLayoutViewToolTemporaryMousePan( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void activate() override;

  private:

    QPoint mLastMousePos;
    QPointer< QgsLayoutViewTool > mPreviousViewTool;

};

#endif // QGSLAYOUTVIEWTOOLTEMPORARYMOUSEPAN_H
