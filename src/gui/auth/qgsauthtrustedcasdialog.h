/***************************************************************************
    qgsauthtrustedcasdialog.h
    ---------------------
    begin                : May 9, 2015
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

#ifndef QGSAUTHTRUSTEDCASDIALOG_H
#define QGSAUTHTRUSTEDCASDIALOG_H

#include <QDialog>
#include "ui_qgsauthtrustedcasdialog.h"

#include <QSslCertificate>

#include "qgsauthmanager.h"

class QgsMessageBar;

/** \ingroup gui
 * Widget for listing trusted Certificate (Intermediate) Authorities used in secure connections
 */
class GUI_EXPORT QgsAuthTrustedCAsDialog : public QDialog, private Ui::QgsAuthTrustedCAsDialog
{
    Q_OBJECT

  public:
    /**
     * Construct a dialog that will list the trusted Certificate Authorities
     * @param parent Parent widget
     * @param trustedCAs List of trusted Certificate Authorities objects
     */
    explicit QgsAuthTrustedCAsDialog( QWidget *parent = nullptr,
                                      const QList<QSslCertificate>& trustedCAs = QList<QSslCertificate>() );
    ~QgsAuthTrustedCAsDialog();

  private slots:
    void populateCaCertsView();

    void showCertInfo( QTreeWidgetItem *item );

    /** Pass selection change on to UI update */
    void selectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

    /** Update UI based upon current selection */
    void checkSelection();

    void handleDoubleClick( QTreeWidgetItem* item, int col );

    void on_btnInfoCa_clicked();

    void on_btnGroupByOrg_toggled( bool checked );

    /** Relay messages to widget's messagebar */
    void authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level );

  protected:
    /** Overridden widget show event */
    void showEvent( QShowEvent *e ) override;

  private:
    enum CaType
    {
      Section = 1000,
      OrgName = 1001,
      CaCert = 1002,
    };

    void setupCaCertsTree();

    void populateCaCertsSection( QTreeWidgetItem *item, const QList<QSslCertificate>& certs,
                                 QgsAuthTrustedCAsDialog::CaType catype );

    void appendCertsToGroup( const QList<QSslCertificate>& certs,
                             QgsAuthTrustedCAsDialog::CaType catype,
                             QTreeWidgetItem *parent = nullptr );

    void appendCertsToItem( const QList<QSslCertificate>& certs,
                            QgsAuthTrustedCAsDialog::CaType catype,
                            QTreeWidgetItem *parent = nullptr );

    QgsMessageBar * messageBar();
    int messageTimeout();

    QList<QSslCertificate> mTrustedCAs;
    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;

    QTreeWidgetItem * mRootCaSecItem;
};

#endif // QGSAUTHTRUSTEDCASDIALOG_H
