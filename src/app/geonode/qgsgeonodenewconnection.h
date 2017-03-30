//
// Created by myarjunar on 27/03/17.
//

#ifndef QGSGEONODENEWCONNECTION_H
#define QGSGEONODENEWCONNECTION_H

#include "ui_qgsnewgeonodeconnectionbase.h"
#include "qgisgui.h"
#include "qgshelp.h"
#include "qgsauthconfigselect.h"

class QgsGeoNodeNewConnection : public QDialog, private Ui::QgsNewGeoNodeConnectionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGeoNodeNewConnection( QWidget *parent = nullptr, const QString &connName = QString::null, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    //! Test the connection using the parameters supplied
    void testConnection();

  public slots:
    void accept() override;
    void okButtonBehavior( const QString & );

  private:
    QString mBaseKey;
    QString mCredentialsBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    QgsAuthConfigSelect *mAuthConfigSelect = nullptr;
};

#endif //QGSGEONODENEWCONNECTION_H
