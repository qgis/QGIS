/***************************************************************************
                             qgsappdbutils.cpp
                             ------------------------
    Date                 : April 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappdbutils.h"

#include "browser/qgsinbuiltdataitemproviders.h"
#include "qgisapp.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsdbqueryhistoryprovider.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgshistoryproviderregistry.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "moc_qgsappdbutils.cpp"

//
// QgsQueryHistoryDialog
//

QgsQueryHistoryDialog::QgsQueryHistoryDialog( QWidget *parent )
  : QDialog( parent )
{
  setObjectName( u"QgsQueryHistoryDialog"_s );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Query History" ) );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 6, 6, 6, 6 );
  mWidget = new QgsDatabaseQueryHistoryWidget();
  vl->addWidget( mWidget, 1 );
  connect( mWidget, &QgsDatabaseQueryHistoryWidget::sqlTriggered, this, &QgsQueryHistoryDialog::openQueryDialog );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::StandardButton::Close | QDialogButtonBox::StandardButton::Help );

  QPushButton *clearButton = new QPushButton( tr( "Clear" ) );
  clearButton->setToolTip( tr( "Clear history" ) );
  mButtonBox->addButton( clearButton, QDialogButtonBox::ActionRole );

  connect( clearButton, &QPushButton::clicked, this, &QgsQueryHistoryDialog::clearHistory );
  connect( mButtonBox->button( QDialogButtonBox::Close ), &QPushButton::clicked, mWidget, [this]() { close(); } );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [] {
    QgsHelp::openHelp( u"managing_data_source/create_layers.html#sql-history"_s );
  } );
  vl->addWidget( mButtonBox );

  setLayout( vl );
}

void QgsQueryHistoryDialog::clearHistory()
{
  if ( QMessageBox::question( this, tr( "Clear History" ), tr( "Are you sure you want to clear the database query history?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
  {
    QgsGui::historyProviderRegistry()->clearHistory( Qgis::HistoryProviderBackend::LocalProfile, u"dbquery"_s );
  }
}

void QgsQueryHistoryDialog::openQueryDialog( const QString &connectionUri, const QString &provider, const QString &sql )
{
  QgisApp::instance()->dbUtils()->openQueryDialog( connectionUri, provider, sql );
}

//
// QgsAppDbUtils
//

void QgsAppDbUtils::setup()
{
  mDatabaseItemGuiProvider = new QgsDatabaseItemGuiProvider();
  QgsGui::dataItemGuiProviderRegistry()->addProvider( mDatabaseItemGuiProvider );
}

void QgsAppDbUtils::openQueryDialog( const QString &connectionUri, const QString &provider, const QString &sql )
{
  mDatabaseItemGuiProvider->openSqlDialogGeneric( connectionUri, provider, sql );
}

void QgsAppDbUtils::showQueryHistory()
{
  QgsQueryHistoryDialog *dlg = new QgsQueryHistoryDialog( QgisApp::instance() );
  dlg->setAttribute( Qt::WidgetAttribute::WA_DeleteOnClose );
  dlg->show();
}
