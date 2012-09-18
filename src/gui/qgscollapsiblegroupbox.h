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
class QScrollArea;

class GUI_EXPORT QgsCollapsibleGroupBox : public QGroupBox
{
    Q_OBJECT

  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0 );
    ~QgsCollapsibleGroupBox();

    bool isCollapsed() const { return mCollapsed; }
    void setCollapsed( bool collapse );

    //! set this to false to not save/restore check and collapse state
    void setSaveState( bool save ) { mSaveState = save; }
    //! set this to false to not automatically scroll parent QScrollArea to this widget's contents when expanded
    void setScrollOnExpand( bool scroll ) { mScrollOnExpand = scroll; }

  signals:
    /** Signal emitted when groupbox collapsed/expanded state is changed, and when first shown */
    void collapsedStateChanged( QgsCollapsibleGroupBox* );

  public slots:
    void checkToggled( bool ckd );
    void toggleCollapsed();

  protected slots:
    void loadState();
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void changeEvent( QEvent *event );

    void updateStyle();
    QRect titleRect() const;
    QString saveKey() const;

    bool mCollapsed;
    bool mSaveState;
    bool mInitFlat;
    bool mScrollOnExpand;
    bool mShown;
    QScrollArea* mParentScrollArea;
    QToolButton* mCollapseButton;

    static QIcon mCollapseIcon;
    static QIcon mExpandIcon;
};

#endif
