/***************************************************************************
                             qgsmodelviewtoolpan.h
                             ----------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELVIEWTOOLPAN_H
#define QGSMODELVIEWTOOLPAN_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model designer view tool for panning a model.
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsModelViewToolPan : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolPan.
     */
    QgsModelViewToolPan( QgsModelGraphicsView *view SIP_TRANSFERTHIS );

    void modelPressEvent( QgsModelViewMouseEvent *event ) override;
    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void modelReleaseEvent( QgsModelViewMouseEvent *event ) override;
    void deactivate() override;

  private:
    bool mIsPanning = false;
    QPoint mLastMousePos;
    //! Start position for mouse press
    QPoint mMousePressStartPos;
};
#endif // QGSMODELVIEWTOOLPAN_H
