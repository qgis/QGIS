/***************************************************************************
                             qgslayoutviewtooltemporarykeyzoom.h
                             -----------------------------------
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

#ifndef QGSLAYOUTVIEWTOOLTEMPORARYKEYZOOM_H
#define QGSLAYOUTVIEWTOOLTEMPORARYKEYZOOM_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtoolzoom.h"
#include "qgslayoutviewrubberband.h"
#include <memory>

/**
 * \ingroup gui
 * Layout view tool for temporarily zooming a layout while a key is depressed.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolTemporaryKeyZoom : public QgsLayoutViewToolZoom
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolTemporaryKeyZoom.
     */
    QgsLayoutViewToolTemporaryKeyZoom( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void activate() override;

  private:

    QPointer< QgsLayoutViewTool > mPreviousViewTool;

    bool mDeactivateOnMouseRelease = false;

    void updateCursor( Qt::KeyboardModifiers modifiers );
};

#endif // QGSLAYOUTVIEWTOOLTEMPORARYKEYZOOM_H
