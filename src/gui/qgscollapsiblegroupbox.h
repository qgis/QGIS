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

#include <QGroupBox>
#include <QSettings>
#include <QPointer>
#include <QToolButton>
#include <QMouseEvent>

class QToolButton;
class QScrollArea;

class QgsGroupBoxCollapseButton: public QToolButton
{
    Q_OBJECT

  public:
    QgsGroupBoxCollapseButton( QWidget *parent = 0 )
        : QToolButton( parent ), mAltDown( false ), mShiftDown( false ) {}

    ~QgsGroupBoxCollapseButton() {}

    bool altDown() const { return mAltDown; }
    void setAltDown( bool updown ) { mAltDown = updown; }

    bool shiftDown() const { return mShiftDown; }
    void setShiftDown( bool shiftdown ) { mShiftDown = shiftdown; }

  protected:
    void mouseReleaseEvent( QMouseEvent *event )
    {
      mAltDown = ( event->modifiers() & ( Qt::AltModifier | Qt::ControlModifier ) );
      mShiftDown = ( event->modifiers() & Qt::ShiftModifier );
      QToolButton::mouseReleaseEvent( event );
    }

  private:
    bool mAltDown;
    bool mShiftDown;
};

/** \ingroup gui
 * \class QgsCollapsibleGroupBoxBasic
 * A groupbox that collapses/expands when toggled.
 * Basic class QgsCollapsibleGroupBoxBasic does not auto-save collapsed or checked state
 * Holding Alt modifier key when toggling collapsed state will synchronize the toggling across other collapsible group boxes with the same syncGroup QString value
 * Holding Shift modifier key when attempting to toggle collapsed state will expand current group box, then collapse any others with the same syncGroup QString value
 * @note To add Collapsible properties in promoted QtDesigner widgets, you can add the following "Dynamic properties" by clicking on the green + in the propreties palette:
 * bool collapsed, QString syncGroup
 */

class GUI_EXPORT QgsCollapsibleGroupBoxBasic : public QGroupBox
{
    Q_OBJECT

    Q_PROPERTY( bool collapsed READ isCollapsed WRITE setCollapsed USER true )
    Q_PROPERTY( QString syncGroup READ syncGroup WRITE setSyncGroup )

  public:
    QgsCollapsibleGroupBoxBasic( QWidget *parent = 0 );
    QgsCollapsibleGroupBoxBasic( const QString &title, QWidget *parent = 0 );
    ~QgsCollapsibleGroupBoxBasic();

    bool isCollapsed() const { return mCollapsed; }
    void setCollapsed( bool collapse );

    /** Named group which synchronizes collapsing action when triangle is clicked while holding alt modifier key */
    QString syncGroup() const { return mSyncGroup; }
    void setSyncGroup( QString grp );

    //! set this to false to not automatically scroll parent QScrollArea to this widget's contents when expanded
    void setScrollOnExpand( bool scroll ) { mScrollOnExpand = scroll; }

  signals:
    /** Signal emitted when groupbox collapsed/expanded state is changed, and when first shown */
    void collapsedStateChanged( bool collapsed );

  public slots:
    void checkToggled( bool ckd );
    void toggleCollapsed();

  protected:
    void init();

    /** Visual fixes for when group box is collapsed/expanded */
    void collapseExpandFixes();

    void showEvent( QShowEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void changeEvent( QEvent *event );

    void updateStyle();
    QRect titleRect() const;
    void clearModifiers();

    bool mCollapsed;
    bool mInitFlat;
    bool mInitFlatChecked;
    bool mScrollOnExpand;
    bool mShown;
    QScrollArea* mParentScrollArea;
    QgsGroupBoxCollapseButton* mCollapseButton;
    QWidget* mSyncParent;
    QString mSyncGroup;
    bool mAltDown;
    bool mShiftDown;
    bool mTitleClicked;

    static QIcon mCollapseIcon;
    static QIcon mExpandIcon;
};

/** \ingroup gui
 * \class QgsCollapsibleGroupBox
 * A groupbox that collapses/expands when toggled and can save its collapsed and checked states.
 * By default, it auto-saves only its collapsed state to the global settings based on the widget and it's parent names.
 * Holding Alt modifier key when toggling collapsed state will synchronize the toggling across other collapsible group boxes with the same syncGroup QString value
 * Holding Shift modifier key when attempting to toggle collapsed state will expand current group box, then collapse any others with the same syncGroup QString value
 * @see basic class QgsCollapsibleGroupBoxBasic which does not auto-save states
 * @note To add Collapsible properties in promoted QtDesigner widgets, you can add the following "Dynamic properties" by clicking on the green + in the propreties palette:
 * bool collapsed, bool saveCollapsedState, bool saveCheckedState, QString syncGroup
 */

class GUI_EXPORT QgsCollapsibleGroupBox : public QgsCollapsibleGroupBoxBasic
{
    Q_OBJECT

    Q_PROPERTY( bool collapsed READ isCollapsed WRITE setCollapsed USER true )
    Q_PROPERTY( bool saveCollapsedState READ saveCollapsedState WRITE setSaveCollapsedState )
    Q_PROPERTY( bool saveCheckedState READ saveCheckedState WRITE setSaveCheckedState )
    Q_PROPERTY( QString syncGroup READ syncGroup WRITE setSyncGroup )

  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0, QSettings* settings = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0, QSettings* settings = 0 );
    ~QgsCollapsibleGroupBox();

    // set custom QSettings pointer if group box was already created from QtDesigner promotion
    void setSettings( QSettings* settings );

    //! set this to false to not save/restore collapsed state
    void setSaveCollapsedState( bool save ) { mSaveCollapsedState = save; }
    /** set this to true to save/restore checked state
     * @note only turn on mSaveCheckedState for groupboxes NOT used
     * in multiple places or used as options for different parent objects */
    void setSaveCheckedState( bool save ) { mSaveCheckedState = save; }
    bool saveCollapsedState() { return mSaveCollapsedState; }
    bool saveCheckedState() { return mSaveCheckedState; }

    //! set this to a defined string to share save/restore states across different parent dialogs
    void setSettingGroup( const QString &group ) { mSettingGroup = group; }
    QString settingGroup() const { return mSettingGroup; }

  protected slots:
    void loadState();
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    QString saveKey() const;

    // pointer to app or custom, external QSettings
    // QPointer in case custom settings obj gets deleted while groupbox's dialog is open
    QPointer<QSettings> mSettings;
    bool mDelSettings;

    bool mSaveCollapsedState;
    bool mSaveCheckedState;
    QString mSettingGroup;
};

#endif
