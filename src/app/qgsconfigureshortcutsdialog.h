/***************************************************************************
    qgsconfigureshortcutsdialog.h
    ---------------------
    begin                : May 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONFIGURESHORTCUTSDIALOG_H
#define QGSCONFIGURESHORTCUTSDIALOG_H

#include <QDialog>

#include "ui_qgsconfigureshortcutsdialog.h"

class APP_EXPORT QgsConfigureShortcutsDialog : public QDialog, private Ui::QgsConfigureShortcutsDialog
{
    Q_OBJECT

  public:
    QgsConfigureShortcutsDialog( QWidget* parent = NULL );
    ~QgsConfigureShortcutsDialog();

    void populateActions();

  protected:
    void keyPressEvent( QKeyEvent * event );
    void keyReleaseEvent( QKeyEvent * event );

    QAction* currentAction();

    void setGettingShortcut( bool getting );
    void setCurrentActionShortcut( QKeySequence s );
    void updateShortcutText();

  public slots:
    void changeShortcut();
    void resetShortcut();
    void setNoShortcut();
    void saveShortcuts();
    void loadShortcuts();

    void actionChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous );

  protected:
    bool mGettingShortcut;
    int mModifiers, mKey;

  private:
    /*!
     * Function to save dialog window state
     */
    void saveState();

    /*!
     * Function to restore dialog window state
     */
    void restoreState();

};

#endif
