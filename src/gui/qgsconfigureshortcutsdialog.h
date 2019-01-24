/***************************************************************************
    qgsconfigureshortcutsdialog.h
    -----------------------------
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
#include "qgis_sip.h"

#include "ui_qgsconfigureshortcutsdialog.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QShortcut;
class QgsShortcutsManager;

/**
 * \ingroup gui
 * \class QgsConfigureShortcutsDialog
 * Reusable dialog for allowing users to configure shortcuts contained in a QgsShortcutsManager.
 * \since QGIS 2.16
 */

class GUI_EXPORT QgsConfigureShortcutsDialog : public QDialog, private Ui::QgsConfigureShortcutsDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsConfigureShortcutsDialog.
     * \param parent parent widget
     * \param manager associated QgsShortcutsManager, or leave as null to use the default
     * singleton QgsShortcutsManager instance.
     */
    QgsConfigureShortcutsDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsShortcutsManager *manager = nullptr );

  protected:
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;

  private slots:
    void changeShortcut();
    void resetShortcut();
    void setNoShortcut();
    void saveShortcuts();
    void loadShortcuts();
    void mLeFilter_textChanged( const QString &text );

    void actionChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );

    //! Open the associated help
    void showHelp();

  private:

    //! Populates the dialog with all actions from the manager
    void populateActions();

    //! Returns the currently selected shortcut object (QAction or QShortcut)
    QObject *currentObject();

    //! Returns the currently selected action, or null if no action selected
    QAction *currentAction();

    //! Returns the currently selected QShortcut, or null if no shortcut selected
    QShortcut *currentShortcut();

    void setGettingShortcut( bool getting );
    void setCurrentActionShortcut( const QKeySequence &s );
    void updateShortcutText();

    QgsShortcutsManager *mManager = nullptr;

    bool mGettingShortcut = false;
    int mModifiers = 0, mKey = 0;

};

#endif //QGSCONFIGURESHORTCUTSDIALOG_H
