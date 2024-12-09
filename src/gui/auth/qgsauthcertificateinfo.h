/***************************************************************************
    qgsauthcertificateinfo.h
    ---------------------
    begin                : April 29, 2015
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


#ifndef QGSAUTHCERTIFICATEINFO_H
#define QGSAUTHCERTIFICATEINFO_H

#include <QFile>
#include "qgis_sip.h"

#ifndef QT_NO_SSL
#include <QtCrypto>
#include <QSslCertificate>
#endif

#include <QDialog>
#include <QWidget>
#include "ui_qgsauthcertificateinfo.h"
#include "qgsauthcertutils.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief Widget for viewing detailed info on a certificate and its hierarchical trust chain
 */
class GUI_EXPORT QgsAuthCertInfo : public QWidget, private Ui::QgsAuthCertInfo
{
    Q_OBJECT

  public:
    //! Constructor for QgsAuthCertInfo
    explicit QgsAuthCertInfo( const QSslCertificate &cert, bool manageCertTrust = false, QWidget *parent SIP_TRANSFERTHIS = nullptr, const QList<QSslCertificate> &connectionCAs = QList<QSslCertificate>() );

    bool trustCacheRebuilt() { return mTrustCacheRebuilt; }

  private slots:
    void setupError( const QString &msg );

    void currentCertItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );

    void updateCurrentCert( QTreeWidgetItem *item );

    void btnSaveTrust_clicked();

    void currentPolicyIndexChanged( int indx );

    void decorateCertTreeItem( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy trustpolicy, QTreeWidgetItem *item = nullptr );

  private:
    enum DetailsType
    {
      DetailsSection = 1000,
      DetailsGroup = 1001,
      DetailsField = 1002,
    };

    enum FieldWidget
    {
      NoWidget = 0,
      LineEdit = 1,
      TextEdit = 2,
    };

    void setUpCertDetailsTree();

    void populateTrustBox();

    bool populateQcaCertCollection();

    bool setQcaCertificate( const QSslCertificate &cert );

    bool populateCertChain();

    void setCertHierarchy();

    void updateCurrentCertInfo( int chainindx );

    void populateCertInfo();

    QTreeWidgetItem *addGroupItem( QTreeWidgetItem *parent, const QString &group );

    void addFieldItem( QTreeWidgetItem *parent, const QString &field, const QString &value, FieldWidget wdgt = NoWidget, const QColor &color = QColor() );

    void populateInfoGeneralSection();

    void populateInfoDetailsSection();

    void populateInfoPemTextSection();

    QCA::Certificate mCert;
    QList<QSslCertificate> mConnectionCAs;
    QMap<QString, QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate>> mCaCertsCache;
    QCA::CertificateCollection mCaCerts;
    QCA::CertificateChain mACertChain;
    QList<QSslCertificate> mQCertChain;
    QSslCertificate mCurrentQCert;
    QCA::Certificate mCurrentACert;

    QBrush mDefaultItemForeground;

    bool mManageTrust;
    bool mTrustCacheRebuilt;
    QgsAuthCertUtils::CertTrustPolicy mDefaultTrustPolicy;
    QgsAuthCertUtils::CertTrustPolicy mCurrentTrustPolicy;

    QTreeWidgetItem *mSecGeneral = nullptr;
    QTreeWidgetItem *mSecDetails = nullptr;
    QTreeWidgetItem *mSecPemText = nullptr;
    QTreeWidgetItem *mGrpSubj = nullptr;
    QTreeWidgetItem *mGrpIssu = nullptr;
    QTreeWidgetItem *mGrpCert = nullptr;
    QTreeWidgetItem *mGrpPkey = nullptr;
    QTreeWidgetItem *mGrpExts = nullptr;

    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;
};

//////////////// Embed in dialog ///////////////////

/**
 * \ingroup gui
 * \brief Dialog wrapper for widget displaying detailed info on a certificate and its hierarchical trust chain
 */
class GUI_EXPORT QgsAuthCertInfoDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Construct a dialog displaying detailed info on a certificate and its hierarchical trust chain
     * \param cert Certificate object
     * \param manageCertTrust Whether to show widgets to manage the trust policy of certs in hierarchy
     * \param parent Parent widget
     * \param connectionCAs List of hierarchical certificates in a connection
     */
    explicit QgsAuthCertInfoDialog( const QSslCertificate &cert, bool manageCertTrust, QWidget *parent SIP_TRANSFERTHIS = nullptr, const QList<QSslCertificate> &connectionCAs = QList<QSslCertificate>() );

    //! Gets access to embedded info widget
    QgsAuthCertInfo *certInfoWidget() { return mCertInfoWdgt; }

    /**
     * Whether the trust cache has been rebuilt
     * \note This happens when a trust policy has been adjusted for any cert in the hierarchy
     */
    bool trustCacheRebuilt() { return mCertInfoWdgt->trustCacheRebuilt(); }

  private:
    QgsAuthCertInfo *mCertInfoWdgt = nullptr;
};

#endif // QGSAUTHCERTIFICATEINFO_H
