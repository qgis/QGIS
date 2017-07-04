/***************************************************************************
                             qgslayoutview.h
                             ---------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTVIEW_H
#define QGSLAYOUTVIEW_H

#include <QGraphicsView>
#include "qgis.h"
#include "qgsprevieweffect.h" // for QgsPreviewEffect::PreviewMode
#include "qgis_gui.h"

class QgsLayout;

/**
 * \ingroup gui
 * A graphical widget to display and interact with QgsLayouts.
 *
 * QgsLayoutView manages the layout interaction tools and mouse/key events.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutView: public QGraphicsView
{

    Q_OBJECT

    Q_PROPERTY( QgsLayout *currentLayout READ currentLayout WRITE setCurrentLayout NOTIFY layoutSet )

  public:

    //! Current view tool
    enum Tool
    {
      ToolSelect = 0,      //!< Select/move/resize item tool
      ToolAddItem, //!< Add new item tool
    };

    /**
     * Constructor for QgsLayoutView.
     */
    QgsLayoutView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current layout associated with the view.
     * \see setCurrentLayout()
     * \see layoutSet()
     */
    QgsLayout *currentLayout();

    /**
     * Sets the current \a layout to edit in the view.
     * \see currentLayout()
     * \see layoutSet()
     */
    void setCurrentLayout( QgsLayout *layout SIP_KEEPREFERENCE );

  signals:

    /**
     * Emitted when a \a layout is set for the view.
     * \see currentLayout()
     * \see setCurrentLayout()
     */
    void layoutSet( QgsLayout *layout );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;

};

#endif // QGSLAYOUTVIEW_H
