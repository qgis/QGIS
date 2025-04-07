/***************************************************************************
                             qgsmodelviewtoollink.h
                             ----------------------------------
    Date                 : January 2024
    Copyright            : (C) 2024 Valentin Buira
    Email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELVIEWTOOLLINK_H
#define QGSMODELVIEWTOOLLINK_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewrubberband.h"
#include "qgsmodelgraphicitem.h"
#include <memory>

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model designer view tool for linking socket together
 * This tool is not exposed in the UI and is only set when the select tool click on a socket 
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsModelViewToolLink : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolLink.
     */
    QgsModelViewToolLink( QgsModelGraphicsView *view SIP_TRANSFERTHIS );

    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void modelReleaseEvent( QgsModelViewMouseEvent *event ) override;
    bool allowItemInteraction() override;
    void activate() override;
    void deactivate() override;

    /**
     * Set the from socket
     * 
     * In the case the user started dragging from an already linked input socket 
     * we need to figure out, which is the output socket used as the source at the other side of the link.
     * 
     * This is used when the user disconnects a a link or relinks to another input socket 
     */
    void setFromSocket( QgsModelDesignerSocketGraphicItem *socket );

  signals:
    /**
     * Emitted when a change was made to the model that requires a full rebuild of the scene.
     */
    void requestRebuildRequired();

  private:
    std::unique_ptr<QgsModelViewBezierRubberBand> mBezierRubberBand;
    QgsModelDesignerSocketGraphicItem *mFromSocket = nullptr;
    QgsModelDesignerSocketGraphicItem *mToSocket = nullptr;

    QgsModelDesignerSocketGraphicItem *mLastHoveredSocket = nullptr;

    /* Used to return to select tool */
    QPointer<QgsModelViewTool> mPreviousViewTool;
};
#endif // QGSMODELVIEWTOOLLINK_H
