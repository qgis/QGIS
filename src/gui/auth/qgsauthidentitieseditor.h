/***************************************************************************
    qgsauthidentitieseditor.h
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

#ifndef QGSAUTHIDENTITIESEDITOR_H
#define QGSAUTHIDENTITIESEDITOR_H

#include <QWidget>
#include <QSslCertificate>

#include "ui_qgsauthidentitieseditor.h"
#include "qgsauthmanager.h"

class QgsMessageBar;

/** \ingroup gui
 * Widget for viewing and editing authentication identities database
 */
class GUI_EXPORT QgsAuthIdentitiesEditor : public QWidget, private Ui::QgsAuthIdentitiesEditor
{
    Q_OBJECT

  public:
    /**
     * Widget for editing authentication configurations directly in database
     * @param parent Parent widget
     */
    explicit QgsAuthIdentitiesEditor( QWidget *parent = nullptr );
    ~QgsAuthIdentitiesEditor();

  private slots:
    void populateIdentitiesView();

    void refreshIdentitiesView();

    void showCertInfo( QTreeWidgetItem *item );

    /** Pass selection change on to UI update */
    void selectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

    /** Update UI based upon current selection */
    void checkSelection();

    void handleDoubleClick( QTreeWidgetItem* item, int col );

    void on_btnAddIdentity_clicked();

    void on_btnRemoveIdentity_clicked();

    void on_btnInfoIdentity_clicked();

    void on_btnGroupByOrg_toggled( bool checked );

    /** Relay messages to widget's messagebar */
    void authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level );

  protected:
    /** Overridden show event of base widget */
    void showEvent( QShowEvent *e ) override;

  private:
    enum IdentityType
    {
      Section = 1000,
      OrgName = 1001,
      CertIdentity = 1002,
    };

    void setupIdentitiesTree();

    void populateIdentitiesSection( QTreeWidgetItem *item, const QList<QSslCertificate>& certs,
                                    QgsAuthIdentitiesEditor::IdentityType identype );

    void appendIdentitiesToGroup( const QList<QSslCertificate>& certs,
                                  QgsAuthIdentitiesEditor::IdentityType identype,
                                  QTreeWidgetItem *parent = nullptr );

    void appendIdentitiesToItem( const QList<QSslCertificate>& certs,
                                 QgsAuthIdentitiesEditor::IdentityType identype,
                                 QTreeWidgetItem *parent = nullptr );

    QgsMessageBar * messageBar();
    int messageTimeout();

    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;

    QTreeWidgetItem *mRootCertIdentItem;
};

#endif // QGSAUTHIDENTITIESEDITOR_H
