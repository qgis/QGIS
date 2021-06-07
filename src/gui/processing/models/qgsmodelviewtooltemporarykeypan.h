/***************************************************************************
                             qgsmodelviewtooltemporarykeypan.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
 * Model designer view tool for temporarily panning a layout while a key is depressed.
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
    QPointer< QgsModelViewTool > mPreviousViewTool;

};
#endif // QGSMODELVIEWTOOLTEMPORARYKEYPAN_H
