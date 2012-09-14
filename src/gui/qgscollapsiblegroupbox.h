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
 * A groupbox that collapses/expands when toggled.
 * @note Collapsible function not shown in promoted QtDesigner widgets.
 */

#include <QGroupBox>

class QToolButton;

class GUI_EXPORT QgsCollapsibleGroupBox : public QGroupBox
{
    Q_OBJECT

  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0 );
    ~QgsCollapsibleGroupBox();
    bool isCollapsed() const { return mCollapsed; }
    void setCollapsed( bool collapse );
    void setSaveState( bool save ) { mSaveState = save; }

  signals:
    void collapsedStateChanged( QWidget* );

  public slots:
    void checkToggled( bool ckd );
    void toggleCollapsed();
    void updateStyle();

  protected slots:
    void loadState();
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    QRect titleRect() const;
    QString saveKey() const;

    bool mCollapsed;
    bool mSaveState;
    bool mInitFlat;
    bool mShown;
    QToolButton* mCollapseButton;

    static QIcon mCollapseIcon;
    static QIcon mExpandIcon;
};

#endif
