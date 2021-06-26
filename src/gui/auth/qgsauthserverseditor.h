/***************************************************************************
    qgsauthserverseditor.h
    ---------------------
    begin                : April 26, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHSERVERSEDITOR_H
#define QGSAUTHSERVERSEDITOR_H

#include <QWidget>
#include "qgis_sip.h"

#include "ui_qgsauthserverseditor.h"
#include "qgsauthmanager.h"
#include "qgis_gui.h"

class QgsMessageBar;

/**
 * \ingroup gui
 * Widget for viewing and editing servers in authentication database
 */
class GUI_EXPORT QgsAuthServersEditor : public QWidget, private Ui::QgsAuthServersEditor
{
    Q_OBJECT

  public:

    /**
     * Widget for editing authentication configurations directly in database
     * \param parent Parent Widget
     */
    explicit QgsAuthServersEditor( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:
    void populateSslConfigsView();

    void refreshSslConfigsView();

    //! Pass selection change on to UI update
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Update UI based upon current selection
    void checkSelection();

    void handleDoubleClick( QTreeWidgetItem *item, int col );

    void btnAddServer_clicked();

    void btnRemoveServer_clicked();

    void btnEditServer_clicked();

    void btnGroupByOrg_toggled( bool checked );

    //! Relay messages to widget's messagebar
    void authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level );

  protected:

    void showEvent( QShowEvent *e ) override;

  private:
    enum ConfigType
    {
      Section = 1000,
      OrgName = 1001,
      ServerConfig = 1002,
    };

    void setupSslConfigsTree();

    void populateSslConfigsSection( QTreeWidgetItem *item,
                                    const QList<QgsAuthConfigSslServer> &configs,
                                    QgsAuthServersEditor::ConfigType conftype );

    void appendSslConfigsToGroup( const QList<QgsAuthConfigSslServer> &configs,
                                  QgsAuthServersEditor::ConfigType conftype,
                                  QTreeWidgetItem *parent = nullptr );

    void appendSslConfigsToItem( const QList<QgsAuthConfigSslServer> &configs,
                                 QgsAuthServersEditor::ConfigType conftype,
                                 QTreeWidgetItem *parent = nullptr );

    QgsMessageBar *messageBar();
    int messageTimeout();

    bool mDisabled = false;
    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;

    QTreeWidgetItem *mRootSslConfigItem = nullptr;
};

#endif // QGSAUTHSERVERSEDITOR_H
