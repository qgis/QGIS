/***************************************************************************
    QgsPersistentMenu.h  -  menu to be used for toolbars
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Till Frankenbach
    email                : till.frankenbach@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QgsPersistentMenu_H
#define QgsPersistentMenu_H

#include "qgis_gui.h"
#include "qgsexpressioncontext.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmaptoolidentify.h"

#include <QMenu>

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Builds a menu to be used with toolbars.
 *
 * Menu doesn't close when an action is triggered, and allows triggering multiple actions in a row.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsPersistentMenu : public QMenu
{
    Q_OBJECT

  public:
    /**
     * \brief QgsPersistentMenu is a menu to be used with toolbars.
     */
    explicit QgsPersistentMenu( const QString &title, QWidget *parent = nullptr );

    void mouseReleaseEvent( QMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

    ~QgsPersistentMenu() override;
};

#endif // QgsPersistentMenu_H
