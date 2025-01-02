/***************************************************************************
                             qgslayoutviewtooltemporarykeypan.h
                             ----------------------------------
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

#ifndef QGSLAYOUTVIEWTOOLTEMPORARYKEYPAN_H
#define QGSLAYOUTVIEWTOOLTEMPORARYKEYPAN_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

/**
 * \ingroup gui
 * \brief Layout view tool for temporarily panning a layout while a key is depressed.
 */
class GUI_EXPORT QgsLayoutViewToolTemporaryKeyPan : public QgsLayoutViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLayoutViewToolTemporaryKeyPan.
     */
    QgsLayoutViewToolTemporaryKeyPan( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void activate() override;

  private:
    QPoint mLastMousePos;
    QPointer<QgsLayoutViewTool> mPreviousViewTool;
};

#endif // QGSLAYOUTVIEWTOOLTEMPORARYKEYPAN_H
