/***************************************************************************
    qgsauthauthoritieseditor.h
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

#ifndef QGSAUTHAUTHORITIESEDITOR_H
#define QGSAUTHAUTHORITIESEDITOR_H

#include <QWidget>
#include "qgis_sip.h"
#include <QSslCertificate>

#include "ui_qgsauthauthoritieseditor.h"
#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgis_gui.h"

class QgsMessageBar;
class QMenu;
class QAction;

/**
 * \ingroup gui
 * \brief Widget for viewing and editing authentication identities database
 */
class GUI_EXPORT QgsAuthAuthoritiesEditor : public QWidget, private Ui::QgsAuthAuthoritiesEditor
{
    Q_OBJECT

  public:

    /**
     * Widget for viewing and editing certificate authorities directly in database
     * \param parent Parent widget
     */
    explicit QgsAuthAuthoritiesEditor( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:
    void populateCaCertsView();

    void refreshCaCertsView();

    void showCertInfo( QTreeWidgetItem *item );

    //! Pass selection change on to UI update
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Update UI based upon current selection
    void checkSelection();

    void handleDoubleClick( QTreeWidgetItem *item, int col );

    void btnAddCa_clicked();

    void btnRemoveCa_clicked();

    void btnInfoCa_clicked();

    void btnGroupByOrg_toggled( bool checked );

    void editDefaultTrustPolicy();

    void defaultTrustPolicyChanged( QgsAuthCertUtils::CertTrustPolicy trustpolicy );

    void btnCaFile_clicked();

    void btnCaFileClear_clicked();

    void showTrustedCertificateAuthorities();

    //! Relay messages to widget's messagebar
    void logMessage( const QString &message, const QString &authtag, Qgis::MessageLevel level );

  protected:
    //! Overridden show event of base widget
    void showEvent( QShowEvent *e ) override;

  private:
    enum CaType
    {
      Section = 1000,
      OrgName = 1001,
      RootCaCert = 1002,
      FileCaCert = 1003,
      DbCaCert = 1004,
    };

    void setupCaCertsTree();

    void populateDatabaseCaCerts();

    void populateFileCaCerts();

    void populateRootCaCerts();

    void populateCaCertsSection( QTreeWidgetItem *item, const QList<QSslCertificate> &certs,
                                 QgsAuthAuthoritiesEditor::CaType catype );

    void appendCertsToGroup( const QList<QSslCertificate> &certs,
                             QgsAuthAuthoritiesEditor::CaType catype,
                             QTreeWidgetItem *parent = nullptr );

    void appendCertsToItem( const QList<QSslCertificate> &certs,
                            QgsAuthAuthoritiesEditor::CaType catype,
                            QTreeWidgetItem *parent = nullptr );

    void updateCertTrustPolicyCache();

    void populateUtilitiesMenu();

    QgsMessageBar *messageBar();
    int messageTimeout();

    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;

    QTreeWidgetItem *mRootCaSecItem = nullptr;
    QTreeWidgetItem *mFileCaSecItem = nullptr;
    QTreeWidgetItem *mDbCaSecItem = nullptr;

    QgsAuthCertUtils::CertTrustPolicy mDefaultTrustPolicy = QgsAuthCertUtils::DefaultTrust;
    QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > mCertTrustCache;

    QMenu *mUtilitiesMenu = nullptr;
    bool mDisabled = false;
    QAction *mActionDefaultTrustPolicy = nullptr;
    QAction *mActionShowTrustedCAs = nullptr;
};

#endif // QGSAUTHAUTHORITIESEDITOR_H
