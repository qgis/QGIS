/***************************************************************************
                          qgscollapsiblegroupbox.h
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLLAPSIBLEGROUPBOX_H
#define QGSCOLLAPSIBLEGROUPBOX_H

#include "qgisgui.h"

/** \ingroup gui
 * A groupbox that collapses/expands when toggled and draws an expand/collapse icon in lieu of checkbox.
 * Widget must be checkable for expand/collapse icon to appear.
 */

#include <QGroupBox>

class GUI_EXPORT QgsCollapsibleGroupBox : public QGroupBox
{
    Q_OBJECT

  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0 );

    bool isCollapsed() const { return mCollapsed; }

  signals:
    void collapsed( QWidget* );
    void expanded( QWidget* );

  public slots:
    void setToggled( bool toggled ) { setCollapsed( ! toggled ); }
    void setCollapsed( bool collapse );

  protected:
    void paintEvent( QPaintEvent * );
    void showEvent( QShowEvent * event );

  private:
    bool mCollapsed;
    QMargins mMargins;
    QList< QWidget* > mHiddenWidgets;
};

#endif
