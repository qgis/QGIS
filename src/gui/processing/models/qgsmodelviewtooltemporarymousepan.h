/***************************************************************************
                             qgsmodelviewtooltemporarymousepan.h
                             ------------------------------------
    Date                 : March 2020
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

#ifndef QgsModelViewToolTemporaryMousePan_H
#define QgsModelViewToolTemporaryMousePan_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model view tool for temporarily panning a model while a mouse button is depressed.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewToolTemporaryMousePan : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolTemporaryMousePan.
     */
    QgsModelViewToolTemporaryMousePan( QgsModelGraphicsView *view SIP_TRANSFERTHIS );

    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void modelReleaseEvent( QgsModelViewMouseEvent *event ) override;
    void activate() override;

  private:
    QPoint mLastMousePos;
    QPointer<QgsModelViewTool> mPreviousViewTool;
};

#endif // QgsModelViewToolTemporaryMousePan_H
