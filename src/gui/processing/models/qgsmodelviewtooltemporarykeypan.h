/***************************************************************************
                             qgsmodelviewtooltemporarykeypan.h
                             ----------------------------------
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

#ifndef QGSMODELVIEWTOOLTEMPORARYKEYPAN_H
#define QGSMODELVIEWTOOLTEMPORARYKEYPAN_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model designer view tool for temporarily panning a layout while a key is depressed.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewToolTemporaryKeyPan : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolTemporaryKeyPan.
     */
    QgsModelViewToolTemporaryKeyPan( QgsModelGraphicsView *view SIP_TRANSFERTHIS );

    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void activate() override;

  private:
    QPoint mLastMousePos;
    QPointer<QgsModelViewTool> mPreviousViewTool;
};
#endif // QGSMODELVIEWTOOLTEMPORARYKEYPAN_H
