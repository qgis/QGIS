#include <QMessageBox>
#include <QListWidgetItem>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "qgsdatasourcemanagerdialog.h"
#include "ui_qgsdatasourcemanagerdialog.h"
#include "qgsbrowserdockwidget.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"

QgsDataSourceManagerDialog::QgsDataSourceManagerDialog( QWidget *parent ) :
  QDialog( parent ),
  ui( new Ui::QgsDataSourceManagerDialog )
{
  ui->setupUi( this );

  // More setup
  int size = QgsSettings().value( QStringLiteral( "/IconSize" ), 24 ).toInt();
  // buffer size to match displayed icon size in toolbars, and expected geometry restore
  // newWidth (above) may need adjusted if you adjust iconBuffer here
  int iconBuffer = 4;
  ui->mList->setIconSize( QSize( size + iconBuffer, size + iconBuffer ) );
  ui->mList->setFrameStyle( QFrame::NoFrame );
  ui->mListFrame->layout()->setContentsMargins( 0, 3, 3, 3 );

  // Bind list index to the stacked dialogs
  connect( ui->mList, SIGNAL( currentRowChanged( int ) ), this, SLOT( setCurrentPage( int ) ) );

  // Add the browser widget to the first stacked widget page
  mBrowserWidget = new QgsBrowserDockWidget( QStringLiteral( "Browser" ), this );
  mBrowserWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
  ui->mStackedWidget->addWidget( mBrowserWidget );

  // Add data provider dialogs

  // WMS
  QDialog *wmss = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "wms" ), this ) );
  if ( !wmss )
  {
    QMessageBox::warning( this, tr( "WMS" ), tr( "Cannot get WMS select dialog from provider." ) );
  }
  else
  {
    connect( wmss, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
             qApp, SLOT( addRasterLayer( QString const &, QString const &, QString const & ) ) );
    //wmss->exec();
    wmss->setWindowFlags( Qt::Widget );
    QMdiArea *wmsMdi = new QMdiArea( this );
    QMdiSubWindow *wmsSub;
    wmsMdi->setViewMode( QMdiArea::TabbedView );
    wmsSub = wmsMdi->addSubWindow( wmss );
    wmsSub->show();
    ui->mStackedWidget->addWidget( wmsMdi );
    mDialogs.append( wmss ); // TODO: rm
    QListWidgetItem *wmsItem = new QListWidgetItem( tr( "WMS" ), ui->mList );
    wmsItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWmsLayer.svg" ) ) );
  }

}

QgsDataSourceManagerDialog::~QgsDataSourceManagerDialog()
{
  delete ui;
}

void QgsDataSourceManagerDialog::setCurrentPage( int index )
{
  ui->mStackedWidget->setCurrentIndex( index );
}
