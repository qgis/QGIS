/***************************************************************************
  qgisapp.cpp  -  description
  -------------------

           begin                : Sat Jun 22 2002
           copyright            : (C) 2002 by Gary E.Sherman
           email                : sherman at mrcc.com
           Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QCursor>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QImageWriter>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLibrary>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPictureIO>
#include <QPixmap>
#include <QPoint>
#include <QPrinter>
#include <QProcess>
#include <QProgressBar>
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include <QScreen>
#include <QShortcut>
#include <QSpinBox>
#include <QSplashScreen>
#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif
#include <QStatusBar>
#include <QStringList>
#include <QSystemTrayIcon>
#include <QTcpSocket>
#include <QTextStream>
#include <QtGlobal>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QWidgetAction>

#include "qgssettings.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgslayerstylingwidget.h"
#include "qgstaskmanager.h"
#include "qgsziputils.h"
#include "qgsbrowsermodel.h"
#include "qgsvectorlayerjoinbuffer.h"

#ifdef HAVE_3D
#include "qgsabstract3drenderer.h"
#include "qgs3dmapcanvasdockwidget.h"
#include "qgs3drendererregistry.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapsettings.h"
#include "qgscameracontroller.h"
#include "qgsflatterraingenerator.h"
#include "qgsvectorlayer3drenderer.h"
#include "processing/qgs3dalgorithms.h"
#endif

#include <QNetworkReply>
#include <QNetworkProxy>
#include <QAuthenticator>

#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#ifdef Q_OS_WIN
#include <QtWinExtras/QWinJumpList>
#include <QtWinExtras/QWinJumpListItem>
#include <QtWinExtras/QWinJumpListCategory>
#endif

Q_GUI_EXPORT extern int qt_defaultDpiX();

//
// Mac OS X Includes
// Must include before GEOS 3 due to unqualified use of 'Point'
//
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#include "qgsmacnative.h"

// check macro breaks QItemDelegate
#ifdef check
#undef check
#endif
#endif

//
// QGIS Specific Includes
//

#include "qgscrashhandler.h"

#include "qgisapp.h"
#include "qgisappinterface.h"
#include "qgisappstylesheet.h"
#include "qgis.h"
#include "qgisplugin.h"
#include "qgsabout.h"
#include "qgsalignrasterdialog.h"
#include "qgsappbrowserproviders.h"
#include "qgsapplayertreeviewmenuprovider.h"
#include "qgsapplication.h"
#include "qgsactionmanager.h"
#include "qgsannotationmanager.h"
#include "qgsannotationregistry.h"
#include "qgsattributetabledialog.h"
#include "qgsattributedialog.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#ifndef QT_NO_SSL
#include "qgsauthcertutils.h"
#include "qgsauthsslerrorsdialog.h"
#endif
#include "qgsbookmarks.h"
#include "qgsbrowserdockwidget.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsclipboard.h"
#include "qgscomposerview.h"
#include "qgsconfigureshortcutsdialog.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinateutils.h"
#include "qgscredentialdialog.h"
#include "qgscustomdrophandler.h"
#include "qgscustomization.h"
#include "qgscustomlayerorderwidget.h"
#include "qgscustomprojectiondialog.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdatasourceuri.h"
#include "qgsdatumtransformdialog.h"
#include "qgsdoublespinbox.h"
#include "qgsdockwidget.h"
#include "qgsdxfexport.h"
#include "qgsdxfexportdialog.h"
#include "qgsdwgimportdialog.h"
#include "qgsdecorationcopyright.h"
#include "qgsdecorationnortharrow.h"
#include "qgsdecorationscalebar.h"
#include "qgsdecorationgrid.h"
#include "qgsdecorationlayoutextent.h"
#include "qgsencodingfiledialog.h"
#include "qgserror.h"
#include "qgserrordialog.h"
#include "qgsexception.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeature.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsformannotation.h"
#include "qgsguiutils.h"
#include "qgshtmlannotation.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsgpsinformationwidget.h"
#include "qgsguivectorlayertools.h"
#include "qgslabelingwidget.h"
#include "qgsdiagramproperties.h"
#include "qgslayerdefinition.h"
#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutcustomdrophandler.h"
#include "qgslayoutdesignerdialog.h"
#include "qgslayoutmanager.h"
#include "qgslayoutmanagerdialog.h"
#include "qgslayoutqptdrophandler.h"
#include "qgslayoutapputils.h"
#include "qgslocatorwidget.h"
#include "qgslocator.h"
#include "qgsinbuiltlocatorfilters.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasdockwidget.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgsmapcanvastracer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmapsettings.h"
#include "qgsmaptip.h"
#include "qgsmenuheader.h"
#include "qgsmergeattributesdialog.h"
#include "qgsmessageviewer.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmemoryproviderutils.h"
#include "qgsmimedatautils.h"
#include "qgsmessagelog.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsnative.h"
#include "qgsnativealgorithms.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgsnewmemorylayerdialog.h"
#include "qgsoptions.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgspluginmanager.h"
#include "qgspluginregistry.h"
#include "qgspointxy.h"
#include "qgsruntimeprofiler.h"
#include "qgshandlebadlayers.h"
#include "qgsprintlayout.h"
#include "qgsprocessingregistry.h"
#include "qgsproject.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgsprojectproperties.h"
#include "qgsproviderregistry.h"
#include "qgspythonrunner.h"
#include "qgsquerybuilder.h"
#include "qgsrastercalcdialog.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterfilewritertask.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrasternuller.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterprojector.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsreport.h"
#include "qgsscalevisibilitydialog.h"
#include "qgsgroupwmsdatadialog.h"
#include "qgsselectbyformdialog.h"
#include "qgsshortcutsmanager.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssnappingwidget.h"
#include "qgsstatisticalsummarydockwidget.h"
#include "qgsstatusbar.h"
#include "qgsstatusbarcoordinateswidget.h"
#include "qgsstatusbarmagnifierwidget.h"
#include "qgsstatusbarscalewidget.h"
#include "qgsstyle.h"
#include "qgssvgannotation.h"
#include "qgstaskmanager.h"
#include "qgstaskmanagerwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgstextannotation.h"
#include "qgsundowidget.h"
#include "qgsuserinputdockwidget.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmapthemes.h"
#include "qgsmessagelogviewer.h"
#include "qgsdataitem.h"
#include "qgsmaplayeractionregistry.h"
#include "qgswelcomepage.h"
#include "qgsversioninfo.h"
#include "qgslegendfilterbutton.h"
#include "qgsvirtuallayerdefinition.h"
#include "qgsvirtuallayerdefinitionutils.h"
#include "qgstransaction.h"
#include "qgstransactiongroup.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerutils.h"
#include "qgshelp.h"
#include "qgsvectorfilewritertask.h"
#include "qgsmapsavedialog.h"
#include "qgsmaprenderertask.h"
#include "qgsmapdecoration.h"
#include "qgsnewnamedialog.h"
#include "qgsgui.h"
#include "qgsdatasourcemanagerdialog.h"

#include "qgsuserprofilemanager.h"
#include "qgsuserprofile.h"

#include "qgssublayersdialog.h"
#include "ogr/qgsvectorlayersaveasdialog.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

//
// GDAL/OGR includes
//
#include <ogr_api.h>
#include <gdal_version.h>
#include <proj_api.h>

//
// Other includes
//
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <list>
#include <memory>
#include <vector>

//
// Map tools
//
#include "qgsmaptooladdpart.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmaptooladdring.h"
#include "qgsmaptoolfillring.h"
#include "qgsmaptoolannotation.h"
#include "qgsmaptoolcircularstringcurvepoint.h"
#include "qgsmaptoolcircularstringradius.h"
#include "qgsmaptoolcircle2points.h"
#include "qgsmaptoolcircle3points.h"
#include "qgsmaptoolcircle3tangents.h"
#include "qgsmaptoolcircle2tangentspoint.h"
#include "qgsmaptoolcirclecenterpoint.h"
#include "qgsmaptoolellipsecenter2points.h"
#include "qgsmaptoolellipsecenterpoint.h"
#include "qgsmaptoolellipseextent.h"
#include "qgsmaptoolellipsefoci.h"
#include "qgsmaptoolrectanglecenter.h"
#include "qgsmaptoolrectangleextent.h"
#include "qgsmaptoolrectangle3points.h"
#include "qgsmaptoolregularpolygon2points.h"
#include "qgsmaptoolregularpolygoncenterpoint.h"
#include "qgsmaptoolregularpolygoncentercorner.h"
#include "qgsmaptooldeletering.h"
#include "qgsmaptooldeletepart.h"
#include "qgsmaptoolfeatureaction.h"
#include "qgsmaptoolformannotation.h"
#include "qgsmaptoolhtmlannotation.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmaptoolmeasureangle.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsmaptoolrotatefeature.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsmaptooloffsetpointsymbol.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectrectangle.h"
#include "qgsmaptoolselectfreehand.h"
#include "qgsmaptoolselectpolygon.h"
#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolsvgannotation.h"
#include "qgsmaptoolreshape.h"
#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsmaptoolsplitparts.h"
#include "qgsmaptooltextannotation.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolsimplify.h"
#include "qgsmeasuretool.h"
#include "qgsmaptoolpinlabels.h"
#include "qgsmaptoolshowhidelabels.h"
#include "qgsmaptoolmovelabel.h"
#include "qgsmaptoolrotatelabel.h"
#include "qgsmaptoolchangelabelproperties.h"

#include "nodetool/qgsnodetool.h"

// Editor widgets
#include "qgseditorwidgetregistry.h"
//
// Conditional Includes
//
#ifdef HAVE_PGCONFIG
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include <pg_config.h>
#else
#define PG_VERSION "unknown"
#endif

#include <sqlite3.h>

extern "C"
{
#include <spatialite.h>
}
#include "qgsnewspatialitelayerdialog.h"
#include "qgsnewgeopackagelayerdialog.h"

#ifdef WITH_BINDINGS
#include "qgspythonutils.h"
#endif

#ifndef Q_OS_WIN
#include <dlfcn.h>
#else
#include <windows.h>
#include <DbgHelp.h>
#endif

class QTreeWidgetItem;
class QgsUserProfileManager;
class QgsUserProfile;

/**
 * Set the application title bar text

  If the current project title is null
  if the project file is null then
  set title text to just application name
  else
  set set title text to the project file name
  else
  set the title text to project title
  */
static void setTitleBarText_( QWidget &qgisApp )
{
  QString caption;
  if ( QgsProject::instance()->title().isEmpty() )
  {
    if ( QgsProject::instance()->fileName().isEmpty() )
    {
      // no project title nor file name, so just leave caption with
      // application name and version
    }
    else
    {
      QFileInfo projectFileInfo( QgsProject::instance()->fileName() );
      caption = projectFileInfo.completeBaseName() + " - ";
    }
  }
  else
  {
    caption = QgsProject::instance()->title() + " - ";
  }

  caption += QgisApp::tr( "QGIS" );

  if ( Qgis::QGIS_VERSION.endsWith( QLatin1String( "Master" ) ) )
  {
    caption += QStringLiteral( " %1" ).arg( Qgis::QGIS_DEV_VERSION );
  }

  qgisApp.setWindowTitle( caption );
}

/**
 Creator function for output viewer
*/
static QgsMessageOutput *messageOutputViewer_()
{
  if ( QThread::currentThread() == qApp->thread() )
    return new QgsMessageViewer( QgisApp::instance() );
  else
    return new QgsMessageOutputConsole();
}

static void customSrsValidation_( QgsCoordinateReferenceSystem &srs )
{
  QgisApp::instance()->emitCustomCrsValidation( srs );
}

void QgisApp::emitCustomCrsValidation( QgsCoordinateReferenceSystem &srs )
{
  emit customCrsValidation( srs );
}

void QgisApp::layerTreeViewDoubleClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
  QgsSettings settings;
  switch ( settings.value( QStringLiteral( "qgis/legendDoubleClickAction" ), 0 ).toInt() )
  {
    case 0:
    {
      //show properties
      if ( mLayerTreeView )
      {
        // if it's a legend node, open symbol editor directly
        if ( QgsSymbolLegendNode *node = dynamic_cast<QgsSymbolLegendNode *>( mLayerTreeView->currentLegendNode() ) )
        {
          const QgsSymbol *originalSymbol = node->symbol();
          if ( !originalSymbol )
            return;

          std::unique_ptr< QgsSymbol > symbol( originalSymbol->clone() );
          QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() );
          QgsSymbolSelectorDialog dlg( symbol.get(), QgsStyle::defaultStyle(), vlayer, this );
          QgsSymbolWidgetContext context;
          context.setMapCanvas( mMapCanvas );
          dlg.setContext( context );
          if ( dlg.exec() )
          {
            node->setSymbol( symbol.release() );
          }

          return;
        }
      }
      QgisApp::instance()->layerProperties();
      break;
    }
    case 1:
      QgisApp::instance()->attributeTable();
      break;
    case 2:
      mapStyleDock( true );
      break;
    default:
      break;
  }
}

void QgisApp::activeLayerChanged( QgsMapLayer *layer )
{
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
    canvas->setCurrentLayer( layer );

  if ( mUndoWidget )
  {
    if ( layer )
    {
      mUndoWidget->setUndoStack( layer->undoStack() );
    }
    else
    {
      mUndoWidget->destroyStack();
    }
    updateUndoActions();
  }
}

/**
 * This function contains forced validation of CRS used in QGIS.
 * There are 3 options depending on the settings:
 * - ask for CRS using projection selecter
 * - use project's CRS
 * - use predefined global CRS
 */
void QgisApp::validateCrs( QgsCoordinateReferenceSystem &srs )
{
  static QString sAuthId = QString();
  QgsSettings mySettings;
  QString myDefaultProjectionOption = mySettings.value( QStringLiteral( "Projections/defaultBehavior" ), "prompt" ).toString();
  if ( myDefaultProjectionOption == QLatin1String( "prompt" ) )
  {
    // \note this class is not a descendent of QWidget so we can't pass
    // it in the ctor of the layer projection selector

    QgsProjectionSelectionDialog *mySelector = new QgsProjectionSelectionDialog();
    mySelector->setMessage( srs.validationHint() ); //shows a generic message, if not specified
    if ( sAuthId.isNull() )
      sAuthId = QgsProject::instance()->crs().authid();

    QgsCoordinateReferenceSystem defaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( sAuthId );
    if ( defaultCrs.isValid() )
    {
      mySelector->setCrs( defaultCrs );
    }

    bool waiting = QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::WaitCursor;
    if ( waiting )
      QApplication::setOverrideCursor( Qt::ArrowCursor );

    if ( mySelector->exec() )
    {
      QgsDebugMsg( "Layer srs set from dialog: " + QString::number( mySelector->crs().srsid() ) );
      srs = mySelector->crs();
      sAuthId = srs.authid();
    }

    if ( waiting )
      QApplication::restoreOverrideCursor();

    delete mySelector;
  }
  else if ( myDefaultProjectionOption == QLatin1String( "useProject" ) )
  {
    // XXX TODO: Change project to store selected CS as 'projectCRS' not 'selectedWkt'
    sAuthId = QgsProject::instance()->crs().authid();
    srs.createFromOgcWmsCrs( sAuthId );
    QgsDebugMsg( "Layer srs set from project: " + sAuthId );
    messageBar()->pushMessage( tr( "CRS was undefined" ), tr( "defaulting to project CRS %1 - %2" ).arg( sAuthId, srs.description() ), QgsMessageBar::WARNING, messageTimeout() );
  }
  else ///Projections/defaultBehavior==useGlobal
  {
    sAuthId = mySettings.value( QStringLiteral( "Projections/layerDefaultCrs" ), GEO_EPSG_CRS_AUTHID ).toString();
    srs.createFromOgcWmsCrs( sAuthId );
    QgsDebugMsg( "Layer srs set from default: " + sAuthId );
    messageBar()->pushMessage( tr( "CRS was undefined" ), tr( "defaulting to CRS %1 - %2" ).arg( sAuthId, srs.description() ), QgsMessageBar::WARNING, messageTimeout() );
  }
}

static bool cmpByText_( QAction *a, QAction *b )
{
  return QString::localeAwareCompare( a->text(), b->text() ) < 0;
}


QgisApp *QgisApp::sInstance = nullptr;

// constructor starts here
QgisApp::QgisApp( QSplashScreen *splash, bool restorePlugins, bool skipVersionCheck, const QString &rootProfileLocation, const QString &activeProfile, QWidget *parent, Qt::WindowFlags fl )
  : QMainWindow( parent, fl )
  , mSplash( splash )
{
  if ( sInstance )
  {
    QMessageBox::critical(
      this,
      tr( "Multiple Instances of QgisApp" ),
      tr( "Multiple instances of QGIS application object detected.\nPlease contact the developers.\n" ) );
    abort();
  }

  sInstance = this;
  QgsRuntimeProfiler *profiler = QgsApplication::profiler();

  startProfile( QStringLiteral( "User profile manager" ) );
  mUserProfileManager = new QgsUserProfileManager( QString(), this );
  mUserProfileManager->setRootLocation( rootProfileLocation );
  mUserProfileManager->setActiveUserProfile( activeProfile );
  mUserProfileManager->setNewProfileNotificationEnabled( true );
  connect( mUserProfileManager, &QgsUserProfileManager::profilesChanged, this, &QgisApp::refreshProfileMenu );
  endProfile();

  // load GUI: actions, menus, toolbars
  profiler->beginGroup( QStringLiteral( "qgisapp" ) );
  profiler->beginGroup( QStringLiteral( "startup" ) );
  startProfile( QStringLiteral( "Setting up UI" ) );
  setupUi( this );
  endProfile();

#if QT_VERSION >= 0x050600
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );
#endif

  //////////

  startProfile( QStringLiteral( "Checking database" ) );
  mSplash->showMessage( tr( "Checking database" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  // Do this early on before anyone else opens it and prevents us copying it
  QString dbError;
  if ( !QgsApplication::createDatabase( &dbError ) )
  {
    QMessageBox::critical( this, tr( "Private qgis.db" ), dbError );
  }
  endProfile();

  mTray = new QSystemTrayIcon();
  mTray->setIcon( QIcon( QgsApplication::appIconPath() ) );
  mTray->hide();

  // Create the themes folder for the user
  startProfile( QStringLiteral( "Creating theme folder" ) );
  QgsApplication::createThemeFolder();
  endProfile();

  mSplash->showMessage( tr( "Reading settings" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  mSplash->showMessage( tr( "Setting up the GUI" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  QgsApplication::initQgis();
  if ( !QgsApplication::authManager()->isDisabled() )
  {
    // Most of the auth initialization is now done inside initQgis, no need to profile here
    masterPasswordSetup();
  }

  QgsSettings settings;

  startProfile( QStringLiteral( "Building style sheet" ) );
  // set up stylesheet builder and apply saved or default style options
  mStyleSheetBuilder = new QgisAppStyleSheet( this );
  connect( mStyleSheetBuilder, &QgisAppStyleSheet::appStyleSheetChanged,
           this, &QgisApp::setAppStyleSheet );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetBuilder->defaultOptions() );
  endProfile();

  QWidget *centralWidget = this->centralWidget();
  QGridLayout *centralLayout = new QGridLayout( centralWidget );
  centralWidget->setLayout( centralLayout );
  centralLayout->setContentsMargins( 0, 0, 0, 0 );

  // "theMapCanvas" used to find this canonical instance later
  startProfile( QStringLiteral( "Creating map canvas" ) );
  mMapCanvas = new QgsMapCanvas( centralWidget );
  mMapCanvas->setObjectName( QStringLiteral( "theMapCanvas" ) );
  connect( mMapCanvas, &QgsMapCanvas::messageEmitted, this, &QgisApp::displayMessage );
  mMapCanvas->setWhatsThis( tr( "Map canvas. This is where raster and vector "
                                "layers are displayed when added to the map" ) );

  if ( settings.value( QStringLiteral( "qgis/main_canvas_preview_jobs" ) ).isNull() )
  {
    // So that it appears in advanced settings
    settings.setValue( QStringLiteral( "qgis/main_canvas_preview_jobs" ), true );
  }
  mMapCanvas->setPreviewJobsEnabled( settings.value( QStringLiteral( "qgis/main_canvas_preview_jobs" ), true ).toBool() );

  // set canvas color right away
  int myRed = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  int myGreen = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  int myBlue = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  mMapCanvas->setCanvasColor( QColor( myRed, myGreen, myBlue ) );
  endProfile();

  // what type of project to auto-open
  mProjOpen = settings.value( QStringLiteral( "qgis/projOpenAtLaunch" ), 0 ).toInt();


  startProfile( QStringLiteral( "Welcome page" ) );
  mWelcomePage = new QgsWelcomePage( skipVersionCheck );
  connect( mWelcomePage, &QgsWelcomePage::projectRemoved, this, [ this ]( int row )
  {
    mRecentProjects.removeAt( row );
    saveRecentProjects();
    updateRecentProjectPaths();
  } );
  connect( mWelcomePage, &QgsWelcomePage::projectPinned, this, [ this ]( int row )
  {
    mRecentProjects.at( row ).pin = true;
    saveRecentProjects();
    updateRecentProjectPaths();
  } );
  connect( mWelcomePage, &QgsWelcomePage::projectUnpinned, this, [ this ]( int row )
  {
    mRecentProjects.at( row ).pin = false;
    saveRecentProjects();
    updateRecentProjectPaths();
  } );
  endProfile();

  mCentralContainer = new QStackedWidget;
  mCentralContainer->insertWidget( 0, mMapCanvas );
  mCentralContainer->insertWidget( 1, mWelcomePage );

  centralLayout->addWidget( mCentralContainer, 0, 0, 2, 1 );

  connect( mMapCanvas, &QgsMapCanvas::layersChanged, this, &QgisApp::showMapCanvas );

  mCentralContainer->setCurrentIndex( mProjOpen ? 0 : 1 );

  // a bar to warn the user with non-blocking messages
  startProfile( QStringLiteral( "Message bar" ) );
  mInfoBar = new QgsMessageBar( centralWidget );
  mInfoBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  centralLayout->addWidget( mInfoBar, 0, 0, 1, 1 );
  endProfile();

  startProfile( QStringLiteral( "User input dock" ) );
  // User Input Dock Widget
  mUserInputDockWidget = new QgsUserInputDockWidget( this );
  mUserInputDockWidget->setObjectName( QStringLiteral( "UserInputDockWidget" ) );
  endProfile();

  //set the focus to the map canvas
  mMapCanvas->setFocus();

  startProfile( QStringLiteral( "Layer tree" ) );
  mLayerTreeView = new QgsLayerTreeView( this );
  mLayerTreeView->setObjectName( QStringLiteral( "theLayerTreeView" ) ); // "theLayerTreeView" used to find this canonical instance later
  endProfile();

  // create undo widget
  startProfile( QStringLiteral( "Undo dock" ) );
  mUndoDock = new QgsDockWidget( tr( "Undo/Redo Panel" ), this );
  mUndoWidget = new QgsUndoWidget( mUndoDock, mMapCanvas );
  mUndoWidget->setObjectName( QStringLiteral( "Undo" ) );
  mUndoDock->setWidget( mUndoWidget );
  mUndoDock->setObjectName( QStringLiteral( "undo/redo dock" ) );
  endProfile();

  // Advanced Digitizing dock
  startProfile( QStringLiteral( "Advanced digitize panel" ) );
  mAdvancedDigitizingDockWidget = new QgsAdvancedDigitizingDockWidget( mMapCanvas, this );
  mAdvancedDigitizingDockWidget->setObjectName( QStringLiteral( "AdvancedDigitizingTools" ) );
  endProfile();

  // Statistical Summary dock
  startProfile( QStringLiteral( "Stats dock" ) );
  mStatisticalSummaryDockWidget = new QgsStatisticalSummaryDockWidget( this );
  mStatisticalSummaryDockWidget->setObjectName( QStringLiteral( "StatistalSummaryDockWidget" ) );
  endProfile();

  // Bookmarks dock
  startProfile( QStringLiteral( "Bookmarks widget" ) );
  mBookMarksDockWidget = new QgsBookmarks( this );
  mBookMarksDockWidget->setObjectName( QStringLiteral( "BookmarksDockWidget" ) );
  endProfile();

  startProfile( QStringLiteral( "Snapping utils" ) );
  mSnappingUtils = new QgsMapCanvasSnappingUtils( mMapCanvas, this );
  mMapCanvas->setSnappingUtils( mSnappingUtils );
  connect( QgsProject::instance(), &QgsProject::snappingConfigChanged, mSnappingUtils, &QgsSnappingUtils::setConfig );
  connect( mSnappingUtils, &QgsSnappingUtils::configChanged, QgsProject::instance(), &QgsProject::setSnappingConfig );


  endProfile();

  functionProfile( &QgisApp::createMenus, this, QStringLiteral( "Create menus" ) );
  functionProfile( &QgisApp::createActions, this, QStringLiteral( "Create actions" ) );
  functionProfile( &QgisApp::createActionGroups, this, QStringLiteral( "Create action group" ) );
  functionProfile( &QgisApp::createToolBars, this, QStringLiteral( "Toolbars" ) );
  functionProfile( &QgisApp::createStatusBar, this, QStringLiteral( "Status bar" ) );
  functionProfile( &QgisApp::createCanvasTools, this, QStringLiteral( "Create canvas tools" ) );

  mMapCanvas->freeze();
  applyDefaultSettingsToCanvas( mMapCanvas );

  functionProfile( &QgisApp::initLayerTreeView, this, QStringLiteral( "Init Layer tree view" ) );
  functionProfile( &QgisApp::createOverview, this, QStringLiteral( "Create overview" ) );
  functionProfile( &QgisApp::createMapTips, this, QStringLiteral( "Create map tips" ) );
  functionProfile( &QgisApp::createDecorations, this, QStringLiteral( "Create decorations" ) );
  functionProfile( &QgisApp::readSettings, this, QStringLiteral( "Read settings" ) );
  functionProfile( &QgisApp::updateProjectFromTemplates, this, QStringLiteral( "Update project from templates" ) );
  functionProfile( &QgisApp::legendLayerSelectionChanged, this, QStringLiteral( "Legend layer selection changed" ) );
  functionProfile( &QgisApp::init3D, this, QStringLiteral( "Initialize 3D support" ) );
  functionProfile( &QgisApp::initNativeProcessing, this, QStringLiteral( "Initialize native processing" ) );
  functionProfile( &QgisApp::initLayouts, this, QStringLiteral( "Initialize layouts support" ) );

  QgsApplication::annotationRegistry()->addAnnotationType( QgsAnnotationMetadata( QStringLiteral( "FormAnnotationItem" ), &QgsFormAnnotation::create ) );
  connect( QgsProject::instance()->annotationManager(), &QgsAnnotationManager::annotationAdded, this, &QgisApp::annotationCreated );

  mSaveRollbackInProgress = false;

  QString templateDirName = settings.value( QStringLiteral( "qgis/projectTemplateDir" ),
                            QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
  if ( !QFileInfo::exists( templateDirName ) )
  {
    // create default template directory
    if ( !QDir().mkdir( QgsApplication::qgisSettingsDirPath() + "project_templates" ) )
      templateDirName.clear();
  }
  if ( !templateDirName.isEmpty() ) // template directory exists, so watch it!
  {
    QFileSystemWatcher *projectsTemplateWatcher = new QFileSystemWatcher( this );
    projectsTemplateWatcher->addPath( templateDirName );
    connect( projectsTemplateWatcher, &QFileSystemWatcher::directoryChanged, this, [this] { updateProjectFromTemplates(); } );
  }

  // Update welcome page list
  startProfile( QStringLiteral( "Update recent project paths" ) );
  updateRecentProjectPaths();
  mWelcomePage->setRecentProjects( mRecentProjects );
  endProfile();

  // initialize the plugin manager
  startProfile( QStringLiteral( "Plugin manager" ) );
  mPluginManager = new QgsPluginManager( this, restorePlugins );
  endProfile();

  addDockWidget( Qt::LeftDockWidgetArea, mUndoDock );
  mUndoDock->hide();

  startProfile( QStringLiteral( "Layer Style dock" ) );
  mMapStylingDock = new QgsDockWidget( this );
  mMapStylingDock->setWindowTitle( tr( "Layer Styling" ) );
  mMapStylingDock->setObjectName( QStringLiteral( "LayerStyling" ) );
  mMapStyleWidget = new QgsLayerStylingWidget( mMapCanvas, mMapLayerPanelFactories );
  mMapStylingDock->setWidget( mMapStyleWidget );
  connect( mMapStyleWidget, &QgsLayerStylingWidget::styleChanged, this, &QgisApp::updateLabelToolButtons );
  connect( mMapStylingDock, &QDockWidget::visibilityChanged, mActionStyleDock, &QAction::setChecked );

  addDockWidget( Qt::RightDockWidgetArea, mMapStylingDock );
  mMapStylingDock->hide();
  endProfile();

  startProfile( QStringLiteral( "Snapping dialog" ) );
  mSnappingDialog = new QgsSnappingWidget( QgsProject::instance(), mMapCanvas, this );
  connect( mSnappingDialog, &QgsSnappingWidget::snappingConfigChanged, QgsProject::instance(), [ = ] { QgsProject::instance()->setSnappingConfig( mSnappingDialog->config() ); } );
  QString mainSnappingWidgetMode = QgsSettings().value( QStringLiteral( "/qgis/mainSnappingWidgetMode" ), "dialog" ).toString();
  if ( mainSnappingWidgetMode == QLatin1String( "dock" ) )
  {
    QgsDockWidget *dock = new QgsDockWidget( tr( "Snapping and Digitizing Options" ), QgisApp::instance() );
    dock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dock->setWidget( mSnappingDialog );
    dock->setObjectName( QStringLiteral( "Snapping and Digitizing Options" ) );
    addDockWidget( Qt::LeftDockWidgetArea, dock );
    mSnappingDialogContainer = dock;
    dock->hide();
  }
  else
  {
    QDialog *dialog = new QDialog( this );
    dialog->setWindowTitle( tr( "Project Snapping Settings" ) );
    QVBoxLayout *layout = new QVBoxLayout( dialog );
    layout->addWidget( mSnappingDialog );
    layout->setMargin( 0 );
    mSnappingDialogContainer = dialog;
  }
  endProfile();

  mBrowserModel = new QgsBrowserModel( this );
  mBrowserWidget = new QgsBrowserDockWidget( tr( "Browser Panel" ), mBrowserModel, this );
  mBrowserWidget->setObjectName( QStringLiteral( "Browser" ) );
  addDockWidget( Qt::LeftDockWidgetArea, mBrowserWidget );
  mBrowserWidget->hide();
  connect( this, &QgisApp::newProject, mBrowserWidget, &QgsBrowserDockWidget::updateProjectHome );
  // Only connect the first widget: the model is shared, there is no need to refresh multiple times.
  connect( this, &QgisApp::connectionsChanged, mBrowserWidget, &QgsBrowserDockWidget::refresh );
  connect( mBrowserWidget, &QgsBrowserDockWidget::connectionsChanged, this, &QgisApp::connectionsChanged );
  connect( mBrowserWidget, &QgsBrowserDockWidget::openFile, this, &QgisApp::openFile );
  connect( mBrowserWidget, &QgsBrowserDockWidget::handleDropUriList, this, &QgisApp::handleDropUriList );

  mBrowserWidget2 = new QgsBrowserDockWidget( tr( "Browser Panel (2)" ), mBrowserModel, this );
  mBrowserWidget2->setObjectName( QStringLiteral( "Browser2" ) );
  addDockWidget( Qt::LeftDockWidgetArea, mBrowserWidget2 );
  mBrowserWidget2->hide();
  connect( this, &QgisApp::newProject, mBrowserWidget2, &QgsBrowserDockWidget::updateProjectHome );
  connect( mBrowserWidget2, &QgsBrowserDockWidget::connectionsChanged, this, &QgisApp::connectionsChanged );
  connect( mBrowserWidget2, &QgsBrowserDockWidget::openFile, this, &QgisApp::openFile );
  connect( mBrowserWidget2, &QgsBrowserDockWidget::handleDropUriList, this, &QgisApp::handleDropUriList );

  addDockWidget( Qt::LeftDockWidgetArea, mAdvancedDigitizingDockWidget );
  mAdvancedDigitizingDockWidget->hide();

  addDockWidget( Qt::LeftDockWidgetArea, mStatisticalSummaryDockWidget );
  mStatisticalSummaryDockWidget->hide();

  addDockWidget( Qt::LeftDockWidgetArea, mBookMarksDockWidget );
  mBookMarksDockWidget->hide();

  QMainWindow::addDockWidget( Qt::BottomDockWidgetArea, mUserInputDockWidget );
  mUserInputDockWidget->setFloating( true );

  // create the GPS tool on starting QGIS - this is like the browser
  mpGpsWidget = new QgsGPSInformationWidget( mMapCanvas );
  //create the dock widget
  mpGpsDock = new QgsDockWidget( tr( "GPS Information Panel" ), this );
  mpGpsDock->setObjectName( QStringLiteral( "GPSInformation" ) );
  mpGpsDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  addDockWidget( Qt::LeftDockWidgetArea, mpGpsDock );
  // add to the Panel submenu
  // now add our widget to the dock - ownership of the widget is passed to the dock
  mpGpsDock->setWidget( mpGpsWidget );
  mpGpsDock->hide();

  mLastMapToolMessage = nullptr;

  mLogViewer = new QgsMessageLogViewer( this );

  mLogDock = new QgsDockWidget( tr( "Log Messages Panel" ), this );
  mLogDock->setObjectName( QStringLiteral( "MessageLog" ) );
  mLogDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
  addDockWidget( Qt::BottomDockWidgetArea, mLogDock );
  mLogDock->setWidget( mLogViewer );
  mLogDock->hide();
  connect( mMessageButton, &QAbstractButton::toggled, mLogDock, &QWidget::setVisible );
  connect( mLogDock, &QDockWidget::visibilityChanged, mMessageButton, &QAbstractButton::setChecked );
  connect( QgsApplication::messageLog(), static_cast < void ( QgsMessageLog::* )( bool ) >( &QgsMessageLog::messageReceived ), this, &QgisApp::toggleLogMessageIcon );
  connect( mMessageButton, &QAbstractButton::toggled, this, &QgisApp::toggleLogMessageIcon );
  mVectorLayerTools = new QgsGuiVectorLayerTools();

  // Init the editor widget types
  QgsGui::editorWidgetRegistry()->initEditors( mMapCanvas, mInfoBar );

  mInternalClipboard = new QgsClipboard; // create clipboard
  connect( mInternalClipboard, &QgsClipboard::changed, this, &QgisApp::clipboardChanged );
  mQgisInterface = new QgisAppInterface( this ); // create the interface

#ifdef Q_OS_MAC
  // action for Window menu (create before generating WindowTitleChange event))
  mWindowAction = new QAction( this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );

  // add this window to Window menu
  addWindow( mWindowAction );
#endif

  activateDeactivateLayerRelatedActions( nullptr ); // after members were created

  connect( QgsGui::mapLayerActionRegistry(), &QgsMapLayerActionRegistry::changed, this, &QgisApp::refreshActionFeatureAction );

  // set application's caption
  QString caption = tr( "QGIS - %1 ('%2')" ).arg( Qgis::QGIS_VERSION, Qgis::QGIS_RELEASE_NAME );
  setWindowTitle( caption );

  QgsMessageLog::logMessage( tr( "QGIS starting..." ), QString(), QgsMessageLog::INFO );

  // set QGIS specific srs validation
  connect( this, &QgisApp::customCrsValidation,
           this, &QgisApp::validateCrs );
  QgsCoordinateReferenceSystem::setCustomCrsValidation( customSrsValidation_ );

  // set graphical message output
  QgsMessageOutput::setMessageOutputCreator( messageOutputViewer_ );

  // set graphical credential requester
  new QgsCredentialDialog( this );

  mLocatorWidget->setMapCanvas( mMapCanvas );
  connect( mLocatorWidget, &QgsLocatorWidget::configTriggered, this, [ = ] { showOptionsDialog( this, QStringLiteral( "mOptionsLocatorSettings" ) ); } );

  qApp->processEvents();

  // load providers
  mSplash->showMessage( tr( "Checking provider plugins" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  // Setup QgsNetworkAccessManager (this needs to happen after authentication, for proxy settings)
  namSetup();

  QgsApplication::dataItemProviderRegistry()->addProvider( new QgsQlrDataItemProvider() );
  registerCustomDropHandler( new QgsQlrDropHandler() );
  QgsApplication::dataItemProviderRegistry()->addProvider( new QgsQptDataItemProvider() );
  registerCustomDropHandler( new QgsQptDropHandler() );
  mSplash->showMessage( tr( "Starting Python" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  loadPythonSupport();

#ifdef WITH_BINDINGS
  QgsApplication::dataItemProviderRegistry()->addProvider( new QgsPyDataItemProvider() );
  registerCustomDropHandler( new QgsPyDropHandler() );
#endif

  // Create the plugin registry and load plugins
  // load any plugins that were running in the last session
  mSplash->showMessage( tr( "Restoring loaded plugins" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  QgsPluginRegistry::instance()->setQgisInterface( mQgisInterface );
  if ( restorePlugins )
  {
    // Restoring of plugins can be disabled with --noplugins command line option
    // because some plugins may cause QGIS to crash during startup
    QgsPluginRegistry::instance()->restoreSessionPlugins( QgsApplication::pluginPath() );

    // Also restore plugins from user specified plugin directories
    QString myPaths = settings.value( QStringLiteral( "plugins/searchPathsForPlugins" ), "" ).toString();
    if ( !myPaths.isEmpty() )
    {
      QStringList myPathList = myPaths.split( '|' );
      QgsPluginRegistry::instance()->restoreSessionPlugins( myPathList );
    }
  }

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    startProfile( QStringLiteral( "initPluginInstaller" ) );
    // initialize the plugin installer to start fetching repositories in background
    QgsPythonRunner::run( QStringLiteral( "import pyplugin_installer" ) );
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.initPluginInstaller()" ) );
    // enable Python in the Plugin Manager and pass the PythonUtils to it
    mPluginManager->setPythonUtils( mPythonUtils );
    endProfile();
  }
  else if ( mActionShowPythonDialog )
#endif
  {
    // python is disabled so get rid of the action for python console
    // and installing plugin from ZUIP
    delete mActionShowPythonDialog;
    mActionShowPythonDialog = nullptr;
  }

  // Set icon size of toolbars
  if ( settings.contains( QStringLiteral( "IconSize" ) ) )
  {
    int size = settings.value( QStringLiteral( "IconSize" ) ).toInt();
    if ( size < 16 )
      size = QGIS_ICON_SIZE;
    setIconSizes( size );
  }
  else
  {
    // first run, guess a good icon size
    int size = chooseReasonableDefaultIconSize();
    settings.setValue( QStringLiteral( "IconSize" ), size );
    setIconSizes( size );
  }

  mSplash->showMessage( tr( "Initializing file filters" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  // now build vector and raster file filters
  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

  // set handler for missing layers (will be owned by QgsProject)
  QgsProject::instance()->setBadLayerHandler( new QgsHandleBadLayersHandler() );

#if 0
  // Set the background color for toolbox and overview as they default to
  // white instead of the window color
  QPalette myPalette = toolBox->palette();
  myPalette.setColor( QPalette::Button, myPalette.window().color() );
  toolBox->setPalette( myPalette );
  //do the same for legend control
  myPalette = toolBox->palette();
  myPalette.setColor( QPalette::Button, myPalette.window().color() );
  mMapLegend->setPalette( myPalette );
  //and for overview control this is done in createOverView method
#endif
  // Do this last in the ctor to ensure that all members are instantiated properly
  setupConnections();
  //
  // Please make sure this is the last thing the ctor does so that we can ensure the
  // widgets are all initialized before trying to restore their state.
  //
  mSplash->showMessage( tr( "Restoring window state" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  startProfile( QStringLiteral( "Restore window state" ) );
  restoreWindowState();
  endProfile();

  // do main window customization - after window state has been restored, before the window is shown
  startProfile( QStringLiteral( "Update customiziation on main window" ) );
  QgsCustomization::instance()->updateMainWindow( mToolbarMenu );
  endProfile();

  mSplash->showMessage( tr( "Populate saved styles" ), Qt::AlignHCenter | Qt::AlignBottom );
  startProfile( QStringLiteral( "Populate saved styles" ) );
  QgsStyle::defaultStyle();
  endProfile();

  mSplash->showMessage( tr( "QGIS Ready!" ), Qt::AlignHCenter | Qt::AlignBottom );

  QgsMessageLog::logMessage( QgsApplication::showSettings(), QString(), QgsMessageLog::INFO );

  QgsMessageLog::logMessage( tr( "QGIS Ready!" ), QString(), QgsMessageLog::INFO );

  mMapTipsVisible = false;
  // This turns on the map tip if they where active in the last session
  if ( settings.value( QStringLiteral( "qgis/enableMapTips" ), false ).toBool() )
  {
    toggleMapTips( true );
  }

  mTrustedMacros = false;

  // setup drag drop
  setAcceptDrops( true );

  mFullScreenMode = false;
  mPrevScreenModeMaximized = false;
  startProfile( QStringLiteral( "Show main window" ) );
  show();
  qApp->processEvents();
  endProfile();

  mMapCanvas->freeze( false );
  mMapCanvas->clearExtentHistory(); // reset zoomnext/zoomlast

  QShortcut *zoomInShortCut = new QShortcut( QKeySequence( tr( "Ctrl++" ) ), this );
  connect( zoomInShortCut, &QShortcut::activated, mMapCanvas, &QgsMapCanvas::zoomIn );
  zoomInShortCut->setObjectName( QStringLiteral( "ZoomInToCanvas" ) );
  zoomInShortCut->setWhatsThis( tr( "Zoom in to canvas" ) );
  zoomInShortCut->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );

  QShortcut *zoomShortCut2 = new QShortcut( QKeySequence( tr( "Ctrl+=" ) ), this );
  connect( zoomShortCut2, &QShortcut::activated, mMapCanvas, &QgsMapCanvas::zoomIn );
  zoomShortCut2->setObjectName( QStringLiteral( "ZoomInToCanvas2" ) );
  zoomShortCut2->setWhatsThis( tr( "Zoom in to canvas (secondary)" ) );
  zoomShortCut2->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );

  QShortcut *zoomOutShortCut = new QShortcut( QKeySequence( tr( "Ctrl+-" ) ), this );
  connect( zoomOutShortCut, &QShortcut::activated, mMapCanvas, &QgsMapCanvas::zoomOut );
  zoomOutShortCut->setObjectName( QStringLiteral( "ZoomOutOfCanvas" ) );
  zoomOutShortCut->setWhatsThis( tr( "Zoom out of canvas" ) );
  zoomOutShortCut->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );

  //also make ctrl+alt+= a shortcut to switch to zoom in map tool
  QShortcut *zoomInToolShortCut = new QShortcut( QKeySequence( tr( "Ctrl+Alt+=" ) ), this );
  connect( zoomInToolShortCut, &QShortcut::activated, this, &QgisApp::zoomIn );
  zoomInToolShortCut->setObjectName( QStringLiteral( "ZoomIn2" ) );
  zoomInToolShortCut->setWhatsThis( tr( "Zoom in (secondary)" ) );
  zoomInToolShortCut->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );

  QShortcut *toggleSnapping = new QShortcut( QKeySequence( tr( "S", "Keyboard Shortcut: Toggle snapping" ) ), this );
  toggleSnapping->setObjectName( QStringLiteral( "toggleSnapping" ) );
  toggleSnapping->setWhatsThis( tr( "Toggle snapping" ) );
  toggleSnapping->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnapping.svg" ) ) );
  connect( toggleSnapping, &QShortcut::activated, mSnappingUtils, &QgsSnappingUtils::toggleEnabled );

  QShortcut *attributeTableSelected = new QShortcut( QKeySequence( tr( "Shift+F6" ) ), this );
  attributeTableSelected->setObjectName( QStringLiteral( "attributeTableSelectedFeatures" ) );
  attributeTableSelected->setWhatsThis( tr( "Open Attribute Table (Selected Features)" ) );
  attributeTableSelected->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  connect( attributeTableSelected, &QShortcut::activated, this, [ = ]
  {
    attributeTable( QgsAttributeTableFilterModel::ShowSelected );
  } );

  QShortcut *attributeTableVisible = new QShortcut( QKeySequence( tr( "Ctrl+F6" ) ), this );
  attributeTableVisible->setObjectName( QStringLiteral( "attributeTableVisibleFeatures" ) );
  attributeTableVisible->setWhatsThis( tr( "Open Attribute Table (Visible Features)" ) );
  attributeTableVisible->setProperty( "Icon", QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  connect( attributeTableVisible, &QShortcut::activated, this, [ = ]
  {
    attributeTable( QgsAttributeTableFilterModel::ShowVisible );
  } );

  if ( ! QTouchDevice::devices().isEmpty() )
  {
    //add reacting to long click in touch
    grabGesture( Qt::TapAndHoldGesture );
  }

  connect( QgsApplication::taskManager(), &QgsTaskManager::statusChanged, this, &QgisApp::onTaskCompleteShowNotify );

#ifdef Q_OS_WIN
  QWinTaskbarButton *taskButton = new QWinTaskbarButton( this );
  taskButton->setWindow( windowHandle() );

  QWinTaskbarProgress *taskProgress = taskButton->progress();
  taskProgress->setVisible( false );
  connect( QgsApplication::taskManager(), &QgsTaskManager::taskAdded, taskProgress, [taskProgress] { taskProgress->setMaximum( 0 ); taskProgress->show(); }
         );
  connect( QgsApplication::taskManager(), &QgsTaskManager::finalTaskProgressChanged, taskProgress, [taskProgress]( double val ) { taskProgress->setMaximum( 100 ); taskProgress->show(); taskProgress->setValue( val ); }
         );
  connect( QgsApplication::taskManager(), &QgsTaskManager::allTasksFinished, taskProgress, &QWinTaskbarProgress::hide );
#endif

  // supposedly all actions have been added, now register them to the shortcut manager
  QgsGui::shortcutsManager()->registerAllChildren( this );

  QgsProviderRegistry::instance()->registerGuis( this );

  setupLayoutManagerConnections();

  setupDuplicateFeaturesAction();

  // update windows
  qApp->processEvents();

  // notify user if authentication system is disabled
  ( void )QgsAuthGuiUtils::isDisabled( messageBar() );

  startProfile( QStringLiteral( "New project" ) );
  fileNewBlank(); // prepare empty project, also skips any default templates from loading
  endProfile();

  // request notification of FileOpen events (double clicking a file icon in Mac OS X Finder)
  // should come after fileNewBlank to ensure project is properly set up to receive any data source files
  QgsApplication::setFileOpenEventReceiver( this );


#ifdef ANDROID
  toggleFullScreen();
#endif
  profiler->endGroup();

  QgsDebugMsg( "PROFILE TIMES" );
  QgsDebugMsg( QString( "PROFILE TIMES TOTAL - %1 " ).arg( profiler->totalTime() ) );
#ifdef QGISDEBUG
  QList<QPair<QString, double> > profileTimes = profiler->profileTimes();
  QList<QPair<QString, double> >::const_iterator it = profileTimes.constBegin();
  for ( ; it != profileTimes.constEnd(); ++it )
  {
    QString name = ( *it ).first;
    double time = ( *it ).second;
    QgsDebugMsg( QString( " - %1 - %2" ).arg( name ).arg( time ) );
  }
#endif

} // QgisApp ctor

QgisApp::QgisApp()
  : QMainWindow( nullptr, nullptr )
#ifdef Q_OS_MAC
  , mWindowMenu( nullptr )
#endif
{
  sInstance = this;
  setupUi( this );
  mInternalClipboard = new QgsClipboard;
  mMapCanvas = new QgsMapCanvas();
  connect( mMapCanvas, &QgsMapCanvas::messageEmitted, this, &QgisApp::displayMessage );
  mMapCanvas->freeze();
  mLayerTreeView = new QgsLayerTreeView( this );
  mUndoWidget = new QgsUndoWidget( nullptr, mMapCanvas );
  mInfoBar = new QgsMessageBar( centralWidget() );
  mAdvancedDigitizingDockWidget = new QgsAdvancedDigitizingDockWidget( mMapCanvas, this );
  mPanelMenu = new QMenu( this );
  mProgressBar = new QProgressBar( this );
  mStatusBar = new QgsStatusBar( this );
  // More tests may need more members to be initialized
}

QgisApp::~QgisApp()
{
  stopRendering();

  delete mInternalClipboard;
  delete mQgisInterface;
  delete mStyleSheetBuilder;

  delete mMapTools.mZoomIn;
  delete mMapTools.mZoomOut;
  delete mMapTools.mPan;
  delete mMapTools.mAddPart;
  delete mMapTools.mAddRing;
  delete mMapTools.mFillRing;
  delete mMapTools.mAnnotation;
  delete mMapTools.mChangeLabelProperties;
  delete mMapTools.mDeletePart;
  delete mMapTools.mDeleteRing;
  delete mMapTools.mFeatureAction;
  delete mMapTools.mFormAnnotation;
  delete mMapTools.mHtmlAnnotation;
  delete mMapTools.mIdentify;
  delete mMapTools.mMeasureAngle;
  delete mMapTools.mMeasureArea;
  delete mMapTools.mMeasureDist;
  delete mMapTools.mMoveFeature;
  delete mMapTools.mMoveFeatureCopy;
  delete mMapTools.mMoveLabel;
  delete mMapTools.mNodeTool;
  delete mMapTools.mOffsetCurve;
  delete mMapTools.mPinLabels;
  delete mMapTools.mReshapeFeatures;
  delete mMapTools.mRotateFeature;
  delete mMapTools.mRotateLabel;
  delete mMapTools.mRotatePointSymbolsTool;
  delete mMapTools.mOffsetPointSymbolTool;
  delete mMapTools.mSelectFreehand;
  delete mMapTools.mSelectPolygon;
  delete mMapTools.mSelectRadius;
  delete mMapTools.mSelectFeatures;
  delete mMapTools.mShowHideLabels;
  delete mMapTools.mSimplifyFeature;
  delete mMapTools.mSplitFeatures;
  delete mMapTools.mSplitParts;
  delete mMapTools.mSvgAnnotation;
  delete mMapTools.mTextAnnotation;
  delete mMapTools.mCircularStringCurvePoint;
  delete mMapTools.mCircularStringRadius;
  delete mMapTools.mCircle2Points;
  delete mMapTools.mCircle3Points;
  delete mMapTools.mCircle3Tangents;
  delete mMapTools.mCircle2TangentsPoint;
  delete mMapTools.mCircleCenterPoint;
  delete mMapTools.mEllipseCenter2Points;
  delete mMapTools.mEllipseCenterPoint;
  delete mMapTools.mEllipseExtent;
  delete mMapTools.mEllipseFoci;
  delete mMapTools.mRectangleCenterPoint;
  delete mMapTools.mRectangleExtent;
  delete mMapTools.mRectangle3Points;
  delete mMapTools.mRegularPolygon2Points;
  delete mMapTools.mRegularPolygonCenterPoint;
  delete mMapTools.mRegularPolygonCenterCorner;
  delete mMapTools.mAddFeature;
  delete mpMaptip;

  delete mpGpsWidget;

  delete mOverviewMapCursor;

  delete mTracer;

  delete mVectorLayerTools;
  delete mWelcomePage;

  deleteLayoutDesigners();
  removeAnnotationItems();

  // cancel request for FileOpen events
  QgsApplication::setFileOpenEventReceiver( nullptr );

  unregisterCustomLayoutDropHandler( mLayoutQptDropHandler );

  delete mPythonUtils;
  delete mTray;
  delete mDataSourceManagerDialog;
  qDeleteAll( mCustomDropHandlers );
  qDeleteAll( mCustomLayoutDropHandlers );

  // This function *MUST* be the last one called, as it destroys in
  // particular GDAL. As above objects can hold GDAL/OGR objects, it is not
  // safe destroying them afterwards
  QgsApplication::exitQgis();
  // Do *NOT* add anything here !
}

void QgisApp::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasUrls() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    // the mime data are coming from layer tree, so ignore that, do not import those layers again
    if ( !event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
      event->acceptProposedAction();
  }
}

void QgisApp::dropEvent( QDropEvent *event )
{
  // dragging app is locked for the duration of dropEvent. This causes explorer windows to hang
  // while large projects/layers are loaded. So instead we return from dropEvent as quickly as possible
  // and do the actual handling of the drop after a very short timeout
  QTimer *timer = new QTimer( this );
  timer->setSingleShot( true );
  timer->setInterval( 50 );

  // first, allow custom handlers to directly operate on the mime data
  const QVector<QPointer<QgsCustomDropHandler >> handlers = mCustomDropHandlers;
  for ( QgsCustomDropHandler *handler : handlers )
  {
    if ( handler )
      handler->handleMimeData( event->mimeData() );
  }

  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  QStringList files;
  for ( i = urls.begin(); i != urls.end(); ++i )
  {
    QString fileName = i->toLocalFile();
#ifdef Q_OS_MAC
    // Mac OS X 10.10, under Qt4.8 ,changes dropped URL format
    // https://bugreports.qt.io/browse/QTBUG-40449
    // [pzion 20150805] Work around
    if ( fileName.startsWith( "/.file/id=" ) )
    {
      QgsDebugMsg( "Mac dropped URL with /.file/id= (converting)" );
      CFStringRef relCFStringRef =
        CFStringCreateWithCString(
          kCFAllocatorDefault,
          fileName.toUtf8().constData(),
          kCFStringEncodingUTF8
        );
      CFURLRef relCFURL =
        CFURLCreateWithFileSystemPath(
          kCFAllocatorDefault,
          relCFStringRef,
          kCFURLPOSIXPathStyle,
          false // isDirectory
        );
      CFErrorRef error = 0;
      CFURLRef absCFURL =
        CFURLCreateFilePathURL(
          kCFAllocatorDefault,
          relCFURL,
          &error
        );
      if ( !error )
      {
        static const CFIndex maxAbsPathCStrBufLen = 4096;
        char absPathCStr[maxAbsPathCStrBufLen];
        if ( CFURLGetFileSystemRepresentation(
               absCFURL,
               true, // resolveAgainstBase
               reinterpret_cast<UInt8 *>( &absPathCStr[0] ),
               maxAbsPathCStrBufLen ) )
        {
          fileName = QString( absPathCStr );
        }
      }
      CFRelease( absCFURL );
      CFRelease( relCFURL );
      CFRelease( relCFStringRef );
    }
#endif
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if ( !fileName.isEmpty() )
    {
      files << fileName;
    }
  }

  QgsMimeDataUtils::UriList lst;
  if ( QgsMimeDataUtils::isUriList( event->mimeData() ) )
  {
    lst = QgsMimeDataUtils::decodeUriList( event->mimeData() );
  }

  connect( timer, &QTimer::timeout, this, [this, timer, files, lst]
  {
    freezeCanvases();

    for ( const QString &file : qgis::as_const( files ) )
    {
      bool handled = false;

      // give custom drop handlers first priority at handling the file
      const QVector<QPointer<QgsCustomDropHandler >> handlers = mCustomDropHandlers;
      for ( QgsCustomDropHandler *handler : handlers )
      {
        if ( handler && handler->handleFileDrop( file ) )
        {
          handled = true;
          break;
        }
      }

      if ( !handled )
        openFile( file );
    }

    if ( !lst.isEmpty() )
    {
      handleDropUriList( lst );
    }

    freezeCanvases( false );
    refreshMapCanvas();

    timer->deleteLater();
  } );

  event->acceptProposedAction();
  timer->start();
}

void QgisApp::annotationCreated( QgsAnnotation *annotation )
{
  // create canvas annotation item for annotation
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
  {
    QgsMapCanvasAnnotationItem *canvasItem = new QgsMapCanvasAnnotationItem( annotation, canvas );
    Q_UNUSED( canvasItem ); //item is already added automatically to canvas scene
  }
}

void QgisApp::registerCustomDropHandler( QgsCustomDropHandler *handler )
{
  if ( !mCustomDropHandlers.contains( handler ) )
    mCustomDropHandlers << handler;
}

void QgisApp::unregisterCustomDropHandler( QgsCustomDropHandler *handler )
{
  mCustomDropHandlers.removeOne( handler );
}

void QgisApp::registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler )
{
  if ( !mCustomLayoutDropHandlers.contains( handler ) )
    mCustomLayoutDropHandlers << handler;
}

void QgisApp::unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler )
{
  mCustomLayoutDropHandlers.removeOne( handler );
}

QVector<QPointer<QgsLayoutCustomDropHandler> > QgisApp::customLayoutDropHandlers() const
{
  return mCustomLayoutDropHandlers;
}

void QgisApp::handleDropUriList( const QgsMimeDataUtils::UriList &lst )
{
  // insert items in reverse order as each one is inserted on top of previous one
  for ( int i = lst.size() - 1 ; i >= 0 ; i-- )
  {
    const QgsMimeDataUtils::Uri &u = lst.at( i );

    QString uri = crsAndFormatAdjustedLayerUri( u.uri, u.supportedCrs, u.supportedFormats );

    if ( u.layerType == QLatin1String( "vector" ) )
    {
      addVectorLayer( uri, u.name, u.providerKey );
    }
    else if ( u.layerType == QLatin1String( "raster" ) )
    {
      addRasterLayer( uri, u.name, u.providerKey );
    }
    else if ( u.layerType == QLatin1String( "plugin" ) )
    {
      addPluginLayer( uri, u.name, u.providerKey );
    }
    else if ( u.layerType == QLatin1String( "custom" ) )
    {
      Q_FOREACH ( QgsCustomDropHandler *handler, mCustomDropHandlers )
      {
        if ( handler && handler->customUriProviderKey() == u.providerKey )
        {
          handler->handleCustomUriDrop( u );
          break;
        }
      }
    }
  }
}


bool QgisApp::event( QEvent *event )
{
  bool done = false;
  if ( event->type() == QEvent::FileOpen )
  {
    // handle FileOpen event (double clicking a file icon in Mac OS X Finder)
    QFileOpenEvent *foe = static_cast<QFileOpenEvent *>( event );
    openFile( foe->file() );
    done = true;
  }
  else if ( !QTouchDevice::devices().isEmpty() && event->type() == QEvent::Gesture )
  {
    done = gestureEvent( static_cast<QGestureEvent *>( event ) );
  }
  else
  {
    // pass other events to base class
    done = QMainWindow::event( event );
  }
  return done;
}

void QgisApp::dataSourceManager( const QString &pageName )
{
  if ( ! mDataSourceManagerDialog )
  {
    mDataSourceManagerDialog = new QgsDataSourceManagerDialog( mBrowserModel, this, mapCanvas() );
    connect( this, &QgisApp::connectionsChanged, mDataSourceManagerDialog, &QgsDataSourceManagerDialog::refresh );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::connectionsChanged, this, &QgisApp::connectionsChanged );
    connect( mDataSourceManagerDialog, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
             this, SLOT( addRasterLayer( QString const &, QString const &, QString const & ) ) );
    connect( mDataSourceManagerDialog, SIGNAL( addVectorLayer( QString const &, QString const &, QString const & ) ),
             this, SLOT( addVectorLayer( QString const &, QString const &, QString const & ) ) );
    connect( mDataSourceManagerDialog, SIGNAL( addVectorLayers( QStringList const &, QString const &, QString const & ) ),
             this, SLOT( addVectorLayers( QStringList const &, QString const &, QString const & ) ) );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::showProgress, this, &QgisApp::showProgress );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::showStatusMessage, this, &QgisApp::showStatusMessage );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::addDatabaseLayers, this, &QgisApp::addDatabaseLayers );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::replaceSelectedVectorLayer, this, &QgisApp::replaceSelectedVectorLayer );
    connect( mDataSourceManagerDialog, static_cast<void ( QgsDataSourceManagerDialog::* )()>( &QgsDataSourceManagerDialog::addRasterLayer ), this, static_cast<void ( QgisApp::* )()>( &QgisApp::addRasterLayer ) );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::handleDropUriList, this, &QgisApp::handleDropUriList );
    connect( this, &QgisApp::newProject, mDataSourceManagerDialog, &QgsDataSourceManagerDialog::updateProjectHome );
    connect( mDataSourceManagerDialog, &QgsDataSourceManagerDialog::openFile, this, &QgisApp::openFile );

  }
  // Try to open the dialog on a particular page
  if ( ! pageName.isEmpty() )
  {
    mDataSourceManagerDialog->openPage( pageName );
  }
  if ( QgsSettings().value( QStringLiteral( "/qgis/dataSourceManagerNonModal" ), true ).toBool() )
  {
    mDataSourceManagerDialog->show();
  }
  else
  {
    mDataSourceManagerDialog->exec();
  }
}

QgisAppStyleSheet *QgisApp::styleSheetBuilder()
{
  Q_ASSERT( mStyleSheetBuilder );
  return mStyleSheetBuilder;
}

void QgisApp::readRecentProjects()
{
  QgsSettings settings;
  mRecentProjects.clear();

  settings.beginGroup( QStringLiteral( "UI" ) );

  // Migrate old recent projects if first time with new system
  if ( !settings.childGroups().contains( QStringLiteral( "recentProjects" ) ) )
  {
    QStringList oldRecentProjects = settings.value( QStringLiteral( "UI/recentProjectsList" ) ).toStringList();

    Q_FOREACH ( const QString &project, oldRecentProjects )
    {
      QgsWelcomePageItemsModel::RecentProjectData data;
      data.path = project;
      data.title = project;

      mRecentProjects.append( data );
    }
  }
  settings.endGroup();

  settings.beginGroup( QStringLiteral( "UI/recentProjects" ) );
  QStringList projectKeysList = settings.childGroups();

  //convert list to int values to obtain proper order
  QList<int> projectKeys;
  Q_FOREACH ( const QString &key, projectKeysList )
  {
    projectKeys.append( key.toInt() );
  }
  std::sort( projectKeys.begin(), projectKeys.end() );

  int pinPos = 0;
  Q_FOREACH ( int key, projectKeys )
  {
    QgsWelcomePageItemsModel::RecentProjectData data;
    settings.beginGroup( QString::number( key ) );
    data.title = settings.value( QStringLiteral( "title" ) ).toString();
    data.path = settings.value( QStringLiteral( "path" ) ).toString();
    data.previewImagePath = settings.value( QStringLiteral( "previewImage" ) ).toString();
    data.crs = settings.value( QStringLiteral( "crs" ) ).toString();
    data.pin = settings.value( QStringLiteral( "pin" ) ).toBool();
    settings.endGroup();
    if ( data.pin )
    {
      mRecentProjects.insert( pinPos, data );
      pinPos++;
    }
    else
    {
      mRecentProjects.append( data );
    }
  }
  settings.endGroup();
}

void QgisApp::applyProjectSettingsToCanvas( QgsMapCanvas *canvas )
{
  int red = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
  int green = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
  int blue = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
  QColor myColor = QColor( red, green, blue );
  canvas->setCanvasColor( myColor );

  int alpha = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), 255 );
  red = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), 255 );
  green = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), 255 );
  blue = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), 0 );
  myColor = QColor( red, green, blue, alpha );
  canvas->setSelectionColor( myColor );
}

void QgisApp::applyDefaultSettingsToCanvas( QgsMapCanvas *canvas )
{
  QgsSettings settings;
  canvas->enableAntiAliasing( settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();
  canvas->setWheelFactor( zoomFactor );
  canvas->setCachingEnabled( settings.value( QStringLiteral( "qgis/enable_render_caching" ), true ).toBool() );
  canvas->setParallelRenderingEnabled( settings.value( QStringLiteral( "qgis/parallel_rendering" ), true ).toBool() );
  canvas->setMapUpdateInterval( settings.value( QStringLiteral( "qgis/map_update_interval" ), 250 ).toInt() );
  canvas->setSegmentationTolerance( settings.value( QStringLiteral( "qgis/segmentationTolerance" ), "0.01745" ).toDouble() );
  canvas->setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType( settings.value( QStringLiteral( "qgis/segmentationToleranceType" ), "0" ).toInt() ) );
}

int QgisApp::chooseReasonableDefaultIconSize() const
{
  QScreen *screen = QApplication::screens().at( 0 );
  if ( screen->physicalDotsPerInch() < 115 )
  {
    // no hidpi screen, use default size
    return QGIS_ICON_SIZE;
  }
  else
  {
    double size = fontMetrics().width( QStringLiteral( "XXX" ) );
    if ( size < 24 )
      return 16;
    else if ( size < 32 )
      return 24;
    else if ( size < 48 )
      return 32;
    else if ( size < 64 )
      return 48;
    else
      return 64;
  }

}

int QgisApp::dockedToolbarIconSize( int standardToolbarIconSize ) const
{
  int dockSize;
  if ( standardToolbarIconSize > 32 )
  {
    dockSize = standardToolbarIconSize - 16;
  }
  else if ( standardToolbarIconSize == 32 )
  {
    dockSize = 24;
  }
  else
  {
    dockSize = 16;
  }
  return dockSize;
}

void QgisApp::readSettings()
{
  QgsSettings settings;
  QString themename = settings.value( QStringLiteral( "UI/UITheme" ), "default" ).toString();
  setTheme( themename );

  // Read legacy settings
  readRecentProjects();

  // this is a new session! reset enable macros value to "ask"
  // whether set to "just for this session"
  if ( settings.value( QStringLiteral( "qgis/enableMacros" ), 1 ).toInt() == 2 )
  {
    settings.setValue( QStringLiteral( "qgis/enableMacros" ), 1 );
  }
}


//////////////////////////////////////////////////////////////////////
//            Set Up the gui toolbars, menus, statusbar etc
//////////////////////////////////////////////////////////////////////

void QgisApp::createActions()
{
  mActionPluginSeparator1 = nullptr;  // plugin list separator will be created when the first plugin is loaded
  mActionPluginSeparator2 = nullptr;  // python separator will be created only if python is found
  mActionRasterSeparator = nullptr;   // raster plugins list separator will be created when the first plugin is loaded

  // Project Menu Items

  connect( mActionNewProject, &QAction::triggered, this, [ = ] { fileNew(); } );
  connect( mActionNewBlankProject, &QAction::triggered, this, &QgisApp::fileNewBlank );
  connect( mActionOpenProject, &QAction::triggered, this, &QgisApp::fileOpen );
  connect( mActionSaveProject, &QAction::triggered, this, &QgisApp::fileSave );
  connect( mActionSaveProjectAs, &QAction::triggered, this, &QgisApp::fileSaveAs );
  connect( mActionSaveMapAsImage, &QAction::triggered, this, [ = ] { saveMapAsImage(); } );
  connect( mActionSaveMapAsPdf, &QAction::triggered, this, [ = ] { saveMapAsPdf(); } );
  connect( mActionNewMapCanvas, &QAction::triggered, this, &QgisApp::newMapCanvas );
  connect( mActionNew3DMapCanvas, &QAction::triggered, this, &QgisApp::new3DMapCanvas );
  connect( mActionNewPrintLayout, &QAction::triggered, this, &QgisApp::newPrintLayout );
  connect( mActionNewReport, &QAction::triggered, this, &QgisApp::newReport );
  connect( mActionShowLayoutManager, &QAction::triggered, this, &QgisApp::showLayoutManager );
  connect( mActionExit, &QAction::triggered, this, &QgisApp::fileExit );
  connect( mActionDxfExport, &QAction::triggered, this, &QgisApp::dxfExport );
  connect( mActionDwgImport, &QAction::triggered, this, &QgisApp::dwgImport );

  // Edit Menu Items

  connect( mActionUndo, &QAction::triggered, mUndoWidget, &QgsUndoWidget::undo );
  connect( mActionRedo, &QAction::triggered, mUndoWidget, &QgsUndoWidget::redo );
  connect( mActionCutFeatures, &QAction::triggered, this, [ = ] { cutSelectionToClipboard(); } );
  connect( mActionCopyFeatures, &QAction::triggered, this, [ = ] { copySelectionToClipboard(); } );
  connect( mActionPasteFeatures, &QAction::triggered, this, [ = ] { pasteFromClipboard(); } );
  connect( mActionPasteAsNewVector, &QAction::triggered, this, &QgisApp::pasteAsNewVector );
  connect( mActionPasteAsNewMemoryVector, &QAction::triggered, this, [ = ] { pasteAsNewMemoryVector(); } );
  connect( mActionCopyStyle, &QAction::triggered, this, [ = ] { copyStyle(); } );
  connect( mActionPasteStyle, &QAction::triggered, this, [ = ] { pasteStyle(); } );
  connect( mActionAddFeature, &QAction::triggered, this, &QgisApp::addFeature );
  connect( mActionCircularStringCurvePoint, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircularStringCurvePoint ); } );
  connect( mActionCircularStringRadius, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircularStringRadius ); } );
  connect( mActionCircle2Points, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircle2Points, true ); } );
  connect( mActionCircle3Points, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircle3Points, true ); } );
  connect( mActionCircle3Tangents, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircle3Tangents, true ); } );
  connect( mActionCircle2TangentsPoint, &QAction::triggered, this, [ = ] { setMapTool( mMapTools.mCircle2TangentsPoint, true ); } );
  connect( mActionCircleCenterPoint, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mCircleCenterPoint, true ); } );
  connect( mActionEllipseCenter2Points, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mEllipseCenter2Points, true ); } );
  connect( mActionEllipseCenterPoint, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mEllipseCenterPoint, true ); } );
  connect( mActionEllipseExtent, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mEllipseExtent, true ); } );
  connect( mActionEllipseFoci, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mEllipseFoci, true ); } );
  connect( mActionRectangleCenterPoint, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRectangleCenterPoint, true ); } );
  connect( mActionRectangleExtent, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRectangleExtent, true ); } );
  connect( mActionRectangle3Points, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRectangle3Points, true ); } );
  connect( mActionRegularPolygon2Points, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRegularPolygon2Points, true ); } );
  connect( mActionRegularPolygonCenterPoint, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRegularPolygonCenterPoint, true ); } );
  connect( mActionRegularPolygonCenterCorner, &QAction::triggered, this,  [ = ] { setMapTool( mMapTools.mRegularPolygonCenterCorner, true ); } );
  connect( mActionMoveFeature, &QAction::triggered, this, &QgisApp::moveFeature );
  connect( mActionMoveFeature, &QAction::triggered, this, &QgisApp::moveFeature );
  connect( mActionMoveFeatureCopy, &QAction::triggered, this, &QgisApp::moveFeatureCopy );
  connect( mActionRotateFeature, &QAction::triggered, this, &QgisApp::rotateFeature );

  connect( mActionReshapeFeatures, &QAction::triggered, this, &QgisApp::reshapeFeatures );
  connect( mActionSplitFeatures, &QAction::triggered, this, &QgisApp::splitFeatures );
  connect( mActionSplitParts, &QAction::triggered, this, &QgisApp::splitParts );
  connect( mActionDeleteSelected, &QAction::triggered, this, [ = ] { deleteSelected(); } );
  connect( mActionAddRing, &QAction::triggered, this, &QgisApp::addRing );
  connect( mActionFillRing, &QAction::triggered, this, &QgisApp::fillRing );
  connect( mActionAddPart, &QAction::triggered, this, &QgisApp::addPart );
  connect( mActionSimplifyFeature, &QAction::triggered, this, &QgisApp::simplifyFeature );
  connect( mActionDeleteRing, &QAction::triggered, this, &QgisApp::deleteRing );
  connect( mActionDeletePart, &QAction::triggered, this, &QgisApp::deletePart );
  connect( mActionMergeFeatures, &QAction::triggered, this, &QgisApp::mergeSelectedFeatures );
  connect( mActionMergeFeatureAttributes, &QAction::triggered, this, &QgisApp::mergeAttributesOfSelectedFeatures );
  connect( mActionMultiEditAttributes, &QAction::triggered, this, &QgisApp::modifyAttributesOfSelectedFeatures );
  connect( mActionNodeTool, &QAction::triggered, this, &QgisApp::nodeTool );
  connect( mActionRotatePointSymbols, &QAction::triggered, this, &QgisApp::rotatePointSymbols );
  connect( mActionOffsetPointSymbol, &QAction::triggered, this, &QgisApp::offsetPointSymbol );
  connect( mActionSnappingOptions, &QAction::triggered, this, &QgisApp::snappingOptions );
  connect( mActionOffsetCurve, &QAction::triggered, this, &QgisApp::offsetCurve );

  // View Menu Items
  connect( mActionPan, &QAction::triggered, this, &QgisApp::pan );
  connect( mActionPanToSelected, &QAction::triggered, this, &QgisApp::panToSelected );
  connect( mActionZoomIn, &QAction::triggered, this, &QgisApp::zoomIn );
  connect( mActionZoomOut, &QAction::triggered, this, &QgisApp::zoomOut );
  connect( mActionSelectFeatures, &QAction::triggered, this, &QgisApp::selectFeatures );
  connect( mActionSelectPolygon, &QAction::triggered, this, &QgisApp::selectByPolygon );
  connect( mActionSelectFreehand, &QAction::triggered, this, &QgisApp::selectByFreehand );
  connect( mActionSelectRadius, &QAction::triggered, this, &QgisApp::selectByRadius );
  connect( mActionDeselectAll, &QAction::triggered, this, &QgisApp::deselectAll );
  connect( mActionSelectAll, &QAction::triggered, this, &QgisApp::selectAll );
  connect( mActionInvertSelection, &QAction::triggered, this, &QgisApp::invertSelection );
  connect( mActionSelectByExpression, &QAction::triggered, this, &QgisApp::selectByExpression );
  connect( mActionSelectByForm, &QAction::triggered, this, &QgisApp::selectByForm );
  connect( mActionIdentify, &QAction::triggered, this, &QgisApp::identify );
  connect( mActionFeatureAction, &QAction::triggered, this, &QgisApp::doFeatureAction );
  connect( mActionMeasure, &QAction::triggered, this, &QgisApp::measure );
  connect( mActionMeasureArea, &QAction::triggered, this, &QgisApp::measureArea );
  connect( mActionMeasureAngle, &QAction::triggered, this, &QgisApp::measureAngle );
  connect( mActionZoomFullExtent, &QAction::triggered, this, &QgisApp::zoomFull );
  connect( mActionZoomToLayer, &QAction::triggered, this, &QgisApp::zoomToLayerExtent );
  connect( mActionZoomToSelected, &QAction::triggered, this, &QgisApp::zoomToSelected );
  connect( mActionZoomLast, &QAction::triggered, this, &QgisApp::zoomToPrevious );
  connect( mActionZoomNext, &QAction::triggered, this, &QgisApp::zoomToNext );
  connect( mActionZoomActualSize, &QAction::triggered, this, &QgisApp::zoomActualSize );
  connect( mActionMapTips, &QAction::toggled, this, &QgisApp::toggleMapTips );
  connect( mActionNewBookmark, &QAction::triggered, this, &QgisApp::newBookmark );
  connect( mActionShowBookmarks, &QAction::triggered, this, &QgisApp::showBookmarks );
  connect( mActionDraw, &QAction::triggered, this, &QgisApp::refreshMapCanvas );
  connect( mActionTextAnnotation, &QAction::triggered, this, &QgisApp::addTextAnnotation );
  connect( mActionFormAnnotation, &QAction::triggered, this, &QgisApp::addFormAnnotation );
  connect( mActionHtmlAnnotation, &QAction::triggered, this, &QgisApp::addHtmlAnnotation );
  connect( mActionSvgAnnotation, &QAction::triggered, this, &QgisApp::addSvgAnnotation );
  connect( mActionAnnotation, &QAction::triggered, this, &QgisApp::modifyAnnotation );
  connect( mActionLabeling, &QAction::triggered, this, &QgisApp::labeling );
  connect( mActionStatisticalSummary, &QAction::triggered, this, &QgisApp::showStatisticsDockWidget );

  // Layer Menu Items

  connect( mActionDataSourceManager, &QAction::triggered, this, [ = ]() { dataSourceManager(); } );
  connect( mActionNewVectorLayer, &QAction::triggered, this, &QgisApp::newVectorLayer );
  connect( mActionNewSpatiaLiteLayer, &QAction::triggered, this, &QgisApp::newSpatialiteLayer );
  connect( mActionNewGeoPackageLayer, &QAction::triggered, this, &QgisApp::newGeoPackageLayer );
  connect( mActionNewMemoryLayer, &QAction::triggered, this, &QgisApp::newMemoryLayer );
  connect( mActionShowRasterCalculator, &QAction::triggered, this, &QgisApp::showRasterCalculator );
  connect( mActionShowAlignRasterTool, &QAction::triggered, this, &QgisApp::showAlignRasterTool );
  connect( mActionEmbedLayers, &QAction::triggered, this, &QgisApp::embedLayers );
  connect( mActionAddLayerDefinition, &QAction::triggered, this, &QgisApp::addLayerDefinition );
  connect( mActionAddOgrLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "ogr" ) ); } );
  connect( mActionAddRasterLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "gdal" ) ); } );
  connect( mActionAddPgLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "postgres" ) ); } );
  connect( mActionAddSpatiaLiteLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "spatialite" ) ); } );
  connect( mActionAddMssqlLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "mssql" ) ); } );
  connect( mActionAddDb2Layer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "DB2" ) ); } );
  connect( mActionAddOracleLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "oracle" ) ); } );
  connect( mActionAddWmsLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "wms" ) ); } );
  connect( mActionAddWcsLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "wcs" ) ); } );
  connect( mActionAddWfsLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "WFS" ) ); } );
  connect( mActionAddAfsLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "arcgisfeatureserver" ) ); } );
  connect( mActionAddAmsLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "arcgismapserver" ) ); } );
  connect( mActionAddDelimitedText, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "delimitedtext" ) ); } );
  connect( mActionAddVirtualLayer, &QAction::triggered, this, [ = ] { dataSourceManager( QStringLiteral( "virtual" ) ); } );
  connect( mActionOpenTable, &QAction::triggered, this, [ = ]
  {
    QgsSettings settings;
    QgsAttributeTableFilterModel::FilterMode initialMode = static_cast< QgsAttributeTableFilterModel::FilterMode>( settings.value( QStringLiteral( "qgis/attributeTableBehavior" ), QgsAttributeTableFilterModel::ShowAll ).toInt() );
    attributeTable( initialMode );
  } );
  connect( mActionOpenFieldCalc, &QAction::triggered, this, &QgisApp::fieldCalculator );
  connect( mActionToggleEditing, &QAction::triggered, this, [ = ] { toggleEditing(); } );
  connect( mActionSaveLayerEdits, &QAction::triggered, this, &QgisApp::saveActiveLayerEdits );
  connect( mActionSaveEdits, &QAction::triggered, this, [ = ] { saveEdits(); } );
  connect( mActionSaveAllEdits, &QAction::triggered, this, &QgisApp::saveAllEdits );
  connect( mActionRollbackEdits, &QAction::triggered, this, &QgisApp::rollbackEdits );
  connect( mActionRollbackAllEdits, &QAction::triggered, this, &QgisApp::rollbackAllEdits );
  connect( mActionCancelEdits, &QAction::triggered, this, [ = ] { cancelEdits(); } );
  connect( mActionCancelAllEdits, &QAction::triggered, this, &QgisApp::cancelAllEdits );
  connect( mActionLayerSaveAs, &QAction::triggered, this, [ = ] { saveAsFile(); } );
  connect( mActionSaveLayerDefinition, &QAction::triggered, this, &QgisApp::saveAsLayerDefinition );
  connect( mActionRemoveLayer, &QAction::triggered, this, &QgisApp::removeLayer );
  connect( mActionDuplicateLayer, &QAction::triggered, this, [ = ] { duplicateLayers(); } );
  connect( mActionSetLayerScaleVisibility, &QAction::triggered, this, &QgisApp::setLayerScaleVisibility );
  connect( mActionSetLayerCRS, &QAction::triggered, this, &QgisApp::setLayerCrs );
  connect( mActionSetProjectCRSFromLayer, &QAction::triggered, this, &QgisApp::setProjectCrsFromLayer );
  connect( mActionLayerProperties, &QAction::triggered, this, &QgisApp::layerProperties );
  connect( mActionLayerSubsetString, &QAction::triggered, this, &QgisApp::layerSubsetString );
  connect( mActionAddToOverview, &QAction::triggered, this, &QgisApp::isInOverview );
  connect( mActionAddAllToOverview, &QAction::triggered, this, &QgisApp::addAllToOverview );
  connect( mActionRemoveAllFromOverview, &QAction::triggered, this, &QgisApp::removeAllFromOverview );
  connect( mActionShowAllLayers, &QAction::triggered, this, &QgisApp::showAllLayers );
  connect( mActionHideAllLayers, &QAction::triggered, this, &QgisApp::hideAllLayers );
  connect( mActionShowSelectedLayers, &QAction::triggered, this, &QgisApp::showSelectedLayers );
  connect( mActionHideSelectedLayers, &QAction::triggered, this, &QgisApp::hideSelectedLayers );
  connect( mActionHideDeselectedLayers, &QAction::triggered, this, &QgisApp::hideDeselectedLayers );

  // Plugin Menu Items

  connect( mActionManagePlugins, &QAction::triggered, this, &QgisApp::showPluginManager );
  connect( mActionShowPythonDialog, &QAction::triggered, this, &QgisApp::showPythonDialog );

  // Settings Menu Items

  connect( mActionToggleFullScreen, &QAction::triggered, this, &QgisApp::toggleFullScreen );
  connect( mActionTogglePanelsVisibility, &QAction::triggered, this, &QgisApp::togglePanelsVisibility );
  connect( mActionProjectProperties, &QAction::triggered, this, &QgisApp::projectProperties );
  connect( mActionOptions, &QAction::triggered, this, &QgisApp::options );
  connect( mActionCustomProjection, &QAction::triggered, this, &QgisApp::customProjection );
  connect( mActionConfigureShortcuts, &QAction::triggered, this, &QgisApp::configureShortcuts );
  connect( mActionStyleManager, &QAction::triggered, this, &QgisApp::showStyleManager );
  connect( mActionCustomization, &QAction::triggered, this, &QgisApp::customize );

#ifdef Q_OS_MAC
  // Window Menu Items

  mActionWindowMinimize = new QAction( tr( "Minimize" ), this );
  mActionWindowMinimize->setShortcut( tr( "Ctrl+M", "Minimize Window" ) );
  mActionWindowMinimize->setStatusTip( tr( "Minimizes the active window to the dock" ) );
  connect( mActionWindowMinimize, SIGNAL( triggered() ), this, SLOT( showActiveWindowMinimized() ) );

  mActionWindowZoom = new QAction( tr( "Zoom" ), this );
  mActionWindowZoom->setStatusTip( tr( "Toggles between a predefined size and the window size set by the user" ) );
  connect( mActionWindowZoom, SIGNAL( triggered() ), this, SLOT( toggleActiveWindowMaximized() ) );

  mActionWindowAllToFront = new QAction( tr( "Bring All to Front" ), this );
  mActionWindowAllToFront->setStatusTip( tr( "Bring forward all open windows" ) );
  connect( mActionWindowAllToFront, SIGNAL( triggered() ), this, SLOT( bringAllToFront() ) );

  // list of open windows
  mWindowActions = new QActionGroup( this );
#endif

  // Vector edits menu
  QMenu *menuAllEdits = new QMenu( tr( "Current Edits" ), this );
  menuAllEdits->addAction( mActionSaveEdits );
  menuAllEdits->addAction( mActionRollbackEdits );
  menuAllEdits->addAction( mActionCancelEdits );
  menuAllEdits->addSeparator();
  menuAllEdits->addAction( mActionSaveAllEdits );
  menuAllEdits->addAction( mActionRollbackAllEdits );
  menuAllEdits->addAction( mActionCancelAllEdits );
  mActionAllEdits->setMenu( menuAllEdits );

  // Raster toolbar items
  connect( mActionLocalHistogramStretch, &QAction::triggered, this, &QgisApp::localHistogramStretch );
  connect( mActionFullHistogramStretch, &QAction::triggered, this, &QgisApp::fullHistogramStretch );
  connect( mActionLocalCumulativeCutStretch, &QAction::triggered, this, &QgisApp::localCumulativeCutStretch );
  connect( mActionFullCumulativeCutStretch, &QAction::triggered, this, &QgisApp::fullCumulativeCutStretch );
  connect( mActionIncreaseBrightness, &QAction::triggered, this, &QgisApp::increaseBrightness );
  connect( mActionDecreaseBrightness, &QAction::triggered, this, &QgisApp::decreaseBrightness );
  connect( mActionIncreaseContrast, &QAction::triggered, this, &QgisApp::increaseContrast );
  connect( mActionDecreaseContrast, &QAction::triggered, this, &QgisApp::decreaseContrast );

  // Help Menu Items

#ifdef Q_OS_MAC
  mActionHelpContents->setShortcut( QString( "Ctrl+?" ) );
  mActionQgisHomePage->setShortcut( QString() );
  mActionReportaBug->setShortcut( QString() );
#endif

  mActionHelpContents->setEnabled( QFileInfo::exists( QgsApplication::pkgDataPath() + "/doc/index.html" ) );

  connect( mActionHelpContents, &QAction::triggered, this, &QgisApp::helpContents );
  connect( mActionHelpAPI, &QAction::triggered, this, &QgisApp::apiDocumentation );
  connect( mActionReportaBug, &QAction::triggered, this, &QgisApp::reportaBug );
  connect( mActionNeedSupport, &QAction::triggered, this, &QgisApp::supportProviders );
  connect( mActionQgisHomePage, &QAction::triggered, this, &QgisApp::helpQgisHomePage );
  connect( mActionCheckQgisVersion, &QAction::triggered, this, &QgisApp::checkQgisVersion );
  connect( mActionAbout, &QAction::triggered, this, &QgisApp::about );
  connect( mActionSponsors, &QAction::triggered, this, &QgisApp::sponsors );

  connect( mActionShowPinnedLabels, &QAction::toggled, this, &QgisApp::showPinnedLabels );
  connect( mActionPinLabels, &QAction::triggered, this, &QgisApp::pinLabels );
  connect( mActionShowHideLabels, &QAction::triggered, this, &QgisApp::showHideLabels );
  connect( mActionMoveLabel, &QAction::triggered, this, &QgisApp::moveLabel );
  connect( mActionRotateLabel, &QAction::triggered, this, &QgisApp::rotateLabel );
  connect( mActionChangeLabelProperties, &QAction::triggered, this, &QgisApp::changeLabelProperties );

  connect( mActionDiagramProperties, &QAction::triggered, this, &QgisApp::diagramProperties );

#ifndef HAVE_POSTGRESQL
  delete mActionAddPgLayer;
  mActionAddPgLayer = 0;
#endif

#ifndef HAVE_ORACLE
  delete mActionAddOracleLayer;
  mActionAddOracleLayer = nullptr;
#endif

}

#include "qgsstyle.h"
#include "qgsstylemanagerdialog.h"

void QgisApp::showStyleManager()
{
  QgsStyleManagerDialog dlg( QgsStyle::defaultStyle(), this );
  dlg.exec();
}

void QgisApp::showPythonDialog()
{
#ifdef WITH_BINDINGS
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
    return;

  bool res = mPythonUtils->runString(
               "import console\n"
               "console.show_console()\n", tr( "Failed to open Python console:" ), false );

  if ( !res )
  {
    QString className, text;
    mPythonUtils->getError( className, text );
    messageBar()->pushMessage( tr( "Error" ), tr( "Failed to open Python console:" ) + '\n' + className + ": " + text, QgsMessageBar::WARNING );
  }
#ifdef Q_OS_MAC
  else
  {
    addWindow( mActionShowPythonDialog );
  }
#endif
#endif
}

void QgisApp::createActionGroups()
{
  //
  // Map Tool Group
  mMapToolGroup = new QActionGroup( this );
  mMapToolGroup->addAction( mActionPan );
  mMapToolGroup->addAction( mActionZoomIn );
  mMapToolGroup->addAction( mActionZoomOut );
  mMapToolGroup->addAction( mActionIdentify );
  mMapToolGroup->addAction( mActionFeatureAction );
  mMapToolGroup->addAction( mActionSelectFeatures );
  mMapToolGroup->addAction( mActionSelectPolygon );
  mMapToolGroup->addAction( mActionSelectFreehand );
  mMapToolGroup->addAction( mActionSelectRadius );
  mMapToolGroup->addAction( mActionDeselectAll );
  mMapToolGroup->addAction( mActionSelectAll );
  mMapToolGroup->addAction( mActionInvertSelection );
  mMapToolGroup->addAction( mActionMeasure );
  mMapToolGroup->addAction( mActionMeasureArea );
  mMapToolGroup->addAction( mActionMeasureAngle );
  mMapToolGroup->addAction( mActionAddFeature );
  mMapToolGroup->addAction( mActionCircularStringCurvePoint );
  mMapToolGroup->addAction( mActionCircularStringRadius );
  mMapToolGroup->addAction( mActionCircle2Points );
  mMapToolGroup->addAction( mActionCircle3Points );
  mMapToolGroup->addAction( mActionCircle3Tangents );
  mMapToolGroup->addAction( mActionCircle2TangentsPoint );
  mMapToolGroup->addAction( mActionCircleCenterPoint );
  mMapToolGroup->addAction( mActionEllipseCenter2Points );
  mMapToolGroup->addAction( mActionEllipseCenterPoint );
  mMapToolGroup->addAction( mActionEllipseExtent );
  mMapToolGroup->addAction( mActionEllipseFoci );
  mMapToolGroup->addAction( mActionRectangleCenterPoint );
  mMapToolGroup->addAction( mActionRectangleExtent );
  mMapToolGroup->addAction( mActionRectangle3Points );
  mMapToolGroup->addAction( mActionRegularPolygon2Points );
  mMapToolGroup->addAction( mActionRegularPolygonCenterPoint );
  mMapToolGroup->addAction( mActionRegularPolygonCenterCorner );
  mMapToolGroup->addAction( mActionMoveFeature );
  mMapToolGroup->addAction( mActionMoveFeatureCopy );
  mMapToolGroup->addAction( mActionRotateFeature );
  mMapToolGroup->addAction( mActionOffsetCurve );
  mMapToolGroup->addAction( mActionReshapeFeatures );
  mMapToolGroup->addAction( mActionSplitFeatures );
  mMapToolGroup->addAction( mActionSplitParts );
  mMapToolGroup->addAction( mActionDeleteSelected );
  mMapToolGroup->addAction( mActionAddRing );
  mMapToolGroup->addAction( mActionFillRing );
  mMapToolGroup->addAction( mActionAddPart );
  mMapToolGroup->addAction( mActionSimplifyFeature );
  mMapToolGroup->addAction( mActionDeleteRing );
  mMapToolGroup->addAction( mActionDeletePart );
  mMapToolGroup->addAction( mActionMergeFeatures );
  mMapToolGroup->addAction( mActionMergeFeatureAttributes );
  mMapToolGroup->addAction( mActionNodeTool );
  mMapToolGroup->addAction( mActionRotatePointSymbols );
  mMapToolGroup->addAction( mActionOffsetPointSymbol );
  mMapToolGroup->addAction( mActionPinLabels );
  mMapToolGroup->addAction( mActionShowHideLabels );
  mMapToolGroup->addAction( mActionMoveLabel );
  mMapToolGroup->addAction( mActionRotateLabel );
  mMapToolGroup->addAction( mActionChangeLabelProperties );

  //
  // Preview Modes Group
  QActionGroup *mPreviewGroup = new QActionGroup( this );
  mPreviewGroup->setExclusive( true );
  mActionPreviewModeOff->setActionGroup( mPreviewGroup );
  mActionPreviewModeGrayscale->setActionGroup( mPreviewGroup );
  mActionPreviewModeMono->setActionGroup( mPreviewGroup );
  mActionPreviewProtanope->setActionGroup( mPreviewGroup );
  mActionPreviewDeuteranope->setActionGroup( mPreviewGroup );
}

void QgisApp::setAppStyleSheet( const QString &stylesheet )
{
  setStyleSheet( stylesheet );

  // cascade styles to any current layout designers
  Q_FOREACH ( QgsLayoutDesignerDialog *d, mLayoutDesignerDialogs )
  {
    d->setStyleSheet( stylesheet );
  }
}

int QgisApp::messageTimeout()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}

void QgisApp::createMenus()
{
  /*
   * The User Interface Guidelines for each platform specify different locations
   * for the following items.
   *
   * Custom CRS, Options:
   * Gnome - bottom of Edit menu
   * Mac - Application menu (moved automatically by Qt)
   * Kde, Win - Settings menu (Win should use Tools menu but we don't have one)
   *
   * Panel and Toolbar submenus, Toggle Full Screen:
   * Gnome, Mac, Win - View menu
   * Kde - Settings menu
   *
   * For Mac, About and Exit are also automatically moved by Qt to the Application menu.
   */

  // Layer menu

  // Panel and Toolbar Submenus
  mPanelMenu = new QMenu( tr( "Panels" ), this );
  mPanelMenu->setObjectName( QStringLiteral( "mPanelMenu" ) );
  mToolbarMenu = new QMenu( tr( "Toolbars" ), this );
  mToolbarMenu->setObjectName( QStringLiteral( "mToolbarMenu" ) );

  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, nullptr, this ) );

  // Connect once for the entire submenu.
  connect( mRecentProjectsMenu, &QMenu::triggered, this, static_cast < void ( QgisApp::* )( QAction *action ) >( &QgisApp::openProject ) );
  connect( mProjectFromTemplateMenu, &QMenu::triggered,
           this, &QgisApp::fileNewFromTemplateAction );


  // View Menu

  if ( layout != QDialogButtonBox::KdeLayout )
  {
    mViewMenu->addSeparator();
    mViewMenu->addMenu( mPanelMenu );
    mViewMenu->addMenu( mToolbarMenu );
    mViewMenu->addAction( mActionToggleFullScreen );
    mViewMenu->addAction( mActionTogglePanelsVisibility );
  }
  else
  {
    // on the top of the settings menu
    QAction *before = mSettingsMenu->actions().first();
    mSettingsMenu->insertMenu( before, mPanelMenu );
    mSettingsMenu->insertMenu( before, mToolbarMenu );
    mSettingsMenu->insertAction( before, mActionToggleFullScreen );
    mSettingsMenu->insertAction( before, mActionTogglePanelsVisibility );
    mSettingsMenu->insertSeparator( before );
  }

#ifdef Q_OS_MAC

  // keep plugins from hijacking About and Preferences application menus
  // these duplicate actions will be moved to application menus by Qt
  mProjectMenu->addAction( mActionAbout );
  QAction *actionPrefs = new QAction( tr( "Preferences..." ), this );
  actionPrefs->setMenuRole( QAction::PreferencesRole );
  actionPrefs->setIcon( mActionOptions->icon() );
  connect( actionPrefs, &QAction::triggered, this, &QgisApp::options );
  mProjectMenu->addAction( actionPrefs );

  // Window Menu

  mWindowMenu = new QMenu( tr( "Window" ), this );

  mWindowMenu->addAction( mActionWindowMinimize );
  mWindowMenu->addAction( mActionWindowZoom );
  mWindowMenu->addSeparator();

  mWindowMenu->addAction( mActionWindowAllToFront );
  mWindowMenu->addSeparator();

  // insert before Help menu, as per Mac OS convention
  menuBar()->insertMenu( mHelpMenu->menuAction(), mWindowMenu );
#endif

  // Database Menu
  // don't add it yet, wait for a plugin
  mDatabaseMenu = new QMenu( tr( "&Database" ), menuBar() );
  mDatabaseMenu->setObjectName( QStringLiteral( "mDatabaseMenu" ) );
  // Web Menu
  // don't add it yet, wait for a plugin
  mWebMenu = new QMenu( tr( "&Web" ), menuBar() );
  mWebMenu->setObjectName( QStringLiteral( "mWebMenu" ) );

  createProfileMenu();
}

void QgisApp::refreshProfileMenu()
{
  mConfigMenu->clear();
  QgsUserProfile *profile = userProfileManager()->userProfile();
  QString activeName = profile->name();
  mConfigMenu->setTitle( tr( "&User Profiles" ) );

  QActionGroup *profileGroup = new QActionGroup( mConfigMenu );
  profileGroup->setExclusive( true );

  Q_FOREACH ( const QString &name, userProfileManager()->allProfiles() )
  {
    std::unique_ptr< QgsUserProfile > namedProfile( userProfileManager()->profileForName( name ) );
    QAction *action = new QAction( namedProfile->icon(), namedProfile->alias(), mConfigMenu );
    action->setToolTip( namedProfile->folder() );
    action->setCheckable( true );
    profileGroup->addAction( action );
    mConfigMenu->addAction( action );

    if ( name == activeName )
    {
      action->setChecked( true );
    }
    else
    {
      connect( action, &QAction::triggered, this, [this, name]()
      {
        userProfileManager()->loadUserProfile( name );
      } );
    }
  }

  mConfigMenu->addSeparator( );

  QAction *openProfileFolderAction = mConfigMenu->addAction( tr( "Open active profile folder" ) );
  connect( openProfileFolderAction, &QAction::triggered, this, [this]()
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( userProfileManager()->userProfile()->folder() ) );
  } );

  QAction *newProfileAction = mConfigMenu->addAction( tr( "New profile" ) );
  connect( newProfileAction, &QAction::triggered, this, &QgisApp::newProfile );
}

void QgisApp::createProfileMenu()
{
  mConfigMenu = new QMenu();

  settingsMenu()->insertMenu( settingsMenu()->actions().first(), mConfigMenu );

  refreshProfileMenu();
}

void QgisApp::createToolBars()
{
  QgsSettings settings;
  // QSize myIconSize ( 32,32 ); //large icons
  // Note: we need to set each object name to ensure that
  // qmainwindow::saveState and qmainwindow::restoreState
  // work properly

  QList<QToolBar *> toolbarMenuToolBars;
  toolbarMenuToolBars << mFileToolBar
                      << mDataSourceManagerToolBar
                      << mLayerToolBar
                      << mDigitizeToolBar
                      << mAdvancedDigitizeToolBar
                      << mMapNavToolBar
                      << mAttributesToolBar
                      << mPluginToolBar
                      << mHelpToolBar
                      << mRasterToolBar
                      << mVectorToolBar
                      << mDatabaseToolBar
                      << mWebToolBar
                      << mLabelToolBar;


  mSnappingWidget = new QgsSnappingWidget( QgsProject::instance(), mMapCanvas, mSnappingToolBar );
  mSnappingWidget->setObjectName( QStringLiteral( "mSnappingWidget" ) );
  connect( mSnappingWidget, &QgsSnappingWidget::snappingConfigChanged, QgsProject::instance(), [ = ] { QgsProject::instance()->setSnappingConfig( mSnappingWidget->config() ); } );
  mSnappingToolBar->addWidget( mSnappingWidget );

  mTracer = new QgsMapCanvasTracer( mMapCanvas, messageBar() );
  mTracer->setActionEnableTracing( mSnappingWidget->enableTracingAction() );
  mTracer->setActionEnableSnapping( mSnappingWidget->enableSnappingAction() );
  connect( mSnappingWidget->tracingOffsetSpinBox(), static_cast< void ( QgsDoubleSpinBox::* )( double ) >( &QgsDoubleSpinBox::valueChanged ),
  this, [ = ]( double v ) { mTracer->setOffset( v ); } );

  QList<QAction *> toolbarMenuActions;
  // Set action names so that they can be used in customization
  Q_FOREACH ( QToolBar *toolBar, toolbarMenuToolBars )
  {
    toolBar->toggleViewAction()->setObjectName( "mActionToggle" + toolBar->objectName().mid( 1 ) );
    toolbarMenuActions << toolBar->toggleViewAction();
  }

  // sort actions in toolbar menu
  std::sort( toolbarMenuActions.begin(), toolbarMenuActions.end(), cmpByText_ );

  mToolbarMenu->addActions( toolbarMenuActions );

  // selection tool button

  QToolButton *bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  QList<QAction *> selectionActions;
  selectionActions << mActionSelectByForm << mActionSelectByExpression << mActionSelectAll
                   << mActionInvertSelection;
  bt->addActions( selectionActions );

  QAction *defSelectionAction = mActionSelectByForm;
  switch ( settings.value( QStringLiteral( "UI/selectionTool" ), 0 ).toInt() )
  {
    case 0:
      defSelectionAction = mActionSelectByForm;
      break;
    case 1:
      defSelectionAction = mActionSelectByExpression;
      break;
    case 2:
      defSelectionAction = mActionSelectAll;
      break;
    case 3:
      defSelectionAction = mActionInvertSelection;
      break;
  }
  bt->setDefaultAction( defSelectionAction );
  QAction *selectionAction = mAttributesToolBar->insertWidget( mActionDeselectAll, bt );
  selectionAction->setObjectName( QStringLiteral( "ActionSelection" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // select tool button

  bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  QList<QAction *> selectActions;
  selectActions << mActionSelectFeatures << mActionSelectPolygon
                << mActionSelectFreehand << mActionSelectRadius;
  bt->addActions( selectActions );

  QAction *defSelectAction = mActionSelectFeatures;
  switch ( settings.value( QStringLiteral( "UI/selectTool" ), 1 ).toInt() )
  {
    case 1:
      defSelectAction = mActionSelectFeatures;
      break;
    case 2:
      defSelectAction = mActionSelectRadius;
      break;
    case 3:
      defSelectAction = mActionSelectPolygon;
      break;
    case 4:
      defSelectAction = mActionSelectFreehand;
      break;
  }
  bt->setDefaultAction( defSelectAction );
  QAction *selectAction = mAttributesToolBar->insertWidget( selectionAction, bt );
  selectAction->setObjectName( QStringLiteral( "ActionSelect" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // feature action tool button

  bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->setDefaultAction( mActionFeatureAction );
  mFeatureActionMenu = new QMenu( bt );
  connect( mFeatureActionMenu, &QMenu::triggered, this, &QgisApp::updateDefaultFeatureAction );
  connect( mFeatureActionMenu, &QMenu::triggered, this, &QgisApp::doFeatureAction );
  connect( mFeatureActionMenu, &QMenu::aboutToShow, this, &QgisApp::refreshFeatureActions );
  bt->setMenu( mFeatureActionMenu );
  QAction *featureActionAction = mAttributesToolBar->insertWidget( selectAction, bt );
  featureActionAction->setObjectName( QStringLiteral( "ActionFeatureAction" ) );

  // measure tool button

  bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionMeasure );
  bt->addAction( mActionMeasureArea );
  bt->addAction( mActionMeasureAngle );

  QAction *defMeasureAction = mActionMeasure;
  switch ( settings.value( QStringLiteral( "UI/measureTool" ), 0 ).toInt() )
  {
    case 0:
      defMeasureAction = mActionMeasure;
      break;
    case 1:
      defMeasureAction = mActionMeasureArea;
      break;
    case 2:
      defMeasureAction = mActionMeasureAngle;
      break;
  }
  bt->setDefaultAction( defMeasureAction );
  QAction *measureAction = mAttributesToolBar->insertWidget( mActionMapTips, bt );
  measureAction->setObjectName( QStringLiteral( "ActionMeasure" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // annotation tool button

  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionTextAnnotation );
  bt->addAction( mActionFormAnnotation );
  bt->addAction( mActionHtmlAnnotation );
  bt->addAction( mActionSvgAnnotation );
  bt->addAction( mActionAnnotation );

  QAction *defAnnotationAction = mActionTextAnnotation;
  switch ( settings.value( QStringLiteral( "UI/annotationTool" ), 0 ).toInt() )
  {
    case 0:
      defAnnotationAction = mActionTextAnnotation;
      break;
    case 1:
      defAnnotationAction = mActionFormAnnotation;
      break;
    case 2:
      defAnnotationAction = mActionHtmlAnnotation;
      break;
    case 3:
      defAnnotationAction = mActionSvgAnnotation;
      break;
    case 4:
      defAnnotationAction = mActionAnnotation;
      break;
  }
  bt->setDefaultAction( defAnnotationAction );
  QAction *annotationAction = mAttributesToolBar->addWidget( bt );
  annotationAction->setObjectName( QStringLiteral( "ActionAnnotation" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // vector layer edits tool buttons
  QToolButton *tbAllEdits = qobject_cast<QToolButton *>( mDigitizeToolBar->widgetForAction( mActionAllEdits ) );
  tbAllEdits->setPopupMode( QToolButton::InstantPopup );

  // new layer tool button

  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionNewVectorLayer );
  bt->addAction( mActionNewSpatiaLiteLayer );
  bt->addAction( mActionNewGeoPackageLayer );
  bt->addAction( mActionNewMemoryLayer );

  QAction *defNewLayerAction = mActionNewVectorLayer;
  switch ( settings.value( QStringLiteral( "UI/defaultNewLayer" ), 1 ).toInt() )
  {
    case 0:
      defNewLayerAction = mActionNewSpatiaLiteLayer;
      break;
    case 1:
      defNewLayerAction = mActionNewVectorLayer;
      break;
    case 2:
      defNewLayerAction = mActionNewMemoryLayer;
      break;
    case 3:
      defNewLayerAction = mActionNewGeoPackageLayer;
      break;
  }
  bt->setDefaultAction( defNewLayerAction );
  QAction *newLayerAction = mLayerToolBar->addWidget( bt );

  newLayerAction->setObjectName( QStringLiteral( "ActionNewLayer" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // map service tool button
  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionAddWmsLayer );
  bt->addAction( mActionAddAmsLayer );
  QAction *defMapServiceAction = mActionAddWmsLayer;
  switch ( settings.value( QStringLiteral( "UI/defaultMapService" ), 0 ).toInt() )
  {
    case 0:
      defMapServiceAction = mActionAddWmsLayer;
      break;
    case 1:
      defMapServiceAction = mActionAddAmsLayer;
      break;
  };
  bt->setDefaultAction( defMapServiceAction );
  QAction *mapServiceAction = mLayerToolBar->insertWidget( mActionAddWmsLayer, bt );
  mLayerToolBar->removeAction( mActionAddWmsLayer );
  mapServiceAction->setObjectName( QStringLiteral( "ActionMapService" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // feature service tool button
  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionAddWfsLayer );
  bt->addAction( mActionAddAfsLayer );
  QAction *defFeatureServiceAction = mActionAddWfsLayer;
  switch ( settings.value( QStringLiteral( "UI/defaultFeatureService" ), 0 ).toInt() )
  {
    case 0:
      defFeatureServiceAction = mActionAddWfsLayer;
      break;
    case 1:
      defFeatureServiceAction = mActionAddAfsLayer;
      break;
  };
  bt->setDefaultAction( defFeatureServiceAction );
  QAction *featureServiceAction = mLayerToolBar->insertWidget( mActionAddWfsLayer, bt );
  mLayerToolBar->removeAction( mActionAddWfsLayer );
  featureServiceAction->setObjectName( QStringLiteral( "ActionFeatureService" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // add db layer button
  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  if ( mActionAddPgLayer )
    bt->addAction( mActionAddPgLayer );
  if ( mActionAddMssqlLayer )
    bt->addAction( mActionAddMssqlLayer );
  if ( mActionAddDb2Layer )
    bt->addAction( mActionAddDb2Layer );
  if ( mActionAddOracleLayer )
    bt->addAction( mActionAddOracleLayer );
  QAction *defAddDbLayerAction = mActionAddPgLayer;
  switch ( settings.value( QStringLiteral( "UI/defaultAddDbLayerAction" ), 0 ).toInt() )
  {
    case 0:
      defAddDbLayerAction = mActionAddPgLayer;
      break;
    case 1:
      defAddDbLayerAction = mActionAddMssqlLayer;
      break;
    case 2:
      defAddDbLayerAction = mActionAddDb2Layer;
      break;
    case 3:
      defAddDbLayerAction = mActionAddOracleLayer;
      break;
  }
  if ( defAddDbLayerAction )
    bt->setDefaultAction( defAddDbLayerAction );
  QAction *addDbLayerAction = mLayerToolBar->insertWidget( mapServiceAction, bt );
  addDbLayerAction->setObjectName( QStringLiteral( "ActionAddDbLayer" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  QLayout *layout = mLayerToolBar->layout();
  for ( int i = 0; i < layout->count(); ++i )
  {
    layout->itemAt( i )->setAlignment( Qt::AlignLeft );
  }

  //circular string digitize tool button
  QToolButton *tbAddCircularString = new QToolButton( mRegularShapeDigitizeToolBar );
  tbAddCircularString->setPopupMode( QToolButton::MenuButtonPopup );
  tbAddCircularString->addAction( mActionCircularStringCurvePoint );
  tbAddCircularString->addAction( mActionCircularStringRadius );
  tbAddCircularString->setDefaultAction( mActionCircularStringCurvePoint );
  connect( tbAddCircularString, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mRegularShapeDigitizeToolBar->insertWidget( mActionNodeTool, tbAddCircularString );

  //circle digitize tool button
  QToolButton *tbAddCircle = new QToolButton( mRegularShapeDigitizeToolBar );
  tbAddCircle->setPopupMode( QToolButton::MenuButtonPopup );
  tbAddCircle->addAction( mActionCircle2Points );
  tbAddCircle->addAction( mActionCircle3Points );
  tbAddCircle->addAction( mActionCircle3Tangents );
  tbAddCircle->addAction( mActionCircle2TangentsPoint );
  tbAddCircle->addAction( mActionCircleCenterPoint );
  tbAddCircle->setDefaultAction( mActionCircle2Points );
  connect( tbAddCircle, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mRegularShapeDigitizeToolBar->insertWidget( mActionNodeTool, tbAddCircle );

  //ellipse digitize tool button
  QToolButton *tbAddEllipse = new QToolButton( mRegularShapeDigitizeToolBar );
  tbAddEllipse->setPopupMode( QToolButton::MenuButtonPopup );
  tbAddEllipse->addAction( mActionEllipseCenter2Points );
  tbAddEllipse->addAction( mActionEllipseCenterPoint );
  tbAddEllipse->addAction( mActionEllipseExtent );
  tbAddEllipse->addAction( mActionEllipseFoci );
  tbAddEllipse->setDefaultAction( mActionEllipseCenter2Points );
  connect( tbAddEllipse, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mRegularShapeDigitizeToolBar->insertWidget( mActionNodeTool, tbAddEllipse );

  //Rectangle digitize tool button
  QToolButton *tbAddRectangle = new QToolButton( mRegularShapeDigitizeToolBar );
  tbAddRectangle->setPopupMode( QToolButton::MenuButtonPopup );
  tbAddRectangle->addAction( mActionRectangleCenterPoint );
  tbAddRectangle->addAction( mActionRectangleExtent );
  tbAddRectangle->addAction( mActionRectangle3Points );
  tbAddRectangle->setDefaultAction( mActionRectangleCenterPoint );
  connect( tbAddRectangle, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mRegularShapeDigitizeToolBar->insertWidget( mActionNodeTool, tbAddRectangle );

  //Regular polygon digitize tool button
  QToolButton *tbAddRegularPolygon = new QToolButton( mRegularShapeDigitizeToolBar );
  tbAddRegularPolygon->setPopupMode( QToolButton::MenuButtonPopup );
  tbAddRegularPolygon->addAction( mActionRegularPolygon2Points );
  tbAddRegularPolygon->addAction( mActionRegularPolygonCenterPoint );
  tbAddRegularPolygon->addAction( mActionRegularPolygonCenterCorner );
  tbAddRegularPolygon->setDefaultAction( mActionRegularPolygon2Points );
  connect( tbAddRegularPolygon, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mRegularShapeDigitizeToolBar->insertWidget( mActionNodeTool, tbAddRegularPolygon );

  // move feature tool button
  QToolButton *moveFeatureButton = new QToolButton( mDigitizeToolBar );
  moveFeatureButton->setPopupMode( QToolButton::MenuButtonPopup );
  moveFeatureButton->addAction( mActionMoveFeature );
  moveFeatureButton->addAction( mActionMoveFeatureCopy );
  QAction *defAction = mActionMoveFeature;
  switch ( settings.value( QStringLiteral( "UI/defaultMoveTool" ), 0 ).toInt() )
  {
    case 0:
      defAction = mActionMoveFeature;
      break;
    case 1:
      defAction = mActionMoveFeatureCopy;
      break;
  };
  moveFeatureButton->setDefaultAction( defAction );
  connect( moveFeatureButton, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );
  mDigitizeToolBar->insertWidget( mActionNodeTool, moveFeatureButton );

  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionRotatePointSymbols );
  bt->addAction( mActionOffsetPointSymbol );

  QAction *defPointSymbolAction = mActionRotatePointSymbols;
  switch ( settings.value( QStringLiteral( "UI/defaultPointSymbolAction" ), 0 ).toInt() )
  {
    case 0:
      defPointSymbolAction = mActionRotatePointSymbols;
      break;
    case 1:
      defPointSymbolAction = mActionOffsetPointSymbol;
      break;
  }
  bt->setDefaultAction( defPointSymbolAction );
  QAction *pointSymbolAction = mAdvancedDigitizeToolBar->addWidget( bt );
  pointSymbolAction->setObjectName( QStringLiteral( "ActionPointSymbolTools" ) );
  connect( bt, &QToolButton::triggered, this, &QgisApp::toolButtonActionTriggered );

  // Cad toolbar
  mAdvancedDigitizeToolBar->insertAction( mActionRotateFeature, mAdvancedDigitizingDockWidget->enableAction() );
}

void QgisApp::createStatusBar()
{
  //remove borders from children under Windows
  statusBar()->setStyleSheet( QStringLiteral( "QStatusBar::item {border: none;}" ) );

  mStatusBar = new QgsStatusBar();

  statusBar()->addPermanentWidget( mStatusBar, 10 );

  // Add a panel to the status bar for the scale, coords and progress
  // And also rendering suppression checkbox
  mProgressBar = new QProgressBar( mStatusBar );
  mProgressBar->setObjectName( QStringLiteral( "mProgressBar" ) );
  mProgressBar->setMaximumWidth( 100 );
  mProgressBar->hide();
  mProgressBar->setWhatsThis( tr( "Progress bar that displays the status "
                                  "of rendering layers and other time-intensive operations" ) );
  mStatusBar->addPermanentWidget( mProgressBar, 1 );

  connect( mMapCanvas, &QgsMapCanvas::renderStarting, this, &QgisApp::canvasRefreshStarted );
  connect( mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &QgisApp::canvasRefreshFinished );

  mTaskManagerWidget = new QgsTaskManagerStatusBarWidget( QgsApplication::taskManager(), mStatusBar );
  mStatusBar->addPermanentWidget( mTaskManagerWidget, 0 );

  // Bumped the font up one point size since 8 was too
  // small on some platforms. A point size of 9 still provides
  // plenty of display space on 1024x768 resolutions
  QFont myFont( QStringLiteral( "Arial" ), 9 );
  statusBar()->setFont( myFont );

  //coords status bar widget
  mCoordsEdit = new QgsStatusBarCoordinatesWidget( mStatusBar );
  mCoordsEdit->setObjectName( QStringLiteral( "mCoordsEdit" ) );
  mCoordsEdit->setMapCanvas( mMapCanvas );
  mCoordsEdit->setFont( myFont );
  mStatusBar->addPermanentWidget( mCoordsEdit, 0 );

  mScaleWidget = new QgsStatusBarScaleWidget( mMapCanvas, mStatusBar );
  mScaleWidget->setObjectName( QStringLiteral( "mScaleWidget" ) );
  mScaleWidget->setFont( myFont );
  connect( mScaleWidget, &QgsStatusBarScaleWidget::scaleLockChanged, mMapCanvas, &QgsMapCanvas::setScaleLocked );
  mStatusBar->addPermanentWidget( mScaleWidget, 0 );

  // zoom widget
  mMagnifierWidget = new QgsStatusBarMagnifierWidget( mStatusBar );
  mMagnifierWidget->setObjectName( QStringLiteral( "mMagnifierWidget" ) );
  mMagnifierWidget->setFont( myFont );
  connect( mMapCanvas, &QgsMapCanvas::magnificationChanged, mMagnifierWidget, &QgsStatusBarMagnifierWidget::updateMagnification );
  connect( mMagnifierWidget, &QgsStatusBarMagnifierWidget::magnificationChanged, mMapCanvas, &QgsMapCanvas::setMagnificationFactor );
  mMagnifierWidget->updateMagnification( QSettings().value( QStringLiteral( "/qgis/magnifier_factor_default" ), 1.0 ).toDouble() );
  mStatusBar->addPermanentWidget( mMagnifierWidget, 0 );

  // add a widget to show/set current rotation
  mRotationLabel = new QLabel( QString(), mStatusBar );
  mRotationLabel->setObjectName( QStringLiteral( "mRotationLabel" ) );
  mRotationLabel->setFont( myFont );
  mRotationLabel->setMinimumWidth( 10 );
  //mRotationLabel->setMaximumHeight( 20 );
  mRotationLabel->setMargin( 3 );
  mRotationLabel->setAlignment( Qt::AlignCenter );
  mRotationLabel->setFrameStyle( QFrame::NoFrame );
  mRotationLabel->setText( tr( "Rotation" ) );
  mRotationLabel->setToolTip( tr( "Current clockwise map rotation in degrees" ) );
  mStatusBar->addPermanentWidget( mRotationLabel, 0 );

  mRotationEdit = new QgsDoubleSpinBox( mStatusBar );
  mRotationEdit->setObjectName( QStringLiteral( "mRotationEdit" ) );
  mRotationEdit->setClearValue( 0.0 );
  mRotationEdit->setKeyboardTracking( false );
  mRotationEdit->setMaximumWidth( 120 );
  mRotationEdit->setDecimals( 1 );
  mRotationEdit->setRange( -360.0, 360.0 );
  mRotationEdit->setWrapping( true );
  mRotationEdit->setSingleStep( 5.0 );
  mRotationEdit->setFont( myFont );
  mRotationEdit->setSuffix( trUtf8( " °" ) );
  mRotationEdit->setWhatsThis( tr( "Shows the current map clockwise rotation "
                                   "in degrees. It also allows editing to set "
                                   "the rotation" ) );
  mRotationEdit->setToolTip( tr( "Current clockwise map rotation in degrees" ) );
  mStatusBar->addPermanentWidget( mRotationEdit, 0 );
  connect( mRotationEdit, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgisApp::userRotation );

  showRotation();

  // render suppression status bar widget
  mRenderSuppressionCBox = new QCheckBox( tr( "Render" ), mStatusBar );
  mRenderSuppressionCBox->setObjectName( QStringLiteral( "mRenderSuppressionCBox" ) );
  mRenderSuppressionCBox->setChecked( true );
  mRenderSuppressionCBox->setFont( myFont );
  mRenderSuppressionCBox->setWhatsThis( tr( "When checked, the map layers "
                                        "are rendered in response to map navigation commands and other "
                                        "events. When not checked, no rendering is done. This allows you "
                                        "to add a large number of layers and symbolize them before rendering." ) );
  mRenderSuppressionCBox->setToolTip( tr( "Toggle map rendering" ) );
  mStatusBar->addPermanentWidget( mRenderSuppressionCBox, 0 );
  // On the fly projection status bar icon
  // Changed this to a tool button since a QPushButton is
  // sculpted on OS X and the icon is never displayed [gsherman]
  mOnTheFlyProjectionStatusButton = new QToolButton( mStatusBar );
  mOnTheFlyProjectionStatusButton->setAutoRaise( true );
  mOnTheFlyProjectionStatusButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  mOnTheFlyProjectionStatusButton->setObjectName( QStringLiteral( "mOntheFlyProjectionStatusButton" ) );
  // Maintain uniform widget height in status bar by setting button height same as labels
  // For Qt/Mac 3.3, the default toolbutton height is 30 and labels were expanding to match
  mOnTheFlyProjectionStatusButton->setMaximumHeight( mScaleWidget->height() );
  mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconProjectionEnabled.svg" ) ) );
  mOnTheFlyProjectionStatusButton->setWhatsThis( tr( "This icon shows whether "
      "on the fly coordinate reference system transformation is enabled or not. "
      "Click the icon to bring up "
      "the project properties dialog to alter this behavior." ) );
  mOnTheFlyProjectionStatusButton->setToolTip( tr( "CRS status - Click "
      "to open coordinate reference system dialog" ) );
  connect( mOnTheFlyProjectionStatusButton, &QAbstractButton::clicked,
           this, &QgisApp::projectPropertiesProjections );//bring up the project props dialog when clicked
  mStatusBar->addPermanentWidget( mOnTheFlyProjectionStatusButton, 0 );
  mStatusBar->showMessage( tr( "Ready" ) );

  mMessageButton = new QToolButton( mStatusBar );
  mMessageButton->setAutoRaise( true );
  mMessageButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mMessageLogRead.svg" ) ) );
  mMessageButton->setToolTip( tr( "Messages" ) );
  mMessageButton->setWhatsThis( tr( "Messages" ) );
  mMessageButton->setObjectName( QStringLiteral( "mMessageLogViewerButton" ) );
  mMessageButton->setMaximumHeight( mScaleWidget->height() );
  mMessageButton->setCheckable( true );
  mStatusBar->addPermanentWidget( mMessageButton, 0 );

  mLocatorWidget = new QgsLocatorWidget( mStatusBar );
  mStatusBar->addPermanentWidget( mLocatorWidget, 0, QgsStatusBar::AnchorLeft );
  QShortcut *locatorShortCut = new QShortcut( QKeySequence( tr( "Ctrl+K" ) ), this );
  connect( locatorShortCut, &QShortcut::activated, mLocatorWidget, [ = ] { mLocatorWidget->search( QString() ); } );
  locatorShortCut->setObjectName( QStringLiteral( "Locator" ) );
  locatorShortCut->setWhatsThis( tr( "Trigger Locator" ) );

  mLocatorWidget->locator()->registerFilter( new QgsLayerTreeLocatorFilter() );
  mLocatorWidget->locator()->registerFilter( new QgsLayoutLocatorFilter() );
  QList< QWidget *> actionObjects;
  actionObjects << menuBar()
                << mAdvancedDigitizeToolBar
                << mFileToolBar
                << mDataSourceManagerToolBar
                << mLayerToolBar
                << mDigitizeToolBar
                << mMapNavToolBar
                << mAttributesToolBar
                << mPluginToolBar
                << mRasterToolBar
                << mLabelToolBar
                << mVectorToolBar
                << mDatabaseToolBar
                << mWebToolBar
                << mSnappingToolBar;

  mLocatorWidget->locator()->registerFilter( new QgsActionLocatorFilter( actionObjects ) );
  mLocatorWidget->locator()->registerFilter( new QgsActiveLayerFeaturesLocatorFilter() );
}

void QgisApp::setIconSizes( int size )
{
  int dockSize = dockedToolbarIconSize( size );

  //Set the icon size of for all the toolbars created in the future.
  setIconSize( QSize( size, size ) );

  //Change all current icon sizes.
  QList<QToolBar *> toolbars = findChildren<QToolBar *>();
  Q_FOREACH ( QToolBar *toolbar, toolbars )
  {
    QString className = toolbar->parent()->metaObject()->className();
    if ( className == QLatin1String( "QgisApp" ) )
    {
      toolbar->setIconSize( QSize( size, size ) );
    }
    else
    {
      toolbar->setIconSize( QSize( dockSize, dockSize ) );
    }
  }

  Q_FOREACH ( QgsLayoutDesignerDialog *d, mLayoutDesignerDialogs )
  {
    d->setIconSizes( size );
  }
}

void QgisApp::setTheme( const QString &themeName )
{
  /*****************************************************************
  // Init the toolbar icons by setting the icon for each action.
  // All toolbar/menu items must be a QAction in order for this
  // to work.
  //
  // When new toolbar/menu QAction objects are added to the interface,
  // add an entry below to set the icon
  //
  // PNG names must match those defined for the default theme. The
  // default theme is installed in <prefix>/share/qgis/themes/default.
  //
  // New core themes can be added by creating a subdirectory under src/themes
  // and modifying the appropriate CMakeLists.txt files. User contributed themes
  // will be installed directly into <prefix>/share/qgis/themes/<themedir>.
  //
  // Themes can be selected from the preferences dialog. The dialog parses
  // the themes directory and builds a list of themes (ie subdirectories)
  // for the user to choose from.
  //
  */
  QgsApplication::setUITheme( themeName );
  //QgsDebugMsg("Setting theme to \n" + themeName);
  mActionNewProject->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileNew.svg" ) ) );
  mActionOpenProject->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  mActionSaveProject->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSave.svg" ) ) );
  mActionSaveProjectAs->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSaveAs.svg" ) ) );
  mActionSaveMapAsImage->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveMapAsImage.svg" ) ) );
  mActionSaveMapAsPdf->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsPDF.svg" ) ) );
  mActionExit->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileExit.png" ) ) );
  mActionAddOgrLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddOgrLayer.svg" ) ) );
  mActionAddRasterLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ) );
#ifdef HAVE_POSTGRESQL
  mActionAddPgLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPostgisLayer.svg" ) ) );
#endif
  mActionNewSpatiaLiteLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewSpatiaLiteLayer.svg" ) ) );
  mActionAddSpatiaLiteLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) ) );
  mActionAddMssqlLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMssqlLayer.svg" ) ) );
  mActionAddDb2Layer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddDb2Layer.svg" ) ) );
#ifdef HAVE_ORACLE
  mActionAddOracleLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddOracleLayer.svg" ) );
#endif
  mActionRemoveLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ) );
  mActionDuplicateLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateLayer.svg" ) ) );
  mActionSetLayerCRS->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSetLayerCRS.png" ) ) );
  mActionSetProjectCRSFromLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSetProjectCRSFromLayer.png" ) ) );
  mActionNewVectorLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) ) );
  mActionDataSourceManager->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDataSourceManager.svg" ) ) );
  mActionNewMemoryLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCreateMemory.svg" ) ) );
  mActionAddAllToOverview->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAllToOverview.svg" ) ) );
  mActionHideAllLayers->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideAllLayers.svg" ) ) );
  mActionShowAllLayers->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  mActionHideSelectedLayers->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideSelectedLayers.svg" ) ) );
  mActionHideDeselectedLayers->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideDeselectedLayers.svg" ) ) );
  mActionShowSelectedLayers->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowSelectedLayers.svg" ) ) );
  mActionRemoveAllFromOverview->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveAllFromOverview.svg" ) ) );
  mActionToggleFullScreen->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleFullScreen.png" ) ) );
  mActionProjectProperties->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionProjectProperties.png" ) ) );
  mActionManagePlugins->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowPluginManager.svg" ) ) );
  mActionShowPythonDialog->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/mIconRunConsole.svg" ) ) );
  mActionCheckQgisVersion->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSuccess.svg" ) ) );
  mActionOptions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) ) );
  mActionConfigureShortcuts->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionKeyboardShortcuts.svg" ) ) );
  mActionCustomization->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInterfaceCustomization.svg" ) ) );
  mActionHelpContents->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHelpContents.svg" ) ) );
  mActionLocalHistogramStretch->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLocalHistogramStretch.svg" ) ) );
  mActionFullHistogramStretch->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFullHistogramStretch.svg" ) ) );
  mActionIncreaseBrightness->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIncreaseBrightness.svg" ) ) );
  mActionDecreaseBrightness->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDecreaseBrightness.svg" ) ) );
  mActionIncreaseContrast->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIncreaseContrast.svg" ) ) );
  mActionDecreaseContrast->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDecreaseContrast.svg" ) ) );
  mActionZoomActualSize->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomNative.png" ) ) );
  mActionQgisHomePage->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionQgisHomePage.png" ) ) );
  mActionAbout->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHelpAbout.svg" ) ) );
  mActionSponsors->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHelpSponsors.png" ) ) );
  mActionDraw->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDraw.svg" ) ) );
  mActionToggleEditing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mActionSaveLayerEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAllEdits.svg" ) ) );
  mActionAllEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAllEdits.svg" ) ) );
  mActionSaveEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
  mActionSaveAllEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAllEdits.svg" ) ) );
  mActionRollbackEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRollbackEdits.svg" ) ) );
  mActionRollbackAllEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRollbackAllEdits.svg" ) ) );
  mActionCancelEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCancelEdits.svg" ) ) );
  mActionCancelAllEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCancelAllEdits.svg" ) ) );
  mActionCutFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCut.svg" ) ) );
  mActionCopyFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  mActionPasteFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePoint.svg" ) ) );
  mActionMoveFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeaturePoint.svg" ) ) );
  mActionMoveFeatureCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeatureCopyPoint.svg" ) ) );
  mActionRotateFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRotateFeature.svg" ) ) );
  mActionReshapeFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionReshape.svg" ) ) );
  mActionSplitFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSplitFeatures.svg" ) ) );
  mActionSplitParts->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSplitParts.svg" ) ) );
  mActionDeleteSelected->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mActionNodeTool->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNodeTool.svg" ) ) );
  mActionSimplifyFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSimplify.svg" ) ) );
  mActionUndo->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );
  mActionRedo->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ) );
  mActionAddRing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRing.svg" ) ) );
  mActionFillRing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFillRing.svg" ) ) );
  mActionAddPart->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPart.svg" ) ) );
  mActionDeleteRing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteRing.svg" ) ) );
  mActionDeletePart->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeletePart.svg" ) ) );
  mActionMergeFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatures.svg" ) ) );
  mActionOffsetCurve->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOffsetCurve.svg" ) ) );
  mActionMergeFeatureAttributes->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatureAttributes.svg" ) ) );
  mActionRotatePointSymbols->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRotatePointSymbols.svg" ) ) );
  mActionOffsetPointSymbol->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOffsetPointSymbols.svg" ) ) );
  mActionZoomIn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mActionZoomOut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  mActionZoomFullExtent->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomFullExtent.svg" ) ) );
  mActionZoomToSelected->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mActionShowRasterCalculator->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowRasterCalculator.png" ) ) );
  mActionPan->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  mActionPanToSelected->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPanToSelected.svg" ) ) );
  mActionZoomLast->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomLast.svg" ) ) );
  mActionZoomNext->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomNext.svg" ) ) );
  mActionZoomToLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ) );
  mActionZoomActualSize->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomActual.svg" ) ) );
  mActionIdentify->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIdentify.svg" ) ) );
  mActionFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
  mActionSelectFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectRectangle.svg" ) ) );
  mActionSelectPolygon->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectPolygon.svg" ) ) );
  mActionSelectFreehand->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectFreehand.svg" ) ) );
  mActionSelectRadius->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectRadius.svg" ) ) );
  mActionDeselectAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeselectAll.svg" ) ) );
  mActionSelectAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectAll.svg" ) ) );
  mActionInvertSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInvertSelection.svg" ) ) );
  mActionSelectByExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionSelect.svg" ) ) );
  mActionSelectByForm->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFormSelect.svg" ) ) );
  mActionOpenTable->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  mActionOpenFieldCalc->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) ) );
  mActionMeasure->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeasure.svg" ) ) );
  mActionMeasureArea->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeasureArea.svg" ) ) );
  mActionMeasureAngle->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeasureAngle.svg" ) ) );
  mActionMapTips->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapTips.svg" ) ) );
  mActionShowBookmarks->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowBookmarks.svg" ) ) );
  mActionNewBookmark->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewBookmark.svg" ) ) );
  mActionCustomProjection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCustomProjection.svg" ) ) );
  mActionAddWmsLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWmsLayer.svg" ) ) );
  mActionAddWcsLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWcsLayer.svg" ) ) );
  mActionAddWfsLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWfsLayer.svg" ) ) );
  mActionAddAfsLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAfsLayer.svg" ) ) );
  mActionAddAmsLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAmsLayer.svg" ) ) );
  mActionAddToOverview->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInOverview.svg" ) ) );
  mActionAnnotation->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAnnotation.svg" ) ) );
  mActionFormAnnotation->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormAnnotation.svg" ) ) );
  mActionHtmlAnnotation->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHtmlAnnotation.svg" ) ) );
  mActionSvgAnnotation->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSvgAnnotation.svg" ) ) );
  mActionTextAnnotation->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionTextAnnotation.svg" ) ) );
  mActionLabeling->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabeling.svg" ) ) );
  mActionShowPinnedLabels->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowPinnedLabels.svg" ) ) );
  mActionPinLabels->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPinLabels.svg" ) ) );
  mActionShowHideLabels->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowHideLabels.svg" ) ) );
  mActionMoveLabel->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveLabel.svg" ) ) );
  mActionRotateLabel->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRotateLabel.svg" ) ) );
  mActionChangeLabelProperties->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionChangeLabelProperties.svg" ) ) );
  mActionDiagramProperties->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/diagram.svg" ) ) );
  mActionDecorationCopyright->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/copyright_label.svg" ) ) );
  mActionDecorationNorthArrow->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/north_arrow.png" ) ) );
  mActionDecorationScaleBar->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionScaleBar.svg" ) ) );
  mActionDecorationGrid->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/transformed.svg" ) ) );

  emit currentThemeChanged( themeName );
}

void QgisApp::setupConnections()
{
  // connect the "cleanup" slot
  connect( qApp, &QApplication::aboutToQuit, this, &QgisApp::saveWindowState );

  // signal when mouse moved over window (coords display in status bar)
  connect( mMapCanvas, &QgsMapCanvas::xyCoordinates, this, &QgisApp::saveLastMousePosition );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgisApp::extentChanged );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, &QgisApp::showScale );
  connect( mMapCanvas, &QgsMapCanvas::rotationChanged, this, &QgisApp::showRotation );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged,
           this, &QgisApp::updateMouseCoordinatePrecision );
  connect( mMapCanvas, &QgsMapCanvas::mapToolSet,
           this, &QgisApp::mapToolChanged );
  connect( mMapCanvas, &QgsMapCanvas::selectionChanged,
           this, &QgisApp::selectionChanged );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged,
           this, &QgisApp::markDirty );
  connect( mMapCanvas, &QgsMapCanvas::layersChanged,
           this, &QgisApp::markDirty );

  connect( mMapCanvas, &QgsMapCanvas::zoomLastStatusChanged,
           mActionZoomLast, &QAction::setEnabled );
  connect( mMapCanvas, &QgsMapCanvas::zoomNextStatusChanged,
           mActionZoomNext, &QAction::setEnabled );
  connect( mRenderSuppressionCBox, &QAbstractButton::toggled,
           this, [ = ]( bool flag )
  {
    Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
      canvas->setRenderFlag( flag );
  }
         );

  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged,
           this, &QgisApp::reprojectAnnotations );

  // connect MapCanvas keyPress event so we can check if selected feature collection must be deleted
  connect( mMapCanvas, &QgsMapCanvas::keyPressed,
           this, &QgisApp::mapCanvas_keyPressed );

  // project crs connections
  connect( QgsProject::instance(), &QgsProject::crsChanged, this, &QgisApp::projectCrsChanged );

  connect( QgsProject::instance(), &QgsProject::missingDatumTransforms, this, [ = ]( const QStringList & transforms )
  {
    QString message = tr( "Transforms are not installed: %1 " ).arg( transforms.join( QStringLiteral( " ," ) ) );
    messageBar()->pushWarning( tr( "Missing datum transforms" ), message );
  } );

  connect( QgsProject::instance(), &QgsProject::labelingEngineSettingsChanged,
           this, [ = ]
  {
    mMapCanvas->setLabelingEngineSettings( QgsProject::instance()->labelingEngineSettings() );
  } );

  // connect legend signals
  connect( mLayerTreeView, &QgsLayerTreeView::currentLayerChanged,
           this, &QgisApp::activateDeactivateLayerRelatedActions );
  connect( mLayerTreeView, &QgsLayerTreeView::currentLayerChanged,
           this, &QgisApp::setMapStyleDockLayer );

  connect( mLayerTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgisApp::legendLayerSelectionChanged );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::addedChildren,
           this, &QgisApp::markDirty );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::addedChildren,
           this, &QgisApp::updateNewLayerInsertionPoint );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::removedChildren,
           this, &QgisApp::markDirty );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::removedChildren,
           this, &QgisApp::updateNewLayerInsertionPoint );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::visibilityChanged,
           this, &QgisApp::markDirty );
  connect( mLayerTreeView->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::customPropertyChanged,
           this, &QgisApp::markDirty );

  // connect map layer registry
  connect( QgsProject::instance(), &QgsProject::layersAdded,
           this, &QgisApp::layersWereAdded );
  connect( QgsProject::instance(),
           static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ),
           this, &QgisApp::removingLayers );

  // connect initialization signal
  connect( this, &QgisApp::initializationCompleted,
           this, &QgisApp::fileOpenAfterLaunch );

  // Connect warning dialog from project reading
  connect( QgsProject::instance(), &QgsProject::oldProjectVersionWarning,
           this, &QgisApp::oldProjectVersionWarning );
  connect( QgsProject::instance(), &QgsProject::layerLoaded,
           this, &QgisApp::showProgress );
  connect( QgsProject::instance(), &QgsProject::loadingLayer,
           this, &QgisApp::showStatusMessage );
  connect( QgsProject::instance(), &QgsProject::readProject,
           this, &QgisApp::readProject );
  connect( QgsProject::instance(), &QgsProject::writeProject,
           this, &QgisApp::writeProject );

  connect( this, &QgisApp::projectRead,
           this, &QgisApp::fileOpenedOKAfterLaunch );

  connect( QgsProject::instance(), &QgsProject::transactionGroupsChanged, this, &QgisApp::onTransactionGroupsChanged );

  // connect preview modes actions
  connect( mActionPreviewModeOff, &QAction::triggered, this, &QgisApp::disablePreviewMode );
  connect( mActionPreviewModeGrayscale, &QAction::triggered, this, &QgisApp::activateGrayscalePreview );
  connect( mActionPreviewModeMono, &QAction::triggered, this, &QgisApp::activateMonoPreview );
  connect( mActionPreviewProtanope, &QAction::triggered, this, &QgisApp::activateProtanopePreview );
  connect( mActionPreviewDeuteranope, &QAction::triggered, this, &QgisApp::activateDeuteranopePreview );

  // setup undo/redo actions
  connect( mUndoWidget, &QgsUndoWidget::undoStackChanged, this, &QgisApp::updateUndoActions );

  connect( mLayoutsMenu, &QMenu::aboutToShow, this, &QgisApp::layoutsMenuAboutToShow );
}

void QgisApp::createCanvasTools()
{
  // create tools
  mMapTools.mZoomIn = new QgsMapToolZoom( mMapCanvas, false /* zoomIn */ );
  mMapTools.mZoomIn->setAction( mActionZoomIn );
  mMapTools.mZoomOut = new QgsMapToolZoom( mMapCanvas, true /* zoomOut */ );
  mMapTools.mZoomOut->setAction( mActionZoomOut );
  mMapTools.mPan = new QgsMapToolPan( mMapCanvas );
  mMapTools.mPan->setAction( mActionPan );
  mMapTools.mIdentify = new QgsMapToolIdentifyAction( mMapCanvas );
  mMapTools.mIdentify->setAction( mActionIdentify );
  connect( mMapTools.mIdentify, &QgsMapToolIdentifyAction::copyToClipboard,
           this, &QgisApp::copyFeatures );
  mMapTools.mFeatureAction = new QgsMapToolFeatureAction( mMapCanvas );
  mMapTools.mFeatureAction->setAction( mActionFeatureAction );
  mMapTools.mMeasureDist = new QgsMeasureTool( mMapCanvas, false /* area */ );
  mMapTools.mMeasureDist->setAction( mActionMeasure );
  mMapTools.mMeasureArea = new QgsMeasureTool( mMapCanvas, true /* area */ );
  mMapTools.mMeasureArea->setAction( mActionMeasureArea );
  mMapTools.mMeasureAngle = new QgsMapToolMeasureAngle( mMapCanvas );
  mMapTools.mMeasureAngle->setAction( mActionMeasureAngle );
  mMapTools.mTextAnnotation = new QgsMapToolTextAnnotation( mMapCanvas );
  mMapTools.mTextAnnotation->setAction( mActionTextAnnotation );
  mMapTools.mFormAnnotation = new QgsMapToolFormAnnotation( mMapCanvas );
  mMapTools.mFormAnnotation->setAction( mActionFormAnnotation );
  mMapTools.mHtmlAnnotation = new QgsMapToolHtmlAnnotation( mMapCanvas );
  mMapTools.mHtmlAnnotation->setAction( mActionHtmlAnnotation );
  mMapTools.mSvgAnnotation = new QgsMapToolSvgAnnotation( mMapCanvas );
  mMapTools.mSvgAnnotation->setAction( mActionSvgAnnotation );
  mMapTools.mAnnotation = new QgsMapToolAnnotation( mMapCanvas );
  mMapTools.mAnnotation->setAction( mActionAnnotation );
  mMapTools.mAddFeature = new QgsMapToolAddFeature( mMapCanvas, QgsMapToolCapture::CaptureNone );
  mMapTools.mAddFeature->setAction( mActionAddFeature );
  mMapTools.mCircularStringCurvePoint = new QgsMapToolCircularStringCurvePoint( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircularStringCurvePoint->setAction( mActionCircularStringCurvePoint );
  mMapTools.mCircularStringRadius = new QgsMapToolCircularStringRadius( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircularStringRadius->setAction( mActionCircularStringRadius );
  mMapTools.mCircle2Points = new QgsMapToolCircle2Points( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircle2Points->setAction( mActionCircle2Points );
  mMapTools.mCircle3Points = new QgsMapToolCircle3Points( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircle3Points->setAction( mActionCircle3Points );
  mMapTools.mCircle3Tangents = new QgsMapToolCircle3Tangents( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircle3Tangents->setAction( mActionCircle3Tangents );
  mMapTools.mCircle2TangentsPoint = new QgsMapToolCircle2TangentsPoint( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircle2TangentsPoint->setAction( mActionCircle2TangentsPoint );
  mMapTools.mCircleCenterPoint = new QgsMapToolCircleCenterPoint( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mCircleCenterPoint->setAction( mActionCircleCenterPoint );
  mMapTools.mEllipseCenter2Points = new QgsMapToolEllipseCenter2Points( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mEllipseCenter2Points->setAction( mActionEllipseCenter2Points );
  mMapTools.mEllipseCenterPoint = new QgsMapToolEllipseCenterPoint( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mEllipseCenterPoint->setAction( mActionEllipseCenterPoint );
  mMapTools.mEllipseExtent = new QgsMapToolEllipseExtent( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mEllipseExtent->setAction( mActionEllipseExtent );
  mMapTools.mEllipseFoci = new QgsMapToolEllipseFoci( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mEllipseFoci->setAction( mActionEllipseFoci );
  mMapTools.mRectangleCenterPoint = new QgsMapToolRectangleCenter( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRectangleCenterPoint->setAction( mActionRectangleCenterPoint );
  mMapTools.mRectangleExtent = new QgsMapToolRectangleExtent( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRectangleExtent->setAction( mActionRectangleExtent );
  mMapTools.mRectangle3Points = new QgsMapToolRectangle3Points( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRectangle3Points->setAction( mActionRectangle3Points );
  mMapTools.mRegularPolygon2Points = new QgsMapToolRegularPolygon2Points( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRegularPolygon2Points->setAction( mActionRegularPolygon2Points );
  mMapTools.mRegularPolygonCenterPoint = new QgsMapToolRegularPolygonCenterPoint( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRegularPolygonCenterPoint->setAction( mActionRegularPolygonCenterPoint );
  mMapTools.mRegularPolygonCenterCorner = new QgsMapToolRegularPolygonCenterCorner( mMapTools.mAddFeature, mMapCanvas );
  mMapTools.mRegularPolygonCenterCorner->setAction( mActionRegularPolygonCenterCorner );
  mMapTools.mMoveFeature = new QgsMapToolMoveFeature( mMapCanvas, QgsMapToolMoveFeature::Move );
  mMapTools.mMoveFeature->setAction( mActionMoveFeature );
  mMapTools.mMoveFeatureCopy = new QgsMapToolMoveFeature( mMapCanvas, QgsMapToolMoveFeature::CopyMove );
  mMapTools.mMoveFeatureCopy->setAction( mActionMoveFeatureCopy );
  mMapTools.mRotateFeature = new QgsMapToolRotateFeature( mMapCanvas );
  mMapTools.mRotateFeature->setAction( mActionRotateFeature );
  mMapTools.mOffsetCurve = new QgsMapToolOffsetCurve( mMapCanvas );
  mMapTools.mOffsetCurve->setAction( mActionOffsetCurve );
  mMapTools.mReshapeFeatures = new QgsMapToolReshape( mMapCanvas );
  mMapTools.mReshapeFeatures->setAction( mActionReshapeFeatures );
  mMapTools.mSplitFeatures = new QgsMapToolSplitFeatures( mMapCanvas );
  mMapTools.mSplitFeatures->setAction( mActionSplitFeatures );
  mMapTools.mSplitParts = new QgsMapToolSplitParts( mMapCanvas );
  mMapTools.mSplitParts->setAction( mActionSplitParts );
  mMapTools.mSelectFeatures = new QgsMapToolSelectFeatures( mMapCanvas );
  mMapTools.mSelectFeatures->setAction( mActionSelectFeatures );
  mMapTools.mSelectPolygon = new QgsMapToolSelectPolygon( mMapCanvas );
  mMapTools.mSelectPolygon->setAction( mActionSelectPolygon );
  mMapTools.mSelectFreehand = new QgsMapToolSelectFreehand( mMapCanvas );
  mMapTools.mSelectFreehand->setAction( mActionSelectFreehand );
  mMapTools.mSelectRadius = new QgsMapToolSelectRadius( mMapCanvas );
  mMapTools.mSelectRadius->setAction( mActionSelectRadius );
  mMapTools.mAddRing = new QgsMapToolAddRing( mMapCanvas );
  mMapTools.mAddRing->setAction( mActionAddRing );
  mMapTools.mFillRing = new QgsMapToolFillRing( mMapCanvas );
  mMapTools.mFillRing->setAction( mActionFillRing );
  mMapTools.mAddPart = new QgsMapToolAddPart( mMapCanvas );
  mMapTools.mAddPart->setAction( mActionAddPart );
  mMapTools.mSimplifyFeature = new QgsMapToolSimplify( mMapCanvas );
  mMapTools.mSimplifyFeature->setAction( mActionSimplifyFeature );
  mMapTools.mDeleteRing = new QgsMapToolDeleteRing( mMapCanvas );
  mMapTools.mDeleteRing->setAction( mActionDeleteRing );
  mMapTools.mDeletePart = new QgsMapToolDeletePart( mMapCanvas );
  mMapTools.mDeletePart->setAction( mActionDeletePart );
  mMapTools.mNodeTool = new QgsNodeTool( mMapCanvas, mAdvancedDigitizingDockWidget );
  mMapTools.mNodeTool->setAction( mActionNodeTool );
  mMapTools.mRotatePointSymbolsTool = new QgsMapToolRotatePointSymbols( mMapCanvas );
  mMapTools.mRotatePointSymbolsTool->setAction( mActionRotatePointSymbols );
  mMapTools.mOffsetPointSymbolTool = new QgsMapToolOffsetPointSymbol( mMapCanvas );
  mMapTools.mOffsetPointSymbolTool->setAction( mActionOffsetPointSymbol );

  mMapTools.mPinLabels = new QgsMapToolPinLabels( mMapCanvas );
  mMapTools.mPinLabels->setAction( mActionPinLabels );
  mMapTools.mShowHideLabels = new QgsMapToolShowHideLabels( mMapCanvas );
  mMapTools.mShowHideLabels->setAction( mActionShowHideLabels );
  mMapTools.mMoveLabel = new QgsMapToolMoveLabel( mMapCanvas );
  mMapTools.mMoveLabel->setAction( mActionMoveLabel );

  mMapTools.mRotateLabel = new QgsMapToolRotateLabel( mMapCanvas );
  mMapTools.mRotateLabel->setAction( mActionRotateLabel );
  mMapTools.mChangeLabelProperties = new QgsMapToolChangeLabelProperties( mMapCanvas );
  mMapTools.mChangeLabelProperties->setAction( mActionChangeLabelProperties );
//ensure that non edit tool is initialized or we will get crashes in some situations
  mNonEditMapTool = mMapTools.mPan;
}

void QgisApp::createOverview()
{
  // overview canvas
  mOverviewCanvas = new QgsMapOverviewCanvas( nullptr, mMapCanvas );

  //set canvas color to default
  QgsSettings settings;
  int red = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  int green = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  int blue = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  mOverviewCanvas->setBackgroundColor( QColor( red, green, blue ) );

  mOverviewCanvas->setWhatsThis( tr( "Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas." ) );

  mOverviewMapCursor = new QCursor( Qt::OpenHandCursor );
  mOverviewCanvas->setCursor( *mOverviewMapCursor );
//  QVBoxLayout *myOverviewLayout = new QVBoxLayout;
//  myOverviewLayout->addWidget(overviewCanvas);
//  overviewFrame->setLayout(myOverviewLayout);
  mOverviewDock = new QgsDockWidget( tr( "Overview Panel" ), this );
  mOverviewDock->setObjectName( QStringLiteral( "Overview" ) );
  mOverviewDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mOverviewDock->setWidget( mOverviewCanvas );
  addDockWidget( Qt::LeftDockWidgetArea, mOverviewDock );
  // add to the Panel submenu
  mPanelMenu->addAction( mOverviewDock->toggleViewAction() );

  mLayerTreeCanvasBridge->setOvervewCanvas( mOverviewCanvas );
}

void QgisApp::addDockWidget( Qt::DockWidgetArea area, QDockWidget *thepDockWidget )
{
  QMainWindow::addDockWidget( area, thepDockWidget );
  // Make the right and left docks consume all vertical space and top
  // and bottom docks nest between them
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
  // add to the Panel submenu
  mPanelMenu->addAction( thepDockWidget->toggleViewAction() );

  thepDockWidget->show();

  // refresh the map canvas
  refreshMapCanvas();
}

void QgisApp::removeDockWidget( QDockWidget *thepDockWidget )
{
  QMainWindow::removeDockWidget( thepDockWidget );
  mPanelMenu->removeAction( thepDockWidget->toggleViewAction() );
}

QToolBar *QgisApp::addToolBar( const QString &name )
{
  QToolBar *toolBar = QMainWindow::addToolBar( name );
  // add to the Toolbar submenu
  mToolbarMenu->addAction( toolBar->toggleViewAction() );
  return toolBar;
}

void QgisApp::addToolBar( QToolBar *toolBar, Qt::ToolBarArea area )
{
  QMainWindow::addToolBar( area, toolBar );
  // add to the Toolbar submenu
  mToolbarMenu->addAction( toolBar->toggleViewAction() );
}

QgsLayerTreeView *QgisApp::layerTreeView()
{
  Q_ASSERT( mLayerTreeView );
  return mLayerTreeView;
}

QgsPluginManager *QgisApp::pluginManager()
{
  Q_ASSERT( mPluginManager );
  return mPluginManager;
}

QgsUserProfileManager *QgisApp::userProfileManager()
{
  Q_ASSERT( mUserProfileManager );
  return mUserProfileManager;
}

QgsMapCanvas *QgisApp::mapCanvas()
{
  Q_ASSERT( mMapCanvas );
  return mMapCanvas;
}

QgsMapCanvas *QgisApp::createNewMapCanvas( const QString &name )
{
  QgsMapCanvasDockWidget *dock = createNewMapCanvasDock( name );
  if ( !dock )
    return nullptr;

  setupDockWidget( dock );  // use default dock position settings

  dock->mapCanvas()->setLayers( mMapCanvas->layers() );
  dock->mapCanvas()->setExtent( mMapCanvas->extent() );
  QgsDebugMsgLevel( QString( "QgisApp::createNewMapCanvas -2- : QgsProject::instance()->crs().description[%1]ellipsoid[%2]" ).arg( QgsProject::instance()->crs().description() ).arg( QgsProject::instance()->crs().ellipsoidAcronym() ), 3 );
  dock->mapCanvas()->setDestinationCrs( QgsProject::instance()->crs() );
  dock->mapCanvas()->freeze( false );
  return dock->mapCanvas();
}

QgsMapCanvasDockWidget *QgisApp::createNewMapCanvasDock( const QString &name )
{
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
  {
    if ( canvas->objectName() == name )
    {
      QgsDebugMsg( tr( "A map canvas with name '%1' already exists!" ).arg( name ) );
      return nullptr;
    }
  }

  QgsMapCanvasDockWidget *mapCanvasWidget = new QgsMapCanvasDockWidget( name, this );
  mapCanvasWidget->setAllowedAreas( Qt::AllDockWidgetAreas );
  mapCanvasWidget->setMainCanvas( mMapCanvas );

  QgsMapCanvas *mapCanvas = mapCanvasWidget->mapCanvas();
  mapCanvas->freeze( true );
  mapCanvas->setObjectName( name );
  connect( mapCanvas, &QgsMapCanvas::messageEmitted, this, &QgisApp::displayMessage );
  connect( mLayerTreeCanvasBridge, &QgsLayerTreeMapCanvasBridge::canvasLayersChanged, mapCanvas, &QgsMapCanvas::setLayers );

  applyProjectSettingsToCanvas( mapCanvas );
  applyDefaultSettingsToCanvas( mapCanvas );

  // add existing annotations to canvas
  Q_FOREACH ( QgsAnnotation *annotation, QgsProject::instance()->annotationManager()->annotations() )
  {
    QgsMapCanvasAnnotationItem *canvasItem = new QgsMapCanvasAnnotationItem( annotation, mapCanvas );
    Q_UNUSED( canvasItem ); //item is already added automatically to canvas scene
  }

  markDirty();
  connect( mapCanvasWidget, &QgsMapCanvasDockWidget::closed, this, &QgisApp::markDirty );
  connect( mapCanvasWidget, &QgsMapCanvasDockWidget::renameTriggered, this, &QgisApp::renameView );

  return mapCanvasWidget;
}


void QgisApp::setupDockWidget( QDockWidget *dockWidget, bool isFloating, const QRect &dockGeometry, Qt::DockWidgetArea area )
{
  dockWidget->setFloating( isFloating );
  if ( dockGeometry.isEmpty() )
  {
    // try to guess a nice initial placement for view - about 3/4 along, half way down
    dockWidget->setGeometry( QRect( rect().width() * 0.75, rect().height() * 0.5, 400, 400 ) );
    addDockWidget( area, dockWidget );
  }
  else
  {
    if ( !isFloating )
    {
      // ugly hack, but only way to set dock size correctly for Qt < 5.6
      dockWidget->setFixedSize( dockGeometry.size() );
      addDockWidget( area, dockWidget );
      dockWidget->resize( dockGeometry.size() );
      QgsApplication::processEvents(); // required!
      dockWidget->setFixedSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    else
    {
      dockWidget->setGeometry( dockGeometry );
      addDockWidget( area, dockWidget );
    }
  }
}

void QgisApp::closeMapCanvas( const QString &name )
{
  Q_FOREACH ( QgsMapCanvasDockWidget *w, findChildren< QgsMapCanvasDockWidget * >() )
  {
    if ( w->mapCanvas()->objectName() == name )
    {
      w->close();
      delete w;
      break;
    }
  }
}

void QgisApp::closeAdditionalMapCanvases()
{
  freezeCanvases( true ); // closing docks may cause canvases to resize, and we don't want a map refresh occurring
  Q_FOREACH ( QgsMapCanvasDockWidget *w, findChildren< QgsMapCanvasDockWidget * >() )
  {
    w->close();
    delete w;
  }
  freezeCanvases( false );
}

void QgisApp::closeAdditional3DMapCanvases()
{
#ifdef HAVE_3D
  for ( Qgs3DMapCanvasDockWidget *w : findChildren< Qgs3DMapCanvasDockWidget * >() )
  {
    w->close();
    delete w;
  }
#endif
}

void QgisApp::freezeCanvases( bool frozen )
{
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
  {
    canvas->freeze( frozen );
  }
}

QgsMessageBar *QgisApp::messageBar()
{
  Q_ASSERT( mInfoBar );
  return mInfoBar;
}

void QgisApp::toggleLogMessageIcon( bool hasLogMessage )
{
  if ( hasLogMessage && !mLogDock->isVisible() )
  {
    mMessageButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mMessageLog.svg" ) ) );
  }
  else
  {
    mMessageButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mMessageLogRead.svg" ) ) );
  }
}

void QgisApp::openMessageLog()
{
  mMessageButton->setChecked( true );
}

void QgisApp::addUserInputWidget( QWidget *widget )
{
  mUserInputDockWidget->addUserInputWidget( widget );
}


void QgisApp::initLayerTreeView()
{
  mLayerTreeView->setWhatsThis( tr( "Map legend that displays all the layers currently on the map canvas. Click on the checkbox to turn a layer on or off. Double-click on a layer in the legend to customize its appearance and set other properties." ) );

  mLayerTreeDock = new QgsDockWidget( tr( "Layers Panel" ), this );
  mLayerTreeDock->setObjectName( QStringLiteral( "Layers" ) );
  mLayerTreeDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

  QgsLayerTreeModel *model = new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), this );
#ifdef ENABLE_MODELTEST
  new ModelTest( model, this );
#endif
  model->setFlag( QgsLayerTreeModel::AllowNodeReorder );
  model->setFlag( QgsLayerTreeModel::AllowNodeRename );
  model->setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility );
  model->setFlag( QgsLayerTreeModel::ShowLegendAsTree );
  model->setFlag( QgsLayerTreeModel::UseEmbeddedWidgets );
  model->setAutoCollapseLegendNodes( 10 );

  mLayerTreeView->setModel( model );
  mLayerTreeView->setMenuProvider( new QgsAppLayerTreeViewMenuProvider( mLayerTreeView, mMapCanvas ) );

  setupLayerTreeViewFromSettings();

  connect( mLayerTreeView, &QAbstractItemView::doubleClicked, this, &QgisApp::layerTreeViewDoubleClicked );
  connect( mLayerTreeView, &QgsLayerTreeView::currentLayerChanged, this, &QgisApp::activeLayerChanged );
  connect( mLayerTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgisApp::updateNewLayerInsertionPoint );
  connect( QgsProject::instance()->layerTreeRegistryBridge(), &QgsLayerTreeRegistryBridge::addedLayersToLayerTree,
           this, &QgisApp::autoSelectAddedLayer );

  // add group action
  QAction *actionAddGroup = new QAction( tr( "Add Group" ), this );
  actionAddGroup->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddGroup.svg" ) ) );
  actionAddGroup->setToolTip( tr( "Add Group" ) );
  connect( actionAddGroup, &QAction::triggered, mLayerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup );

  // visibility groups tool button
  QToolButton *btnVisibilityPresets = new QToolButton;
  btnVisibilityPresets->setAutoRaise( true );
  btnVisibilityPresets->setToolTip( tr( "Manage Map Themes" ) );
  btnVisibilityPresets->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ) );
  btnVisibilityPresets->setPopupMode( QToolButton::InstantPopup );
  btnVisibilityPresets->setMenu( QgsMapThemes::instance()->menu() );

  // filter legend action
  mActionFilterLegend = new QAction( tr( "Filter Legend By Map Content" ), this );
  mActionFilterLegend->setCheckable( true );
  mActionFilterLegend->setToolTip( tr( "Filter Legend By Map Content" ) );
  mActionFilterLegend->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFilter2.svg" ) ) );
  connect( mActionFilterLegend, &QAction::toggled, this, &QgisApp::updateFilterLegend );

  mLegendExpressionFilterButton = new QgsLegendFilterButton( this );
  mLegendExpressionFilterButton->setToolTip( tr( "Filter legend by expression" ) );
  connect( mLegendExpressionFilterButton, &QAbstractButton::toggled, this, &QgisApp::toggleFilterLegendByExpression );

  mActionStyleDock = new QAction( tr( "Layer Styling" ), this );
  mActionStyleDock->setCheckable( true );
  mActionStyleDock->setToolTip( tr( "Open the layer styling dock" ) );
  mActionStyleDock->setShortcut( QStringLiteral( "F7" ) );
  mActionStyleDock->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ) );
  connect( mActionStyleDock, &QAction::toggled, this, &QgisApp::mapStyleDock );

  // expand / collapse tool buttons
  QAction *actionExpandAll = new QAction( tr( "Expand All" ), this );
  actionExpandAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionExpandTree.svg" ) ) );
  actionExpandAll->setToolTip( tr( "Expand All" ) );
  connect( actionExpandAll, &QAction::triggered, mLayerTreeView, &QgsLayerTreeView::expandAllNodes );
  QAction *actionCollapseAll = new QAction( tr( "Collapse All" ), this );
  actionCollapseAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCollapseTree.svg" ) ) );
  actionCollapseAll->setToolTip( tr( "Collapse All" ) );
  connect( actionCollapseAll, &QAction::triggered, mLayerTreeView, &QgsLayerTreeView::collapseAllNodes );

  QToolBar *toolbar = new QToolBar();
  toolbar->setIconSize( iconSize( true ) );
  toolbar->addAction( mActionStyleDock );
  toolbar->addAction( actionAddGroup );
  toolbar->addWidget( btnVisibilityPresets );
  toolbar->addAction( mActionFilterLegend );
  toolbar->addWidget( mLegendExpressionFilterButton );
  toolbar->addAction( actionExpandAll );
  toolbar->addAction( actionCollapseAll );
  toolbar->addAction( mActionRemoveLayer );

  QVBoxLayout *vboxLayout = new QVBoxLayout;
  vboxLayout->setMargin( 0 );
  vboxLayout->setContentsMargins( 0, 0, 0, 0 );
  vboxLayout->setSpacing( 0 );
  vboxLayout->addWidget( toolbar );
  vboxLayout->addWidget( mLayerTreeView );

  QWidget *w = new QWidget;
  w->setLayout( vboxLayout );
  mLayerTreeDock->setWidget( w );
  addDockWidget( Qt::LeftDockWidgetArea, mLayerTreeDock );

  mLayerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge( QgsProject::instance()->layerTreeRoot(), mMapCanvas, this );

  mMapLayerOrder = new QgsCustomLayerOrderWidget( mLayerTreeCanvasBridge, this );
  mMapLayerOrder->setObjectName( QStringLiteral( "theMapLayerOrder" ) );

  mMapLayerOrder->setWhatsThis( tr( "Map layer list that displays all layers in drawing order." ) );
  mLayerOrderDock = new QgsDockWidget( tr( "Layer Order Panel" ), this );
  mLayerOrderDock->setObjectName( QStringLiteral( "LayerOrder" ) );
  mLayerOrderDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

  mLayerOrderDock->setWidget( mMapLayerOrder );
  addDockWidget( Qt::LeftDockWidgetArea, mLayerOrderDock );
  mLayerOrderDock->hide();

  connect( mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &QgisApp::updateFilterLegend );
}

void QgisApp::setupLayerTreeViewFromSettings()
{
  QgsSettings s;

  QgsLayerTreeModel *model = mLayerTreeView->layerTreeModel();
  QFont fontLayer, fontGroup;
  fontLayer.setBold( true );
  fontGroup.setBold( false );
  model->setLayerTreeNodeFont( QgsLayerTreeNode::NodeLayer, fontLayer );
  model->setLayerTreeNodeFont( QgsLayerTreeNode::NodeGroup, fontGroup );
}


void QgisApp::updateNewLayerInsertionPoint()
{
  // defaults
  QgsLayerTreeGroup *parentGroup = mLayerTreeView->layerTreeModel()->rootGroup();
  int index = 0;
  QModelIndex current = mLayerTreeView->currentIndex();

  if ( current.isValid() )
  {
    if ( QgsLayerTreeNode *currentNode = mLayerTreeView->currentNode() )
    {
      // if the insertion point is actually a group, insert new layers into the group
      if ( QgsLayerTree::isGroup( currentNode ) )
      {
        QgsProject::instance()->layerTreeRegistryBridge()->setLayerInsertionPoint( QgsLayerTree::toGroup( currentNode ), 0 );
        return;
      }

      // otherwise just set the insertion point in front of the current node
      QgsLayerTreeNode *parentNode = currentNode->parent();
      if ( QgsLayerTree::isGroup( parentNode ) )
        parentGroup = QgsLayerTree::toGroup( parentNode );
    }

    index = current.row();
  }

  QgsProject::instance()->layerTreeRegistryBridge()->setLayerInsertionPoint( parentGroup, index );
}

void QgisApp::autoSelectAddedLayer( QList<QgsMapLayer *> layers )
{
  if ( !layers.isEmpty() )
  {
    QgsLayerTreeLayer *nodeLayer = QgsProject::instance()->layerTreeRoot()->findLayer( layers[0]->id() );

    if ( !nodeLayer )
      return;

    QModelIndex index = mLayerTreeView->layerTreeModel()->node2index( nodeLayer );
    mLayerTreeView->setCurrentIndex( index );
  }
}

void QgisApp::createMapTips()
{
  // Set up the timer for maptips. The timer is reset every time the mouse is moved
  mpMapTipsTimer = new QTimer( mMapCanvas );
  // connect the timer to the maptips slot
  connect( mpMapTipsTimer, &QTimer::timeout, this, &QgisApp::showMapTip );
  // set the interval to 0.850 seconds - timer will be started next time the mouse moves
  mpMapTipsTimer->setInterval( 850 );
  mpMapTipsTimer->setSingleShot( true );

  // Create the maptips object
  mpMaptip = new QgsMapTip();
}

void QgisApp::createDecorations()
{
  QgsDecorationCopyright *mDecorationCopyright = new QgsDecorationCopyright( this );
  connect( mActionDecorationCopyright, &QAction::triggered, mDecorationCopyright, &QgsDecorationCopyright::run );

  QgsDecorationNorthArrow *mDecorationNorthArrow = new QgsDecorationNorthArrow( this );
  connect( mActionDecorationNorthArrow, &QAction::triggered, mDecorationNorthArrow, &QgsDecorationNorthArrow::run );

  QgsDecorationScaleBar *mDecorationScaleBar = new QgsDecorationScaleBar( this );
  connect( mActionDecorationScaleBar, &QAction::triggered, mDecorationScaleBar, &QgsDecorationScaleBar::run );

  QgsDecorationGrid *mDecorationGrid = new QgsDecorationGrid( this );
  connect( mActionDecorationGrid, &QAction::triggered, mDecorationGrid, &QgsDecorationGrid::run );

  QgsDecorationLayoutExtent *decorationLayoutExtent = new QgsDecorationLayoutExtent( this );
  connect( mActionDecorationLayoutExtent, &QAction::triggered, decorationLayoutExtent, &QgsDecorationLayoutExtent::run );

  // add the decorations in a particular order so they are rendered in that order
  addDecorationItem( mDecorationGrid );
  addDecorationItem( mDecorationCopyright );
  addDecorationItem( mDecorationNorthArrow );
  addDecorationItem( mDecorationScaleBar );
  addDecorationItem( decorationLayoutExtent );
  connect( mMapCanvas, &QgsMapCanvas::renderComplete, this, &QgisApp::renderDecorationItems );
  connect( this, &QgisApp::newProject, this, &QgisApp::projectReadDecorationItems );
  connect( this, &QgisApp::projectRead, this, &QgisApp::projectReadDecorationItems );
}

void QgisApp::renderDecorationItems( QPainter *p )
{
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() );
  context.setPainter( p );

  Q_FOREACH ( QgsDecorationItem *item, mDecorationItems )
  {
    item->render( mMapCanvas->mapSettings(), context );
  }
}

void QgisApp::projectReadDecorationItems()
{
  Q_FOREACH ( QgsDecorationItem *item, mDecorationItems )
  {
    item->projectRead();
  }
}

// Update project menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
  mRecentProjectsMenu->clear();

  Q_FOREACH ( const QgsWelcomePageItemsModel::RecentProjectData &recentProject, mRecentProjects )
  {
    QAction *action = mRecentProjectsMenu->addAction( QStringLiteral( "%1 (%2)" ).arg( recentProject.title != recentProject.path ? recentProject.title : QFileInfo( recentProject.path ).completeBaseName(),
                      QDir::toNativeSeparators( recentProject.path ) ) );
    //action->setEnabled( QFile::exists( ( recentProject.path ) ) );
    action->setData( recentProject.path );
    if ( recentProject.pin )
    {
      action->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/pin.svg" ) ) );
    }
  }

#if defined(Q_OS_WIN)
  QWinJumpList jumplist;
  jumplist.recent()->clear();
  Q_FOREACH ( const QgsWelcomePageItemsModel::RecentProjectData &recentProject, mRecentProjects )
  {
    QString name = recentProject.title != recentProject.path ? recentProject.title : QFileInfo( recentProject.path ).baseName();
    QWinJumpListItem *newProject = new QWinJumpListItem( QWinJumpListItem::Link );
    newProject->setTitle( name );
    newProject->setFilePath( QDir::toNativeSeparators( QCoreApplication::applicationFilePath() ) );
    newProject->setArguments( QStringList( recentProject.path ) );
    jumplist.recent()->addItem( newProject );
  }
#endif

} // QgisApp::updateRecentProjectPaths

// add this file to the recently opened/saved projects list
void QgisApp::saveRecentProjectPath( const QString &projectPath, bool savePreviewImage )
{
  // first, re-read the recent project paths. This prevents loss of recent
  // projects when multiple QGIS sessions are open
  readRecentProjects();

  // Get canonical absolute path
  QFileInfo myFileInfo( projectPath );
  QgsWelcomePageItemsModel::RecentProjectData projectData;
  projectData.path = myFileInfo.absoluteFilePath();
  projectData.title = QgsProject::instance()->title();
  if ( projectData.title.isEmpty() )
    projectData.title = projectData.path;

  projectData.crs = QgsProject::instance()->crs().authid();

  int idx = mRecentProjects.indexOf( projectData );
  if ( idx != -1 )
    projectData.pin = mRecentProjects.at( idx ).pin;

  if ( savePreviewImage )
  {
    // Generate a unique file name
    QString fileName( QCryptographicHash::hash( ( projectData.path.toUtf8() ), QCryptographicHash::Md5 ).toHex() );
    QString previewDir = QStringLiteral( "%1/previewImages" ).arg( QgsApplication::qgisSettingsDirPath() );
    projectData.previewImagePath = QStringLiteral( "%1/%2.png" ).arg( previewDir, fileName );
    QDir().mkdir( previewDir );

    // Render the map canvas
    QSize previewSize( 250, 177 ); // h = w / std::sqrt(2)
    QRect previewRect( QPoint( ( mMapCanvas->width() - previewSize.width() ) / 2
                               , ( mMapCanvas->height() - previewSize.height() ) / 2 )
                       , previewSize );

    QPixmap previewImage( previewSize );
    QPainter previewPainter( &previewImage );
    mMapCanvas->render( &previewPainter, QRect( QPoint(), previewSize ), previewRect );

    // Save
    previewImage.save( projectData.previewImagePath );
  }
  else
  {
    if ( idx != -1 )
      projectData.previewImagePath = mRecentProjects.at( idx ).previewImagePath;
  }

  // Count the number of pinned items, those shouldn't affect trimming
  int pinnedCount = 0;
  int nonPinnedPos = 0;
  bool pinnedTop = true;
  Q_FOREACH ( const QgsWelcomePageItemsModel::RecentProjectData &recentProject, mRecentProjects )
  {
    if ( recentProject.pin )
    {
      pinnedCount++;
      if ( pinnedTop )
      {
        nonPinnedPos++;
      }
    }
    else if ( pinnedTop )
    {
      pinnedTop = false;
    }
  }

  // If this file is already in the list, remove it
  mRecentProjects.removeAll( projectData );

  // Insert this file to the list
  mRecentProjects.insert( projectData.pin ? 0 : nonPinnedPos, projectData );

  // Keep the list to 10 items by trimming excess off the bottom
  // And remove the associated image
  while ( mRecentProjects.count() > 10 + pinnedCount )
  {
    QFile( mRecentProjects.takeLast().previewImagePath ).remove();
  }

  // Persist the list
  saveRecentProjects();

  // Update menu list of paths
  updateRecentProjectPaths();

  // Update welcome page list
  if ( mWelcomePage )
    mWelcomePage->setRecentProjects( mRecentProjects );

} // QgisApp::saveRecentProjectPath

// Save recent projects list to settings
void QgisApp::saveRecentProjects()
{
  QgsSettings settings;

  settings.remove( QStringLiteral( "/UI/recentProjects" ) );
  int idx = 0;

  Q_FOREACH ( const QgsWelcomePageItemsModel::RecentProjectData &recentProject, mRecentProjects )
  {
    ++idx;
    settings.beginGroup( QStringLiteral( "UI/recentProjects/%1" ).arg( idx ) );
    settings.setValue( QStringLiteral( "title" ), recentProject.title );
    settings.setValue( QStringLiteral( "path" ), recentProject.path );
    settings.setValue( QStringLiteral( "previewImage" ), recentProject.previewImagePath );
    settings.setValue( QStringLiteral( "crs" ), recentProject.crs );
    settings.setValue( QStringLiteral( "pin" ), recentProject.pin );
    settings.endGroup();
  }
}

// Update project menu with the project templates
void QgisApp::updateProjectFromTemplates()
{
  // get list of project files in template dir
  QgsSettings settings;
  QString templateDirName = settings.value( QStringLiteral( "qgis/projectTemplateDir" ),
                            QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
  QDir templateDir( templateDirName );
  QStringList filters( QStringLiteral( "*.qgs" ) );
  templateDir.setNameFilters( filters );
  QStringList templateFiles = templateDir.entryList( filters );

  // Remove existing entries
  mProjectFromTemplateMenu->clear();

  // Add entries
  Q_FOREACH ( const QString &templateFile, templateFiles )
  {
    mProjectFromTemplateMenu->addAction( templateFile );
  }

  // add <blank> entry, which loads a blank template (regardless of "default template")
  if ( settings.value( QStringLiteral( "qgis/newProjectDefault" ), QVariant( false ) ).toBool() )
    mProjectFromTemplateMenu->addAction( tr( "< Blank >" ) );

} // QgisApp::updateProjectFromTemplates

void QgisApp::saveWindowState()
{
  // store window and toolbar positions
  QgsSettings settings;
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue( QStringLiteral( "UI/state" ), saveState() );

  // store window geometry
  settings.setValue( QStringLiteral( "UI/geometry" ), saveGeometry() );

  QgsPluginRegistry::instance()->unloadAll();
}

#include "ui_defaults.h"

void QgisApp::restoreWindowState()
{
  // restore the toolbar and dock widgets positions using Qt4 settings API
  QgsSettings settings;

  if ( !restoreState( settings.value( QStringLiteral( "UI/state" ), QByteArray::fromRawData( reinterpret_cast< const char * >( defaultUIstate ), sizeof defaultUIstate ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI state failed" );
  }

  // restore window geometry
  if ( !restoreGeometry( settings.value( QStringLiteral( "UI/geometry" ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI geometry failed" );
    // default to 80% of screen size, at 10% from top left corner
    resize( QDesktopWidget().availableGeometry( this ).size() * 0.8 );
    QSize pos = QDesktopWidget().availableGeometry( this ).size() * 0.1;
    move( pos.width(), pos.height() );
  }

}
///////////// END OF GUI SETUP ROUTINES ///////////////
void QgisApp::sponsors()
{
  QgsSettings settings;
  QString qgisSponsorsUrl = settings.value( QStringLiteral( "qgis/qgisSponsorsUrl" ),
                            tr( "http://qgis.org/en/site/about/sponsorship.html" ) ).toString();
  openURL( qgisSponsorsUrl, false );
}

void QgisApp::about()
{
  static QgsAbout *sAbt = nullptr;
  if ( !sAbt )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    sAbt = new QgsAbout( this );
    QString versionString = QStringLiteral( "<html><body><div align='center'><table width='100%'>" );

    versionString += QLatin1String( "<tr>" );
    versionString += "<td>" + tr( "QGIS version" )       + "</td><td>" + Qgis::QGIS_VERSION + "</td><td>";


    if ( QString( Qgis::QGIS_DEV_VERSION ) == QLatin1String( "exported" ) )
    {
      versionString += tr( "QGIS code branch" ) + QStringLiteral( "</td><td><a href=\"https://github.com/qgis/QGIS/tree/release-%1_%2\">Release %1.%2</a></td>" )
                       .arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 );
    }
    else
    {
      versionString += tr( "QGIS code revision" ) + QStringLiteral( "</td><td><a href=\"https://github.com/qgis/QGIS/commit/%1\">%1</a></td>" ).arg( Qgis::QGIS_DEV_VERSION );
    }

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "Compiled against Qt" ) + "</td><td>" + QT_VERSION_STR + "</td>";
    versionString += "<td>" + tr( "Running against Qt" )  + "</td><td>" + qVersion() + "</td>";

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "Compiled against GDAL/OGR" ) + "</td><td>" + GDAL_RELEASE_NAME + "</td>";
    versionString += "<td>" + tr( "Running against GDAL/OGR" )  + "</td><td>" + GDALVersionInfo( "RELEASE_NAME" ) + "</td>";

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "Compiled against GEOS" ) + "</td><td>" + GEOS_CAPI_VERSION + "</td>";
    versionString += "<td>" + tr( "Running against GEOS" ) + "</td><td>" + GEOSversion() + "</td>";

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "PostgreSQL Client Version" ) + "</td><td>";
#ifdef HAVE_POSTGRESQL
    versionString += PG_VERSION;
#else
    versionString += tr( "No support" );
#endif
    versionString += QLatin1String( "</td>" );

    versionString += "<td>" +  tr( "SpatiaLite Version" ) + "</td><td>";
    versionString += spatialite_version();
    versionString += QLatin1String( "</td>" );

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "QWT Version" ) + "</td><td>" + QWT_VERSION_STR + "</td>";
    versionString += "<td>" + tr( "PROJ.4 Version" ) + "</td><td>" + QString::number( PJ_VERSION ) + "</td>";

    versionString += QLatin1String( "</tr><tr>" );

    versionString += "<td>" + tr( "QScintilla2 Version" ) + "</td><td>" + QSCINTILLA_VERSION_STR + "</td>";

#ifdef QGISDEBUG
    versionString += "<td colspan=2>" + tr( "This copy of QGIS writes debugging output." ) + "</td>";
#endif

    versionString += QLatin1String( "</tr></table></div></body></html>" );

    sAbt->setVersion( versionString );

    QApplication::restoreOverrideCursor();
  }
  sAbt->show();
  sAbt->raise();
  sAbt->activateWindow();
}

void QgisApp::addLayerDefinition()
{
  QString path = QFileDialog::getOpenFileName( this, QStringLiteral( "Add Layer Definition File" ), QDir::home().path(), QStringLiteral( "*.qlr" ) );
  if ( path.isEmpty() )
    return;

  openLayerDefinition( path );
}

QString QgisApp::crsAndFormatAdjustedLayerUri( const QString &uri, const QStringList &supportedCrs, const QStringList &supportedFormats ) const
{
  QString newuri = uri;

  // Adjust layer CRS to project CRS
  QgsCoordinateReferenceSystem testCrs;
  Q_FOREACH ( const QString &c, supportedCrs )
  {
    testCrs.createFromOgcWmsCrs( c );
    if ( testCrs == mMapCanvas->mapSettings().destinationCrs() )
    {
      newuri.replace( QRegExp( "crs=[^&]+" ), "crs=" + c );
      QgsDebugMsg( QString( "Changing layer crs to %1, new uri: %2" ).arg( c, uri ) );
      break;
    }
  }

  // Use the last used image format
  QString lastImageEncoding = QgsSettings().value( QStringLiteral( "/qgis/lastWmsImageEncoding" ), "image/png" ).toString();
  Q_FOREACH ( const QString &fmt, supportedFormats )
  {
    if ( fmt == lastImageEncoding )
    {
      newuri.replace( QRegExp( "format=[^&]+" ), "format=" + fmt );
      QgsDebugMsg( QString( "Changing layer format to %1, new uri: %2" ).arg( fmt, uri ) );
      break;
    }
  }
  return newuri;
}

bool QgisApp::addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType )
{
  bool wasfrozen = mMapCanvas->isFrozen();
  QList<QgsMapLayer *> myList;
  Q_FOREACH ( QString src, layerQStringList )
  {
    src = src.trimmed();
    QString base;
    if ( dataSourceType == QLatin1String( "file" ) )
    {
      QString srcWithoutLayername( src );
      int posPipe = srcWithoutLayername.indexOf( '|' );
      if ( posPipe >= 0 )
        srcWithoutLayername.resize( posPipe );
      QFileInfo fi( srcWithoutLayername );
      base = fi.completeBaseName();

      // if needed prompt for zipitem layers
      QString vsiPrefix = QgsZipItem::vsiPrefix( src );
      if ( ! src.startsWith( QLatin1String( "/vsi" ), Qt::CaseInsensitive ) &&
           ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) ) )
      {
        if ( askUserForZipItemLayers( src ) )
          continue;
      }
    }
    else if ( dataSourceType == QLatin1String( "database" ) )
    {
      base = src;
    }
    else //directory //protocol
    {
      QFileInfo fi( src );
      base = fi.completeBaseName();
    }
    base = QgsMapLayer::formatLayerName( base );

    QgsDebugMsg( "completeBaseName: " + base );

    // create the layer

    QgsVectorLayer::LayerOptions options;
    options.loadDefaultStyle = false;
    QgsVectorLayer *layer = new QgsVectorLayer( src, base, QStringLiteral( "ogr" ), options );
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
      freezeCanvases( false );

      // Let render() do its own cursor management
      //      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false;
    }

    if ( layer->isValid() )
    {
      layer->setProviderEncoding( enc );

      QStringList sublayers = layer->dataProvider()->subLayers();
      QgsDebugMsg( QString( "got valid layer with %1 sublayers" ).arg( sublayers.count() ) );

      // If the newly created layer has more than 1 layer of data available, we show the
      // sublayers selection dialog so the user can select the sublayers to actually load.
      if ( sublayers.count() > 1 )
      {
        askUserForOGRSublayers( layer );

        // The first layer loaded is not useful in that case. The user can select it in
        // the list if he wants to load it.
        delete layer;

      }
      else if ( !sublayers.isEmpty() ) // there is 1 layer of data available
      {
        //set friendly name for datasources with only one layer
        QStringList elements = sublayers.at( 0 ).split( QgsDataProvider::SUBLAYER_SEPARATOR );
        QString subLayerNameFormatted = elements.size() >= 2 ? QgsMapLayer::formatLayerName( elements.at( 1 ) ) : QString();

        if ( elements.size() >= 4 && layer->name().compare( elements.at( 1 ), Qt::CaseInsensitive ) != 0
             && layer->name().compare( subLayerNameFormatted, Qt::CaseInsensitive ) != 0 )
        {
          layer->setName( QStringLiteral( "%1 %2" ).arg( layer->name(), elements.at( 1 ) ) );
        }

        myList << layer;
      }
      else
      {
        QString msg = tr( "%1 doesn't have any layers." ).arg( src );
        messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
        delete layer;
      }
    }
    else
    {
      QString msg = tr( "%1 is not a valid or recognized data source." ).arg( src );
      messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );

      // since the layer is bad, stomp on it
      delete layer;
    }

  }

  // make sure at least one layer was successfully added
  if ( myList.isEmpty() )
  {
    return false;
  }

  // Register this layer with the layers registry
  QgsProject::instance()->addMapLayers( myList );
  Q_FOREACH ( QgsMapLayer *l, myList )
  {
    bool ok;
    l->loadDefaultStyle( ok );
    l->loadDefaultMetadata( ok );
  }
  activateDeactivateLayerRelatedActions( activeLayer() );

  // Only update the map if we frozen in this method
  // Let the caller do it otherwise
  if ( !wasfrozen )
  {
    freezeCanvases( false );
    refreshMapCanvas();
  }
// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  // statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  return true;
} // QgisApp::addVectorLayer()

// present a dialog to choose zipitem layers
bool QgisApp::askUserForZipItemLayers( const QString &path )
{
  bool ok = false;
  QVector<QgsDataItem *> childItems;
  QgsZipItem *zipItem = nullptr;
  QgsSettings settings;
  int promptLayers = settings.value( QStringLiteral( "qgis/promptForRasterSublayers" ), 1 ).toInt();

  QgsDebugMsg( "askUserForZipItemLayers( " + path + ')' );

  // if scanZipBrowser == no: skip to the next file
  if ( settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString() == QLatin1String( "no" ) )
  {
    return false;
  }

  zipItem = new QgsZipItem( nullptr, path, path );
  if ( ! zipItem )
    return false;

  zipItem->populate();
  QgsDebugMsg( QString( "Path= %1 got zipitem with %2 children" ).arg( path ).arg( zipItem->rowCount() ) );

  // if 1 or 0 child found, exit so a normal item is created by gdal or ogr provider
  if ( zipItem->rowCount() <= 1 )
  {
    delete zipItem;
    return false;
  }

  // if promptLayers=Load all, load all layers without prompting
  if ( promptLayers == 3 )
  {
    childItems = zipItem->children();
  }
  // exit if promptLayers=Never
  else if ( promptLayers == 2 )
  {
    delete zipItem;
    return false;
  }
  else
  {
    // We initialize a selection dialog and display it.
    QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Vsifile, QStringLiteral( "vsi" ), this );
    QgsSublayersDialog::LayerDefinitionList layers;

    for ( int i = 0; i < zipItem->children().size(); i++ )
    {
      QgsDataItem *item = zipItem->children().at( i );
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( item );
      if ( !layerItem )
        continue;

      QgsDebugMsgLevel( QString( "item path=%1 provider=%2" ).arg( item->path(), layerItem->providerKey() ), 2 );

      QgsSublayersDialog::LayerDefinition def;
      def.layerId = i;
      def.layerName = item->name();
      if ( layerItem->providerKey() == QLatin1String( "gdal" ) )
      {
        def.type = tr( "Raster" );
      }
      else if ( layerItem->providerKey() == QLatin1String( "ogr" ) )
      {
        def.type = tr( "Vector" );
      }
      layers << def;
    }

    chooseSublayersDialog.populateLayerTable( layers );

    if ( chooseSublayersDialog.exec() )
    {
      Q_FOREACH ( const QgsSublayersDialog::LayerDefinition &def, chooseSublayersDialog.selection() )
      {
        childItems << zipItem->children().at( def.layerId );
      }
    }
  }

  if ( childItems.isEmpty() )
  {
    // return true so dialog doesn't popup again (#6225) - hopefully this doesn't create other trouble
    ok = true;
  }

  // add childItems
  Q_FOREACH ( QgsDataItem *item, childItems )
  {
    QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( item );
    if ( !layerItem )
      continue;

    QgsDebugMsg( QString( "item path=%1 provider=%2" ).arg( item->path(), layerItem->providerKey() ) );
    if ( layerItem->providerKey() == QLatin1String( "gdal" ) )
    {
      if ( addRasterLayer( item->path(), QFileInfo( item->name() ).completeBaseName() ) )
        ok = true;
    }
    else if ( layerItem->providerKey() == QLatin1String( "ogr" ) )
    {
      if ( addVectorLayers( QStringList( item->path() ), QStringLiteral( "System" ), QStringLiteral( "file" ) ) )
        ok = true;
    }
  }

  delete zipItem;
  return ok;
}

// present a dialog to choose GDAL raster sublayers
void QgisApp::askUserForGDALSublayers( QgsRasterLayer *layer )
{
  if ( !layer )
    return;

  QStringList sublayers = layer->subLayers();
  QgsDebugMsg( QString( "raster has %1 sublayers" ).arg( layer->subLayers().size() ) );

  if ( sublayers.empty() )
    return;

  // if promptLayers=Load all, load all sublayers without prompting
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/promptForRasterSublayers" ), 1 ).toInt() == 3 )
  {
    loadGDALSublayers( layer->source(), sublayers );
    return;
  }

  // We initialize a selection dialog and display it.
  QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Gdal, QStringLiteral( "gdal" ), this );
  chooseSublayersDialog.setShowAddToGroupCheckbox( true );

  QgsSublayersDialog::LayerDefinitionList layers;
  QStringList names;
  names.reserve( sublayers.size() );
  layers.reserve( sublayers.size() );
  for ( int i = 0; i < sublayers.size(); i++ )
  {
    // simplify raster sublayer name - should add a function in gdal provider for this?
    // code is copied from QgsGdalLayerItem::createChildren
    QString name = sublayers[i];
    QString path = layer->source();
    // if netcdf/hdf use all text after filename
    // for hdf4 it would be best to get description, because the subdataset_index is not very practical
    if ( name.startsWith( QLatin1String( "netcdf" ), Qt::CaseInsensitive ) ||
         name.startsWith( QLatin1String( "hdf" ), Qt::CaseInsensitive ) )
      name = name.mid( name.indexOf( path ) + path.length() + 1 );
    else
    {
      // remove driver name and file name
      name.remove( name.split( QgsDataProvider::SUBLAYER_SEPARATOR )[0] );
      name.remove( path );
    }
    // remove any : or " left over
    if ( name.startsWith( ':' ) )
      name.remove( 0, 1 );

    if ( name.startsWith( '\"' ) )
      name.remove( 0, 1 );

    if ( name.endsWith( ':' ) )
      name.chop( 1 );

    if ( name.endsWith( '\"' ) )
      name.chop( 1 );

    names << name;

    QgsSublayersDialog::LayerDefinition def;
    def.layerId = i;
    def.layerName = name;
    layers << def;
  }

  chooseSublayersDialog.populateLayerTable( layers );

  if ( chooseSublayersDialog.exec() )
  {
    // create more informative layer names, containing filename as well as sublayer name
    QRegExp rx( "\"(.*)\"" );
    QString uri, name;

    QgsLayerTreeGroup *group = nullptr;
    bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), true ).toBool();
    if ( addToGroup )
    {
      group = QgsProject::instance()->layerTreeRoot()->insertGroup( 0, layer->name() );
    }

    Q_FOREACH ( const QgsSublayersDialog::LayerDefinition &def, chooseSublayersDialog.selection() )
    {
      int i = def.layerId;
      if ( rx.indexIn( sublayers[i] ) != -1 )
      {
        uri = rx.cap( 1 );
        name = sublayers[i];
        name.replace( uri, QFileInfo( uri ).completeBaseName() );
      }
      else
      {
        name = names[i];
      }

      QgsRasterLayer *rlayer = new QgsRasterLayer( sublayers[i], name );
      if ( rlayer && rlayer->isValid() )
      {
        if ( addToGroup )
        {
          QgsProject::instance()->addMapLayer( rlayer, false );
          group->addLayer( rlayer );
        }
        else
        {
          addRasterLayer( rlayer );
        }
      }
    }
  }
}

// should the GDAL sublayers dialog should be presented to the user?
bool QgisApp::shouldAskUserForGDALSublayers( QgsRasterLayer *layer )
{
  // return false if layer is empty or raster has no sublayers
  if ( !layer || layer->providerType() != QLatin1String( "gdal" ) || layer->subLayers().empty() )
    return false;

  QgsSettings settings;
  int promptLayers = settings.value( QStringLiteral( "qgis/promptForRasterSublayers" ), 1 ).toInt();
  // 0 = Always -> always ask (if there are existing sublayers)
  // 1 = If needed -> ask if layer has no bands, but has sublayers
  // 2 = Never -> never prompt, will not load anything
  // 3 = Load all -> never prompt, but load all sublayers

  return promptLayers == 0 || promptLayers == 3 || ( promptLayers == 1 && layer->bandCount() == 0 );
}

// This method will load with GDAL the layers in parameter.
// It is normally triggered by the sublayer selection dialog.
void QgisApp::loadGDALSublayers( const QString &uri, const QStringList &list )
{
  QString path, name;
  QgsRasterLayer *subLayer = nullptr;
  QgsSettings settings;
  QgsLayerTreeGroup *group = nullptr;
  bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), true ).toBool();
  if ( addToGroup )
    group = QgsProject::instance()->layerTreeRoot()->insertGroup( 0, QFileInfo( uri ).completeBaseName() );

  //add layers in reverse order so they appear in the right order in the layer dock
  for ( int i = list.size() - 1; i >= 0 ; i-- )
  {
    path = list[i];
    // shorten name by replacing complete path with filename
    name = path;
    name.replace( uri, QFileInfo( uri ).completeBaseName() );
    subLayer = new QgsRasterLayer( path, name );
    if ( subLayer )
    {
      if ( subLayer->isValid() )
        if ( addToGroup )
        {
          QgsProject::instance()->addMapLayer( subLayer, false );
          group->addLayer( subLayer );
        }
        else
        {
          addRasterLayer( subLayer );
        }
      else
        delete subLayer;
    }

  }
}

// This method is the method that does the real job. If the layer given in
// parameter is nullptr, then the method tries to act on the activeLayer.
void QgisApp::askUserForOGRSublayers( QgsVectorLayer *layer )
{
  if ( !layer )
  {
    layer = qobject_cast<QgsVectorLayer *>( activeLayer() );
    if ( !layer || layer->dataProvider()->name() != QLatin1String( "ogr" ) )
      return;
  }

  QStringList sublayers = layer->dataProvider()->subLayers();

  QgsSublayersDialog::LayerDefinitionList list;
  QMap< QString, int > mapLayerNameToCount;
  bool uniqueNames = true;
  int lastLayerId = -1;
  Q_FOREACH ( const QString &sublayer, sublayers )
  {
    // OGR provider returns items in this format:
    // <layer_index>:<name>:<feature_count>:<geom_type>

    QStringList elements = sublayer.split( QgsDataProvider::SUBLAYER_SEPARATOR );
    // merge back parts of the name that may have been split
    while ( elements.size() > 5 )
    {
      elements[1] += ":" + elements[2];
      elements.removeAt( 2 );
    }

    if ( elements.count() >= 4 )
    {
      QgsSublayersDialog::LayerDefinition def;
      def.layerId = elements[0].toInt();
      def.layerName = elements[1];
      def.count = elements[2].toInt();
      def.type = elements[3];
      if ( lastLayerId != def.layerId )
      {
        int count = ++mapLayerNameToCount[def.layerName];
        if ( count > 1 || def.layerName.isEmpty() )
          uniqueNames = false;
        lastLayerId = def.layerId;
      }
      list << def;
    }
    else
    {
      QgsDebugMsg( "Unexpected output from OGR provider's subLayers()! " + sublayer );
    }
  }

  // Check if the current layer uri contains the

  // We initialize a selection dialog and display it.
  QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Ogr, QStringLiteral( "ogr" ), this );
  chooseSublayersDialog.setShowAddToGroupCheckbox( true );
  chooseSublayersDialog.populateLayerTable( list );

  if ( !chooseSublayersDialog.exec() )
    return;

  QString uri = layer->source();
  QString name = layer->name();
  //the separator char & was changed to | to be compatible
  //with url for protocol drivers
  if ( uri.contains( '|', Qt::CaseSensitive ) )
  {
    // If we get here, there are some options added to the filename.
    // A valid uri is of the form: filename&option1=value1&option2=value2,...
    // We want only the filename here, so we get the first part of the split.
    QStringList theURIParts = uri.split( '|' );
    uri = theURIParts.at( 0 );
  }

  // The uri must contain the actual uri of the vectorLayer from which we are
  // going to load the sublayers.
  QString fileName = QFileInfo( uri ).baseName();
  QList<QgsMapLayer *> myList;
  Q_FOREACH ( const QgsSublayersDialog::LayerDefinition &def, chooseSublayersDialog.selection() )
  {
    QString layerGeometryType = def.type;
    QString composedURI = uri;
    if ( uniqueNames )
    {
      composedURI += "|layername=" + def.layerName;
    }
    else
    {
      // Only use layerId if there are ambiguities with names
      composedURI += "|layerid=" + QString::number( def.layerId );
    }

    if ( !layerGeometryType.isEmpty() )
    {
      composedURI += "|geometrytype=" + layerGeometryType;
    }

    QgsDebugMsg( "Creating new vector layer using " + composedURI );
    QString name = fileName + " " + def.layerName;
    QgsVectorLayer::LayerOptions options;
    options.loadDefaultStyle = false;
    QgsVectorLayer *layer = new QgsVectorLayer( composedURI, name, QStringLiteral( "ogr" ), options );
    if ( layer && layer->isValid() )
    {
      myList << layer;
    }
    else
    {
      QString msg = tr( "%1 is not a valid or recognized data source" ).arg( composedURI );
      messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
      delete layer;
    }
  }

  if ( ! myList.isEmpty() )
  {
    QgsSettings settings;
    bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), true ).toBool();
    QgsLayerTreeGroup *group = nullptr;
    if ( addToGroup )
      group = QgsProject::instance()->layerTreeRoot()->insertGroup( 0, name );

    QgsProject::instance()->addMapLayers( myList, ! addToGroup );
    Q_FOREACH ( QgsMapLayer *l, myList )
    {
      bool ok;
      l->loadDefaultStyle( ok );
      l->loadDefaultMetadata( ok );
      if ( addToGroup )
        group->addLayer( l );
    }
  }
}

void QgisApp::addDatabaseLayer()
{
#ifdef HAVE_POSTGRESQL
  // Fudge for now
  QgsDebugMsg( "about to addRasterLayer" );

  // TODO: QDialog for now, switch to QWidget in future
  QDialog *dbs = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "postgres" ), this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "PostgreSQL" ), tr( "Cannot get PostgreSQL select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  connect( dbs, SIGNAL( progress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( dbs, SIGNAL( progressMessage( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );
  dbs->exec();
  delete dbs;
#endif
} // QgisApp::addDatabaseLayer()

void QgisApp::addDatabaseLayers( QStringList const &layerPathList, QString const &providerKey )
{
  QList<QgsMapLayer *> myList;

  if ( layerPathList.empty() )
  {
    // no layers to add so bail out, but
    // allow mMapCanvas to handle events
    // first
    freezeCanvases( false );
    return;
  }

  freezeCanvases( true );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  Q_FOREACH ( const QString &layerPath, layerPathList )
  {
    // create the layer
    QgsDataSourceUri uri( layerPath );

    QgsVectorLayer::LayerOptions options;
    options.loadDefaultStyle = false;
    QgsVectorLayer *layer = new QgsVectorLayer( uri.uri( false ), uri.table(), providerKey, options );
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
      freezeCanvases( false );
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return;
    }

    if ( layer->isValid() )
    {
      // add to list of layers to register
      //with the central layers registry
      myList << layer;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "%1 is an invalid layer - not loaded" ).arg( layerPath ) );
      QLabel *msgLabel = new QLabel( tr( "%1 is an invalid layer and cannot be loaded. Please check the <a href=\"#messageLog\">message log</a> for further info." ).arg( layerPath ), messageBar() );
      msgLabel->setWordWrap( true );
      connect( msgLabel, &QLabel::linkActivated, mLogDock, &QWidget::show );
      QgsMessageBarItem *item = new QgsMessageBarItem( msgLabel, QgsMessageBar::WARNING );
      messageBar()->pushItem( item );
      delete layer;
    }
    //qWarning("incrementing iterator");
  }

  QgsProject::instance()->addMapLayers( myList );

  // load default style after adding to process readCustomSymbology signals
  Q_FOREACH ( QgsMapLayer *l, myList )
  {
    bool ok;
    l->loadDefaultStyle( ok );
    l->loadDefaultMetadata( ok );
  }

  // draw the map
  freezeCanvases( false );
  refreshMapCanvas();

  QApplication::restoreOverrideCursor();
}


void QgisApp::addSpatiaLiteLayer()
{
  // show the SpatiaLite dialog
  QDialog *dbs = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "spatialite" ), this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "SpatiaLite" ), tr( "Cannot get SpatiaLite select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  dbs->exec();
  delete dbs;
} // QgisApp::addSpatiaLiteLayer()

void QgisApp::addDelimitedTextLayer()
{
  // show the Delimited text dialog
  QDialog *dts = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "delimitedtext" ), this ) );
  if ( !dts )
  {
    QMessageBox::warning( this, tr( "Delimited Text" ), tr( "Cannot get Delimited Text select dialog from provider." ) );
    return;
  }
  connect( dts, SIGNAL( addVectorLayer( QString, QString, QString ) ),
           this, SLOT( addSelectedVectorLayer( QString, QString, QString ) ) );
  dts->exec();
  delete dts;
} // QgisApp::addDelimitedTextLayer()

void QgisApp::addVirtualLayer()
{
  // show the Delimited text dialog
  QDialog *dts = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "virtual" ), this ) );
  if ( !dts )
  {
    QMessageBox::warning( this, tr( "Virtual layer" ), tr( "Cannot get virtual layer select dialog from provider." ) );
    return;
  }
  connect( dts, SIGNAL( addVectorLayer( QString, QString, QString ) ),
           this, SLOT( addSelectedVectorLayer( QString, QString, QString ) ) );
  connect( dts, SIGNAL( replaceVectorLayer( QString, QString, QString, QString ) ),
           this, SLOT( replaceSelectedVectorLayer( QString, QString, QString, QString ) ) );
  dts->exec();
  delete dts;
} // QgisApp::addVirtualLayer()

void QgisApp::addSelectedVectorLayer( const QString &uri, const QString &layerName, const QString &provider )
{
  addVectorLayer( uri, layerName, provider );
} // QgisApp:addSelectedVectorLayer

void QgisApp::replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider )
{
  QgsMapLayer *old = QgsProject::instance()->mapLayer( oldId );
  if ( !old )
    return;
  QgsVectorLayer *oldLayer = static_cast<QgsVectorLayer *>( old );
  QgsVectorLayer *newLayer = new QgsVectorLayer( uri, layerName, provider );
  if ( !newLayer || !newLayer->isValid() )
    return;

  QgsProject::instance()->addMapLayer( newLayer, /*addToLegend*/ false, /*takeOwnership*/ true );
  duplicateVectorStyle( oldLayer, newLayer );

  // insert the new layer just below the old one
  QgsLayerTreeUtils::insertLayerBelow( QgsProject::instance()->layerTreeRoot(), oldLayer, newLayer );
  // and remove the old layer
  QgsProject::instance()->removeMapLayer( oldLayer );
} // QgisApp:replaceSelectedVectorLayer

void QgisApp::addMssqlLayer()
{
  // show the MSSQL dialog
  QDialog *dbs = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "mssql" ), this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "MSSQL" ), tr( "Cannot get MSSQL select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  dbs->exec();
  delete dbs;
} // QgisApp::addMssqlLayer()

void QgisApp::addDb2Layer()
{
  // show the DB2 dialog
  QgsDebugMsg( "Show dialog for DB2 " );
  QDialog *dbs = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( QStringLiteral( "DB2" ), this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "DB2" ), tr( "Cannot get DB2 select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  dbs->exec();
  delete dbs;
} // QgisApp::addDb2Layer()

void QgisApp::addOracleLayer()
{
#ifdef HAVE_ORACLE
  // show the Oracle dialog
  QDialog *dbs = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( "oracle", this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "Oracle" ), tr( "Cannot get Oracle select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  connect( dbs, SIGNAL( progress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( dbs, SIGNAL( progressMessage( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );
  dbs->exec();
  delete dbs;
#endif
} // QgisApp::addOracleLayer()


void QgisApp::fileExit()
{
  if ( QgsApplication::taskManager()->countActiveTasks() > 0 )
  {
    QStringList tasks;
    Q_FOREACH ( QgsTask *task, QgsApplication::taskManager()->activeTasks() )
    {
      tasks << tr( " • %1" ).arg( task->description() );
    }

    // active tasks
    if ( QMessageBox::question( this, tr( "Active tasks" ),
                                tr( "The following tasks are currently running in the background:\n\n%1\n\nDo you want to try canceling these active tasks?" ).arg( tasks.join( QStringLiteral( "\n" ) ) ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
    {
      QgsApplication::taskManager()->cancelAll();
    }
    return;
  }

  if ( saveDirty() )
  {
    closeProject();
    userProfileManager()->setDefaultFromActive();
    qApp->exit( 0 );
  }
}


void QgisApp::fileNew()
{
  fileNew( true ); // prompts whether to save project
} // fileNew()


void QgisApp::fileNewBlank()
{
  fileNew( true, true );
}


//as file new but accepts flags to indicate whether we should prompt to save
void QgisApp::fileNew( bool promptToSaveFlag, bool forceBlank )
{
  if ( checkTasksDependOnProject() )
    return;

  if ( promptToSaveFlag )
  {
    if ( !saveDirty() )
    {
      return; //cancel pressed
    }
  }

  mProjectLastModified = QDateTime();

  QgsSettings settings;

  closeProject();

  QgsProject *prj = QgsProject::instance();
  prj->layerTreeRegistryBridge()->setNewLayersVisible( settings.value( QStringLiteral( "qgis/new_layers_visible" ), true ).toBool() );

  //set the color for selections
  //the default can be set in qgisoptions
  //use project properties to override the color on a per project basis
  int red = settings.value( QStringLiteral( "qgis/default_selection_color_red" ), 255 ).toInt();
  int green = settings.value( QStringLiteral( "qgis/default_selection_color_green" ), 255 ).toInt();
  int blue = settings.value( QStringLiteral( "qgis/default_selection_color_blue" ), 0 ).toInt();
  int alpha = settings.value( QStringLiteral( "qgis/default_selection_color_alpha" ), 255 ).toInt();
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), red );
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), green );
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), blue );
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), alpha );

  //set the canvas to the default background color
  //the default can be set in qgisoptions
  //use project properties to override the color on a per project basis
  red = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  green = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  blue = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), red );
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), green );
  prj->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), blue );

  mOverviewCanvas->setBackgroundColor( QColor( red, green, blue ) );
  applyProjectSettingsToCanvas( mMapCanvas );

  prj->setDirty( false );

  setTitleBarText_( *this );

  //QgsDebugMsg("emitting new project signal");

  // emit signal so listeners know we have a new project
  emit newProject();

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
  mMapCanvas->clearExtentHistory();
  mMapCanvas->setRotation( 0.0 );
  mScaleWidget->updateScales();

  // set project CRS
  QString defCrs = settings.value( QStringLiteral( "Projections/projectDefaultCrs" ), GEO_EPSG_CRS_AUTHID ).toString();
  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( defCrs );
  // write the projections _proj string_ to project settings
  prj->setCrs( srs );
  prj->setEllipsoid( srs.ellipsoidAcronym() );
  prj->setDirty( false );

  /* New Empty Project Created
      (before attempting to load custom project templates/filepaths) */

  // load default template
  /* NOTE: don't open default template on launch until after initialization,
           in case a project was defined via command line */

  // don't open template if last auto-opening of a project failed
  if ( ! forceBlank )
  {
    forceBlank = ! settings.value( QStringLiteral( "qgis/projOpenedOKAtLaunch" ), QVariant( true ) ).toBool();
  }

  if ( ! forceBlank && settings.value( QStringLiteral( "qgis/newProjectDefault" ), QVariant( false ) ).toBool() )
  {
    fileNewFromDefaultTemplate();
  }

  // set the initial map tool
  mMapCanvas->setMapTool( mMapTools.mPan );
  mNonEditMapTool = mMapTools.mPan;  // signals are not yet setup to catch this

}

bool QgisApp::fileNewFromTemplate( const QString &fileName )
{
  if ( checkTasksDependOnProject() )
    return false;

  if ( !saveDirty() )
  {
    return false; //cancel pressed
  }

  QgsDebugMsg( QString( "loading project template: %1" ).arg( fileName ) );
  if ( addProject( fileName ) )
  {
    // set null filename so we don't override the template
    QgsProject::instance()->setFileName( QString() );
    return true;
  }
  return false;
}

void QgisApp::fileNewFromDefaultTemplate()
{
  QString projectTemplate = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_default.qgs" );
  QString msgTxt;
  if ( !projectTemplate.isEmpty() && QFile::exists( projectTemplate ) )
  {
    if ( fileNewFromTemplate( projectTemplate ) )
    {
      return;
    }
    msgTxt = tr( "Default failed to open: %1" );
  }
  else
  {
    msgTxt = tr( "Default not found: %1" );
  }
  messageBar()->pushMessage( tr( "Open Template Project" ),
                             msgTxt.arg( projectTemplate ),
                             QgsMessageBar::WARNING );
}

void QgisApp::fileOpenAfterLaunch()
{
  // TODO: move auto-open project options to enums

  // check if a project is already loaded via command line or filesystem
  if ( !QgsProject::instance()->fileName().isNull() )
  {
    return;
  }

  // check if a data source is already loaded via command line or filesystem
  // empty project with layer loaded, but may not trigger a dirty project at this point
  if ( QgsProject::instance() && QgsProject::instance()->count() > 0 )
  {
    return;
  }

  // fileNewBlank() has already been called in QgisApp constructor
  // loaded project is either a new blank one, or one from command line/filesystem
  QgsSettings settings;
  QString autoOpenMsgTitle = tr( "Auto-open Project" );

  // get path of project file to open, or was attempted
  QString projPath;

  if ( mProjOpen == 0 ) // welcome page
  {
    connect( this, &QgisApp::newProject, this, &QgisApp::showMapCanvas );
    connect( this, &QgisApp::projectRead, this, &QgisApp::showMapCanvas );
    return;
  }
  if ( mProjOpen == 1 && !mRecentProjects.isEmpty() ) // most recent project
  {
    projPath = mRecentProjects.at( 0 ).path;
  }
  if ( mProjOpen == 2 ) // specific project
  {
    projPath = settings.value( QStringLiteral( "qgis/projOpenAtLaunchPath" ) ).toString();
  }

  // whether last auto-opening of a project failed
  bool projOpenedOK = settings.value( QStringLiteral( "qgis/projOpenedOKAtLaunch" ), QVariant( true ) ).toBool();

  // notify user if last attempt at auto-opening a project failed

  /* NOTE: Notification will not show if last auto-opened project failed but
      next project opened is from command line (minor issue) */

  /* TODO: Keep projOpenedOKAtLaunch from being reset to true after
      reading command line project (which happens before initialization signal) */
  if ( !projOpenedOK )
  {
    // only show the following 'auto-open project failed' message once, at launch
    settings.setValue( QStringLiteral( "qgis/projOpenedOKAtLaunch" ), QVariant( true ) );

    // set auto-open project back to 'New' to avoid re-opening bad project
    settings.setValue( QStringLiteral( "qgis/projOpenAtLaunch" ), QVariant( 0 ) );

    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "Failed to open: %1" ).arg( projPath ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  if ( mProjOpen == 3 ) // new project
  {
    // open default template, if defined
    if ( settings.value( QStringLiteral( "qgis/newProjectDefault" ), QVariant( false ) ).toBool() )
    {
      fileNewFromDefaultTemplate();
    }
    return;
  }

  if ( projPath.isEmpty() ) // projPath required from here
  {
    return;
  }

  if ( !projPath.endsWith( QLatin1String( ".qgs" ), Qt::CaseInsensitive ) )
  {
    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "Not valid project file: %1" ).arg( projPath ),
                               QgsMessageBar::WARNING );
    return;
  }

  if ( QFile::exists( projPath ) )
  {
    // set flag to check on next app launch if the following project opened OK
    settings.setValue( QStringLiteral( "qgis/projOpenedOKAtLaunch" ), QVariant( false ) );

    if ( !addProject( projPath ) )
    {
      messageBar()->pushMessage( autoOpenMsgTitle,
                                 tr( "Project failed to open: %1" ).arg( projPath ),
                                 QgsMessageBar::WARNING );
    }

    if ( projPath.endsWith( QLatin1String( "project_default.qgs" ) ) )
    {
      messageBar()->pushMessage( autoOpenMsgTitle,
                                 tr( "Default template has been reopened: %1" ).arg( projPath ),
                                 QgsMessageBar::INFO );
    }
  }
  else
  {
    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "File not found: %1" ).arg( projPath ),
                               QgsMessageBar::WARNING );
  }
}

void QgisApp::fileOpenedOKAfterLaunch()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/projOpenedOKAtLaunch" ), QVariant( true ) );
}

void QgisApp::fileNewFromTemplateAction( QAction *qAction )
{
  if ( ! qAction )
    return;

  if ( qAction->text() == tr( "< Blank >" ) )
  {
    fileNewBlank();
  }
  else
  {
    QgsSettings settings;
    QString templateDirName = settings.value( QStringLiteral( "qgis/projectTemplateDir" ),
                              QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
    fileNewFromTemplate( templateDirName + QDir::separator() + qAction->text() );
  }
}


void QgisApp::newVectorLayer()
{
  QString enc;
  QString fileName = QgsNewVectorLayerDialog::runAndCreateLayer( this, &enc, QgsProject::instance()->defaultCrsForNewLayers() );

  if ( !fileName.isEmpty() )
  {
    //then add the layer to the view
    QStringList fileNames;
    fileNames.append( fileName );
    //todo: the last parameter will change accordingly to layer type
    addVectorLayers( fileNames, enc, QStringLiteral( "file" ) );
  }
  else if ( fileName.isNull() )
  {
    QLabel *msgLabel = new QLabel( tr( "Layer creation failed. Please check the <a href=\"#messageLog\">message log</a> for further information." ), messageBar() );
    msgLabel->setWordWrap( true );
    connect( msgLabel, &QLabel::linkActivated, mLogDock, &QWidget::show );
    QgsMessageBarItem *item = new QgsMessageBarItem( msgLabel, QgsMessageBar::WARNING );
    messageBar()->pushItem( item );
  }
}

void QgisApp::newMemoryLayer()
{
  QgsVectorLayer *newLayer = QgsNewMemoryLayerDialog::runAndCreateLayer( this, QgsProject::instance()->defaultCrsForNewLayers() );

  if ( newLayer )
  {
    //then add the layer to the view
    QList< QgsMapLayer * > layers;
    layers << newLayer;

    QgsProject::instance()->addMapLayers( layers );
    newLayer->startEditing();
  }
}

void QgisApp::newSpatialiteLayer()
{
  QgsNewSpatialiteLayerDialog spatialiteDialog( this, QgsGuiUtils::ModalDialogFlags, QgsProject::instance()->defaultCrsForNewLayers() );
  spatialiteDialog.exec();
}

void QgisApp::newGeoPackageLayer()
{
  QgsNewGeoPackageLayerDialog dialog( this );
  dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
  dialog.exec();
}

void QgisApp::showRasterCalculator()
{
  QgsRasterCalcDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    //invoke analysis library
    QgsRasterCalculator rc( d.formulaString(), d.outputFile(), d.outputFormat(), d.outputRectangle(), d.outputCrs(), d.numberOfColumns(), d.numberOfRows(), d.rasterEntries() );

    QProgressDialog p( tr( "Calculating..." ), tr( "Abort..." ), 0, 0 );
    p.setWindowModality( Qt::WindowModal );
    p.setMaximum( 100.0 );
    QgsFeedback feedback;
    connect( &feedback, &QgsFeedback::progressChanged, &p, &QProgressDialog::setValue );
    connect( &feedback, &QgsFeedback::canceled, &p, &QProgressDialog::cancel );
    QgsRasterCalculator::Result res = static_cast< QgsRasterCalculator::Result >( rc.processCalculation( &feedback ) );
    switch ( res )
    {
      case QgsRasterCalculator::Success:
        if ( d.addLayerToProject() )
        {
          addRasterLayer( d.outputFile(), QFileInfo( d.outputFile() ).baseName() );
        }
        messageBar()->pushMessage( tr( "Raster calculator" ),
                                   tr( "Calculation complete." ),
                                   QgsMessageBar::INFO, messageTimeout() );
        break;

      case QgsRasterCalculator::CreateOutputError:
        messageBar()->pushMessage( tr( "Raster calculator" ),
                                   tr( "Could not create destination file." ),
                                   QgsMessageBar::CRITICAL );
        break;

      case QgsRasterCalculator::InputLayerError:
        messageBar()->pushMessage( tr( "Raster calculator" ),
                                   tr( "Could not read input layer." ),
                                   QgsMessageBar::CRITICAL );
        break;

      case QgsRasterCalculator::Canceled:
        break;

      case QgsRasterCalculator::ParserError:
        messageBar()->pushMessage( tr( "Raster calculator" ),
                                   tr( "Could not parse raster formula." ),
                                   QgsMessageBar::CRITICAL );
        break;

      case QgsRasterCalculator::MemoryError:
        messageBar()->pushMessage( tr( "Raster calculator" ),
                                   tr( "Insufficient memory available for operation." ),
                                   QgsMessageBar::CRITICAL );
        break;

    }
  }
}


void QgisApp::showAlignRasterTool()
{
  QgsAlignRasterDialog dlg( this );
  dlg.exec();
}


void QgisApp::fileOpen()
{
  if ( checkTasksDependOnProject() )
    return;

  // possibly save any pending work before opening a new project
  if ( saveDirty() )
  {
    // Retrieve last used project dir from persistent settings
    QgsSettings settings;
    QString lastUsedDir = settings.value( QStringLiteral( "UI/lastProjectDir" ), QDir::homePath() ).toString();
    QString fullPath = QFileDialog::getOpenFileName( this,
                       tr( "Choose a QGIS project file to open" ),
                       lastUsedDir,
                       tr( "QGIS files" ) + " (*.qgs *.qgz *.QGS)" );
    if ( fullPath.isNull() )
    {
      return;
    }

    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    QFileInfo myFI( fullPath );
    QString myPath = myFI.path();
    // Persist last used project dir
    settings.setValue( QStringLiteral( "UI/lastProjectDir" ), myPath );

    // open the selected project
    addProject( fullPath );
  }
} // QgisApp::fileOpen

void QgisApp::enableProjectMacros()
{
  mTrustedMacros = true;

  // load macros
  QgsPythonRunner::run( QStringLiteral( "qgis.utils.reloadProjectMacros()" ) );
}

/**
  adds a saved project to qgis, usually called on startup by specifying a
  project file on the command line
  */
bool QgisApp::addProject( const QString &projectFile )
{
  // close the previous opened project if any
  closeProject();

  QFileInfo pfi( projectFile );
  mStatusBar->showMessage( tr( "Loading project: %1" ).arg( pfi.fileName() ) );
  qApp->processEvents();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  bool autoSetupOnFirstLayer = mLayerTreeCanvasBridge->autoSetupOnFirstLayer();
  mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( false );

  if ( !QgsProject::instance()->read( projectFile ) && !QgsZipUtils::isZipFile( projectFile ) )
  {
    QString backupFile = projectFile + "~";
    QString loadBackupPrompt;
    QMessageBox::StandardButtons buttons;
    if ( QFile( backupFile ).exists() )
    {
      loadBackupPrompt = "\n\n" + tr( "Do you want to open the backup file\n%1\ninstead?" ).arg( backupFile );
      buttons |= QMessageBox::Yes;
      buttons |= QMessageBox::No;
    }
    else
    {
      buttons |= QMessageBox::Ok;
    }
    QApplication::restoreOverrideCursor();
    mStatusBar->clearMessage();

    int r = QMessageBox::critical( this,
                                   tr( "Unable to open project" ),
                                   QgsProject::instance()->error() + loadBackupPrompt,
                                   buttons );

    if ( QMessageBox::Yes == r && addProject( backupFile ) )
    {
      // We loaded data from the backup file, but we pretend to work on the original project file.
      QgsProject::instance()->setFileName( projectFile );
      QgsProject::instance()->setDirty( true );
      mProjectLastModified = pfi.lastModified();
      return true;
    }

    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
    return false;
  }

  mProjectLastModified = pfi.lastModified();

  setTitleBarText_( *this );
  int  myRedInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
  int  myGreenInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
  int  myBlueInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  mOverviewCanvas->setBackgroundColor( myColor );

  applyProjectSettingsToCanvas( mMapCanvas );

  //load project scales
  bool projectScales = QgsProject::instance()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) );
  if ( projectScales )
  {
    mScaleWidget->updateScales( QgsProject::instance()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) ) );
  }

  mMapCanvas->updateScale();
  QgsDebugMsg( "Scale restored..." );

  mActionFilterLegend->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "Legend" ), QStringLiteral( "filterByMap" ) ) );

  QgsSettings settings;

#ifdef WITH_BINDINGS
  // does the project have any macros?
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    if ( !QgsProject::instance()->readEntry( QStringLiteral( "Macros" ), QStringLiteral( "/pythonCode" ), QString() ).isEmpty() )
    {
      int enableMacros = settings.value( QStringLiteral( "qgis/enableMacros" ), 1 ).toInt();
      // 0 = never, 1 = ask, 2 = just for this session, 3 = always

      if ( enableMacros == 3 || enableMacros == 2 )
      {
        enableProjectMacros();
      }
      else if ( enableMacros == 1 ) // ask
      {
        // create the notification widget for macros


        QToolButton *btnEnableMacros = new QToolButton();
        btnEnableMacros->setText( tr( "Enable macros" ) );
        btnEnableMacros->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" ) );
        btnEnableMacros->setCursor( Qt::PointingHandCursor );
        btnEnableMacros->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

        QgsMessageBarItem *macroMsg = new QgsMessageBarItem(
          tr( "Security warning" ),
          tr( "project macros have been disabled." ),
          btnEnableMacros,
          QgsMessageBar::WARNING,
          0,
          mInfoBar );

        connect( btnEnableMacros, &QToolButton::clicked, this, [this, macroMsg]
        {
          enableProjectMacros();
          mInfoBar->popWidget( macroMsg );
        } );

        // display the macros notification widget
        mInfoBar->pushItem( macroMsg );
      }
    }
  }
#endif

  emit projectRead(); // let plug-ins know that we've read in a new
  // project so that they can check any project
  // specific plug-in state

  // add this to the list of recently used project files
  saveRecentProjectPath( projectFile, false );

  QApplication::restoreOverrideCursor();

  if ( autoSetupOnFirstLayer )
    mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( true );

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

  mStatusBar->showMessage( tr( "Project loaded" ), 3000 );
  return true;
} // QgisApp::addProject(QString projectFile)



bool QgisApp::fileSave()
{
  // if we don't have a file name, then obviously we need to get one; note
  // that the project file name is reset to null in fileNew()
  QFileInfo fullPath;

  if ( QgsProject::instance()->fileName().isNull() )
  {
    // Retrieve last used project dir from persistent settings
    QgsSettings settings;
    QString lastUsedDir = settings.value( QStringLiteral( "UI/lastProjectDir" ), QDir::homePath() ).toString();

    const QString qgsExt = tr( "QGIS files" ) + " (*.qgs)";
    const QString zipExt = tr( "QGZ files" ) + " (*.qgz)";
    QString filter;
    QString path = QFileDialog::getSaveFileName(
                     this,
                     tr( "Choose a QGIS project file" ),
                     lastUsedDir + '/' + QgsProject::instance()->title(),
                     qgsExt + ";;" + zipExt, &filter );
    if ( path.isEmpty() )
      return false;

    fullPath.setFile( path );

    // make sure we have the .qgs extension in the file name
    if ( filter == zipExt )
    {
      if ( fullPath.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) != 0 )
        fullPath.setFile( fullPath.filePath() + ".qgz" );
    }
    else
    {
      if ( fullPath.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) != 0 )
        fullPath.setFile( fullPath.filePath() + ".qgs" );
    }

    QgsProject::instance()->setFileName( fullPath.filePath() );
  }
  else
  {
    QFileInfo fi( QgsProject::instance()->fileName() );
    fullPath = fi.absoluteFilePath();
    if ( fi.exists() && !mProjectLastModified.isNull() && mProjectLastModified != fi.lastModified() )
    {
      if ( QMessageBox::warning( this,
                                 tr( "Project file was changed" ),
                                 tr( "The loaded project file on disk was meanwhile changed.  Do you want to overwrite the changes?\n"
                                     "\nLast modification date on load was: %1"
                                     "\nCurrent last modification date is: %2" )
                                 .arg( mProjectLastModified.toString( Qt::DefaultLocaleLongDate ),
                                       fi.lastModified().toString( Qt::DefaultLocaleLongDate ) ),
                                 QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
        return false;
    }

    if ( fi.exists() && ! fi.isWritable() )
    {
      messageBar()->pushMessage( tr( "Insufficient permissions" ),
                                 tr( "The project file is not writable." ),
                                 QgsMessageBar::WARNING );
      return false;
    }
  }

  if ( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    mStatusBar->showMessage( tr( "Saved project to: %1" ).arg( QDir::toNativeSeparators( QgsProject::instance()->fileName() ) ), 5000 );

    saveRecentProjectPath( fullPath.filePath() );

    QFileInfo fi( QgsProject::instance()->fileName() );
    mProjectLastModified = fi.lastModified();
  }
  else
  {
    QMessageBox::critical( this,
                           tr( "Unable to save project %1" ).arg( QDir::toNativeSeparators( QgsProject::instance()->fileName() ) ),
                           QgsProject::instance()->error() );
    return false;
  }

  // run the saved project macro
  if ( mTrustedMacros )
  {
    QgsPythonRunner::run( QStringLiteral( "qgis.utils.saveProjectMacro();" ) );
  }

  return true;
} // QgisApp::fileSave

void QgisApp::fileSaveAs()
{
  // Retrieve last used project dir from persistent settings
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastProjectDir" ), QDir::homePath() ).toString();

  const QString qgsExt = tr( "QGIS files" ) + " (*.qgs *.QGS)";
  const QString zipExt = tr( "QGZ files" ) + " (*.qgz)";
  QString filter;
  QString path = QFileDialog::getSaveFileName( this,
                 tr( "Choose a file name to save the QGIS project file as" ),
                 lastUsedDir + '/' + QgsProject::instance()->title(),
                 qgsExt + ";;" + zipExt, &filter );
  if ( path.isEmpty() )
    return;

  QFileInfo fullPath( path );

  settings.setValue( QStringLiteral( "UI/lastProjectDir" ), fullPath.path() );

  if ( filter == zipExt )
  {
    if ( fullPath.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) != 0 )
      fullPath.setFile( fullPath.filePath() + ".qgz" );
  }
  else // .qgs
  {
    if ( fullPath.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) != 0 )
      fullPath.setFile( fullPath.filePath() + ".qgs" );
  }

  QgsProject::instance()->setFileName( fullPath.filePath() );

  if ( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    mStatusBar->showMessage( tr( "Saved project to: %1" ).arg( QDir::toNativeSeparators( QgsProject::instance()->fileName() ) ), 5000 );
    // add this to the list of recently used project files
    saveRecentProjectPath( fullPath.filePath() );
    mProjectLastModified = fullPath.lastModified();
  }
  else
  {
    QMessageBox::critical( this,
                           tr( "Unable to save project %1" ).arg( QDir::toNativeSeparators( QgsProject::instance()->fileName() ) ),
                           QgsProject::instance()->error(),
                           QMessageBox::Ok,
                           Qt::NoButton );
  }
} // QgisApp::fileSaveAs

void QgisApp::dxfExport()
{
  QgsDxfExportDialog d;
  if ( d.exec() == QDialog::Accepted )
  {
    QgsDxfExport dxfExport;

    QgsMapSettings settings( mapCanvas()->mapSettings() );
    settings.setLayerStyleOverrides( QgsProject::instance()->mapThemeCollection()->mapThemeStyleOverrides( d.mapTheme() ) );
    dxfExport.setMapSettings( settings );
    dxfExport.addLayers( d.layers() );
    dxfExport.setSymbologyScale( d.symbologyScale() );
    dxfExport.setSymbologyExport( d.symbologyMode() );
    dxfExport.setLayerTitleAsName( d.layerTitleAsName() );
    dxfExport.setDestinationCrs( d.crs() );
    dxfExport.setForce2d( d.force2d() );

    QgsDxfExport::Flags flags = nullptr;
    if ( !d.useMText() )
      flags = flags | QgsDxfExport::FlagNoMText;
    dxfExport.setFlags( flags );

    if ( mapCanvas() )
    {
      //extent
      if ( d.exportMapExtent() )
      {
        QgsCoordinateTransform t( mapCanvas()->mapSettings().destinationCrs(), d.crs(), QgsProject::instance() );
        dxfExport.setExtent( t.transformBoundingBox( mapCanvas()->extent() ) );
      }
    }

    QString fileName( d.saveFile() );
    if ( !fileName.endsWith( QLatin1String( ".dxf" ), Qt::CaseInsensitive ) )
      fileName += QLatin1String( ".dxf" );
    QFile dxfFile( fileName );
    QApplication::setOverrideCursor( Qt::BusyCursor );
    if ( dxfExport.writeToFile( &dxfFile, d.encoding() ) == 0 )
    {
      messageBar()->pushMessage( tr( "DXF export completed" ), QgsMessageBar::INFO, 4 );
    }
    else
    {
      messageBar()->pushMessage( tr( "DXF export failed" ), QgsMessageBar::CRITICAL, 4 );
    }
    QApplication::restoreOverrideCursor();
  }
}

void QgisApp::dwgImport()
{
  QgsDwgImportDialog d;
  d.exec();
}

void QgisApp::openLayerDefinition( const QString &path )
{
  QString errorMessage;
  bool loaded = QgsLayerDefinition::loadLayerDefinition( path, QgsProject::instance(), QgsProject::instance()->layerTreeRoot(), errorMessage );
  if ( !loaded )
  {
    QgsDebugMsg( errorMessage );
    messageBar()->pushMessage( tr( "Error loading layer definition" ), errorMessage, QgsMessageBar::WARNING );
  }
}

void QgisApp::openTemplate( const QString &fileName )
{
  QFile templateFile;
  templateFile.setFileName( fileName );

  if ( !templateFile.open( QIODevice::ReadOnly ) )
  {
    messageBar()->pushMessage( tr( "Load template" ), tr( "Could not read template file" ), QgsMessageBar::WARNING );
    return;
  }

  QString title;
  if ( !uniqueLayoutTitle( this, title, true, QgsMasterLayoutInterface::PrintLayout ) )
  {
    return;
  }

  QgsLayoutDesignerDialog *designer = createNewPrintLayout( title );
  if ( !designer )
  {
    messageBar()->pushMessage( tr( "Load template" ), tr( "Could not create print layout" ), QgsMessageBar::WARNING );
    return;
  }

  bool loadedOk = false;
  QDomDocument templateDoc;
  if ( templateDoc.setContent( &templateFile, false ) )
  {
    designer->currentLayout()->loadFromTemplate( templateDoc, QgsReadWriteContext(), true, &loadedOk );
    designer->activate();
  }

  if ( !loadedOk )
  {
    designer->close();
    messageBar()->pushMessage( tr( "Load template" ), tr( "Could not load template file" ), QgsMessageBar::WARNING );
  }
}

// Open the project file corresponding to the
// path at the given index in mRecentProjectPaths
void QgisApp::openProject( QAction *action )
{
  // possibly save any pending work before opening a different project
  Q_ASSERT( action );

  if ( checkTasksDependOnProject() )
    return;

  QString debugme = action->data().toString();
  if ( saveDirty() )
    addProject( debugme );
}

void QgisApp::runScript( const QString &filePath )
{
#ifdef WITH_BINDINGS
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
    return;

  mPythonUtils->runString(
    QString( "import sys\n"
             "from qgis.utils import iface\n"
             "exec(open(\"%1\".replace(\"\\\\\", \"/\").encode(sys.getfilesystemencoding())).read())\n" ).arg( filePath )
    , tr( "Failed to run Python script:" ), false );
#else
  Q_UNUSED( filePath );
#endif
}


/**
  Open the specified project file; prompt to save previous project if necessary.
  Used to process a commandline argument or OpenDocument AppleEvent.
  */
void QgisApp::openProject( const QString &fileName )
{
  if ( checkTasksDependOnProject() )
    return;

  // possibly save any pending work before opening a different project
  if ( saveDirty() )
  {
    // error handling and reporting is in addProject() function
    addProject( fileName );
  }
}

/**
  Open a raster or vector file; ignore other files.
  Used to process a commandline argument or OpenDocument AppleEvent.
  @returns true if the file is successfully opened
  */
bool QgisApp::openLayer( const QString &fileName, bool allowInteractive )
{
  QFileInfo fileInfo( fileName );
  bool ok( false );

  CPLPushErrorHandler( CPLQuietErrorHandler );

  // if needed prompt for zipitem layers
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) )
  {
    if ( askUserForZipItemLayers( fileName ) )
    {
      CPLPopErrorHandler();
      return true;
    }
  }

  // try to load it as raster
  if ( QgsRasterLayer::isValidRasterFileName( fileName ) )
  {
    // open .adf as a directory
    if ( fileName.endsWith( QLatin1String( ".adf" ), Qt::CaseInsensitive ) )
    {
      QString dirName = fileInfo.path();
      ok  = addRasterLayer( dirName, QFileInfo( dirName ).completeBaseName() );
    }
    else
      ok  = addRasterLayer( fileName, fileInfo.completeBaseName() );
  }
  // TODO - should we really call isValidRasterFileName() before addRasterLayer()
  //        this results in 2 calls to GDALOpen()
  // I think (Radim) that it is better to test only first if valid,
  // addRasterLayer() is really trying to add layer and gives error if fails
  //
  // if ( addRasterLayer( fileName, fileInfo.completeBaseName() ) )
  // {
  //   ok  = true );
  // }
  else // nope - try to load it as a shape/ogr
  {
    if ( allowInteractive )
    {
      ok = addVectorLayers( QStringList( fileName ), QStringLiteral( "System" ), QStringLiteral( "file" ) );
    }
    else
    {
      ok = addVectorLayer( fileName, fileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
    }
  }

  CPLPopErrorHandler();

  if ( !ok )
  {
    // we have no idea what this file is...
    QgsMessageLog::logMessage( tr( "Unable to load %1" ).arg( fileName ) );
  }

  return ok;
}


// Open a file specified by a commandline argument, Drop or FileOpen event.
void QgisApp::openFile( const QString &fileName )
{
  // check to see if we are opening a project file
  QFileInfo fi( fileName );
  if ( fi.completeSuffix() == QLatin1String( "qgs" ) || fi.completeSuffix() == QLatin1String( "qgz" ) )
  {
    QgsDebugMsg( "Opening project " + fileName );
    openProject( fileName );
  }
  else if ( fi.completeSuffix() == QLatin1String( "qlr" ) )
  {
    openLayerDefinition( fileName );
  }
  else if ( fi.completeSuffix() == QLatin1String( "qpt" ) )
  {
    openTemplate( fileName );
  }
  else if ( fi.completeSuffix() == QLatin1String( "py" ) )
  {
    runScript( fileName );
  }
  else
  {
    QgsDebugMsg( "Adding " + fileName + " to the map canvas" );
    openLayer( fileName, true );
  }
}

void QgisApp::newPrintLayout()
{
  QString title;
  if ( !uniqueLayoutTitle( this, title, true, QgsMasterLayoutInterface::PrintLayout ) )
  {
    return;
  }
  createNewPrintLayout( title );
}

void QgisApp::newReport()
{
  QString title;
  if ( !uniqueLayoutTitle( this, title, true, QgsMasterLayoutInterface::Report ) )
  {
    return;
  }
  createNewReport( title );
}

void QgisApp::disablePreviewMode()
{
  mMapCanvas->setPreviewModeEnabled( false );
}

void QgisApp::activateGrayscalePreview()
{
  mMapCanvas->setPreviewModeEnabled( true );
  mMapCanvas->setPreviewMode( QgsPreviewEffect::PreviewGrayscale );
}

void QgisApp::activateMonoPreview()
{
  mMapCanvas->setPreviewModeEnabled( true );
  mMapCanvas->setPreviewMode( QgsPreviewEffect::PreviewMono );
}

void QgisApp::activateProtanopePreview()
{
  mMapCanvas->setPreviewModeEnabled( true );
  mMapCanvas->setPreviewMode( QgsPreviewEffect::PreviewProtanope );
}

void QgisApp::activateDeuteranopePreview()
{
  mMapCanvas->setPreviewModeEnabled( true );
  mMapCanvas->setPreviewMode( QgsPreviewEffect::PreviewDeuteranope );
}

void QgisApp::toggleFilterLegendByExpression( bool checked )
{
  QgsLayerTreeNode *node = mLayerTreeView->currentNode();
  if ( ! node )
    return;

  if ( QgsLayerTree::isLayer( node ) )
  {
    QString e = mLegendExpressionFilterButton->expressionText();
    QgsLayerTreeUtils::setLegendFilterByExpression( *QgsLayerTree::toLayer( node ), e, checked );
  }

  updateFilterLegend();
}

void QgisApp::updateFilterLegend()
{
  bool hasExpressions = mLegendExpressionFilterButton->isChecked() && QgsLayerTreeUtils::hasLegendFilterExpression( *mLayerTreeView->layerTreeModel()->rootGroup() );
  if ( mActionFilterLegend->isChecked() || hasExpressions )
  {
    layerTreeView()->layerTreeModel()->setLegendFilter( &mMapCanvas->mapSettings(),
        /* useExtent */ mActionFilterLegend->isChecked(),
        /* polygon */ QgsGeometry(),
        hasExpressions );
  }
  else
  {
    layerTreeView()->layerTreeModel()->setLegendFilterByMap( nullptr );
  }
}

void QgisApp::saveMapAsImage()
{
  QList< QgsDecorationItem * > decorations;
  Q_FOREACH ( QgsDecorationItem *decoration, mDecorationItems )
  {
    if ( decoration->enabled() )
    {
      decorations << decoration;
    }
  }

  QgsMapSaveDialog *dlg = new QgsMapSaveDialog( this, mMapCanvas, decorations, QgsProject::instance()->annotationManager()->annotations() );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
} // saveMapAsImage

void QgisApp::saveMapAsPdf()
{
  QList< QgsDecorationItem * > decorations;
  Q_FOREACH ( QgsDecorationItem *decoration, mDecorationItems )
  {
    if ( decoration->enabled() )
    {
      decorations << decoration;
    }
  }

  QgsMapSaveDialog *dlg = new QgsMapSaveDialog( this, mMapCanvas, decorations, QgsProject::instance()->annotationManager()->annotations(), QgsMapSaveDialog::Pdf );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
} // saveMapAsPdf

//overloaded version of the above function
void QgisApp::saveMapAsImage( const QString &imageFileNameQString, QPixmap *theQPixmap )
{
  if ( imageFileNameQString.isEmpty() )
  {
    //no fileName chosen
    return;
  }
  else
  {
    //force the size of the canvas
    mMapCanvas->resize( theQPixmap->width(), theQPixmap->height() );
    //save the mapview to the selected file
    mMapCanvas->saveAsImage( imageFileNameQString, theQPixmap );
  }
} // saveMapAsImage

//reimplements method from base (gui) class
void QgisApp::addAllToOverview()
{
  if ( mLayerTreeView )
  {
    Q_FOREACH ( QgsLayerTreeLayer *nodeL, mLayerTreeView->layerTreeModel()->rootGroup()->findLayers() )
      nodeL->setCustomProperty( QStringLiteral( "overview" ), 1 );
  }

  markDirty();
}

//reimplements method from base (gui) class
void QgisApp::removeAllFromOverview()
{
  if ( mLayerTreeView )
  {
    Q_FOREACH ( QgsLayerTreeLayer *nodeL, mLayerTreeView->layerTreeModel()->rootGroup()->findLayers() )
      nodeL->setCustomProperty( QStringLiteral( "overview" ), 0 );
  }

  markDirty();
}

void QgisApp::toggleFullScreen()
{
  if ( mFullScreenMode )
  {
    if ( mPrevScreenModeMaximized )
    {
      // Change to maximized state. Just calling showMaximized() results in
      // the window going to the normal state. Calling showNormal() then
      // showMaxmized() is a work-around. Turn off rendering for this as it
      // would otherwise cause two re-renders of the map, which can take a
      // long time.
      bool wasFrozen = mapCanvas()->isFrozen();
      freezeCanvases();
      showNormal();
      showMaximized();
      if ( !wasFrozen )
        freezeCanvases( false );
      mPrevScreenModeMaximized = false;
    }
    else
    {
      showNormal();
    }
    mFullScreenMode = false;
  }
  else
  {
    if ( isMaximized() )
    {
      mPrevScreenModeMaximized = true;
    }
    showFullScreen();
    mFullScreenMode = true;
  }
}

void QgisApp::togglePanelsVisibility()
{
  QgsSettings settings;

  QStringList docksTitle = settings.value( QStringLiteral( "UI/hiddenDocksTitle" ), QStringList() ).toStringList();
  QStringList docksActive = settings.value( QStringLiteral( "UI/hiddenDocksActive" ), QStringList() ).toStringList();

  QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  QList<QTabBar *> tabBars = findChildren<QTabBar *>();

  if ( docksTitle.isEmpty() )
  {

    Q_FOREACH ( QDockWidget *dock, docks )
    {
      if ( dock->isVisible() && !dock->isFloating() )
      {
        docksTitle << dock->windowTitle();
        dock->setVisible( false );
      }
    }

    Q_FOREACH ( QTabBar *tabBar, tabBars )
    {
      docksActive << tabBar->tabText( tabBar->currentIndex() );
    }

    settings.setValue( QStringLiteral( "UI/hiddenDocksTitle" ), docksTitle );
    settings.setValue( QStringLiteral( "UI/hiddenDocksActive" ), docksActive );
  }
  else
  {
    Q_FOREACH ( QDockWidget *dock, docks )
    {
      if ( docksTitle.contains( dock->windowTitle() ) )
      {
        dock->setVisible( true );
      }
    }

    Q_FOREACH ( QTabBar *tabBar, tabBars )
    {
      for ( int i = 0; i < tabBar->count(); ++i )
      {
        if ( docksActive.contains( tabBar->tabText( i ) ) )
        {
          tabBar->setCurrentIndex( i );
        }
      }
    }

    settings.setValue( QStringLiteral( "UI/hiddenDocksTitle" ), QStringList() );
    settings.setValue( QStringLiteral( "UI/hiddenDocksActive" ), QStringList() );
  }
}

void QgisApp::showActiveWindowMinimized()
{
  QWidget *window = QApplication::activeWindow();
  if ( window )
  {
    window->showMinimized();
  }
}

void QgisApp::toggleActiveWindowMaximized()
{
  QWidget *window = QApplication::activeWindow();
  if ( window )
  {
    if ( window->isMaximized() )
      window->showNormal();
    else
      window->showMaximized();
  }
}

void QgisApp::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

void QgisApp::bringAllToFront()
{
  QgsGui::nativePlatformInterface()->currentAppActivateIgnoringOtherApps();
}

void QgisApp::addWindow( QAction *action )
{
#ifdef Q_OS_MAC
  mWindowActions->addAction( action );
  mWindowMenu->addAction( action );
  action->setCheckable( true );
  action->setChecked( true );
#else
  Q_UNUSED( action );
#endif
}

void QgisApp::removeWindow( QAction *action )
{
#ifdef Q_OS_MAC
  mWindowActions->removeAction( action );
  mWindowMenu->removeAction( action );
#else
  Q_UNUSED( action );
#endif
}

void QgisApp::stopRendering()
{
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
    canvas->stopRendering();
}

//reimplements method from base (gui) class
void QgisApp::hideAllLayers()
{
  QgsDebugMsg( "hiding all layers!" );
  mLayerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive( false );

}


// reimplements method from base (gui) class
void QgisApp::showAllLayers()
{
  QgsDebugMsg( "Showing all layers!" );
  mLayerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive( true );
}

//reimplements method from base (gui) class
void QgisApp::hideSelectedLayers()
{
  QgsDebugMsg( "hiding selected layers!" );

  Q_FOREACH ( QgsLayerTreeNode *node, mLayerTreeView->selectedNodes() )
  {
    node->setItemVisibilityChecked( false );
  }
}

void QgisApp::hideDeselectedLayers()
{
  QList<QgsLayerTreeLayer *> selectedLayerNodes = mLayerTreeView->selectedLayerNodes();

  Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, mLayerTreeView->layerTreeModel()->rootGroup()->findLayers() )
  {
    if ( selectedLayerNodes.contains( nodeLayer ) )
      continue;
    nodeLayer->setItemVisibilityChecked( false );
  }
}

// reimplements method from base (gui) class
void QgisApp::showSelectedLayers()
{
  QgsDebugMsg( "show selected layers!" );

  Q_FOREACH ( QgsLayerTreeNode *node, mLayerTreeView->selectedNodes() )
  {
    QgsLayerTreeNode *nodeIter = node;
    while ( nodeIter )
    {
      nodeIter->setItemVisibilityChecked( true );
      nodeIter = nodeIter->parent();
    }
  }
}


void QgisApp::zoomIn()
{
  QgsDebugMsg( "Setting map tool to zoomIn" );

  mMapCanvas->setMapTool( mMapTools.mZoomIn );
}


void QgisApp::zoomOut()
{
  mMapCanvas->setMapTool( mMapTools.mZoomOut );
}

void QgisApp::zoomToSelected()
{
  mMapCanvas->zoomToSelected();
}

void QgisApp::panToSelected()
{
  mMapCanvas->panToSelected();
}

void QgisApp::pan()
{
  mMapCanvas->setMapTool( mMapTools.mPan );
}

void QgisApp::zoomFull()
{
  mMapCanvas->zoomToFullExtent();
}

void QgisApp::zoomToPrevious()
{
  mMapCanvas->zoomToPreviousExtent();
}

void QgisApp::zoomToNext()
{
  mMapCanvas->zoomToNextExtent();
}

void QgisApp::zoomActualSize()
{
  legendLayerZoomNative();
}

void QgisApp::identify()
{
  mMapCanvas->setMapTool( mMapTools.mIdentify );
}

void QgisApp::doFeatureAction()
{
  mMapCanvas->setMapTool( mMapTools.mFeatureAction );
}

void QgisApp::updateDefaultFeatureAction( QAction *action )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  mActionFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
  mActionFeatureAction->setToolTip( tr( "No action selected" ) );

  mFeatureActionMenu->setActiveAction( action );

  QgsAction qgsAction;
  if ( action )
  {
    qgsAction = action->data().value<QgsAction>();
  }

  if ( qgsAction.isValid() )
  {
    vlayer->actions()->setDefaultAction( QStringLiteral( "Canvas" ), qgsAction.id() );
    QgsGui::mapLayerActionRegistry()->setDefaultActionForLayer( vlayer, nullptr );

    mActionFeatureAction->setToolTip( tr( "Run feature action<br><b>%1</b>" ).arg( qgsAction.name() ) );

    if ( !qgsAction.icon().isNull() )
      mActionFeatureAction->setIcon( qgsAction.icon() );
  }
  else
  {
    //action is from QgsMapLayerActionRegistry
    vlayer->actions()->setDefaultAction( QStringLiteral( "Canvas" ), QString() );

    QgsMapLayerAction *mapLayerAction = qobject_cast<QgsMapLayerAction *>( action );
    if ( mapLayerAction )
    {
      QgsGui::mapLayerActionRegistry()->setDefaultActionForLayer( vlayer, mapLayerAction );

      if ( !mapLayerAction->text().isEmpty() )
        mActionFeatureAction->setToolTip( tr( "Run feature action<br><b>%1</b>" ).arg( mapLayerAction->text() ) );

      if ( !mapLayerAction->icon().isNull() )
        mActionFeatureAction->setIcon( mapLayerAction->icon() );
    }
    else
    {
      QgsGui::mapLayerActionRegistry()->setDefaultActionForLayer( vlayer, nullptr );
    }
  }
}

void QgisApp::refreshFeatureActions()
{
  mFeatureActionMenu->clear();

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  QList<QgsAction> actions = vlayer->actions()->actions( QStringLiteral( "Canvas" ) );
  Q_FOREACH ( const QgsAction &action, actions )
  {
    QAction *qAction = new QAction( action.icon(), action.shortTitle(), mFeatureActionMenu );
    qAction->setData( QVariant::fromValue<QgsAction>( action ) );
    mFeatureActionMenu->addAction( qAction );

    if ( action.name() == vlayer->actions()->defaultAction( QStringLiteral( "Canvas" ) ).name() )
    {
      mFeatureActionMenu->setActiveAction( qAction );
    }
  }

  //add actions registered in QgsMapLayerActionRegistry
  QList<QgsMapLayerAction *> registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer );
  if ( !actions.isEmpty() && !registeredActions.empty() )
  {
    //add a separator between user defined and standard actions
    mFeatureActionMenu->addSeparator();
  }

  for ( int i = 0; i < registeredActions.size(); i++ )
  {
    mFeatureActionMenu->addAction( registeredActions.at( i ) );
    if ( registeredActions.at( i ) == QgsGui::mapLayerActionRegistry()->defaultActionForLayer( vlayer ) )
    {
      mFeatureActionMenu->setActiveAction( registeredActions.at( i ) );
    }
  }

  updateDefaultFeatureAction( mFeatureActionMenu->activeAction() );
}

void QgisApp::measure()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureDist );
}

void QgisApp::measureArea()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureArea );
}

void QgisApp::measureAngle()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureAngle );
}

void QgisApp::addFormAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mFormAnnotation );
}

void QgisApp::addHtmlAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mHtmlAnnotation );
}

void QgisApp::addTextAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mTextAnnotation );
}

void QgisApp::addSvgAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mSvgAnnotation );
}

void QgisApp::modifyAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mAnnotation );
}

void QgisApp::reprojectAnnotations()
{
  Q_FOREACH ( QgsMapCanvasAnnotationItem *annotation, annotationItems() )
  {
    annotation->updatePosition();
  }
}

void QgisApp::labelingFontNotFound( QgsVectorLayer *vlayer, const QString &fontfamily )
{
  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  QString substitute = tr( "Default system font substituted." );

  QToolButton *btnOpenPrefs = new QToolButton();
  btnOpenPrefs->setStyleSheet( QStringLiteral( "QToolButton{ background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline; }" ) );
  btnOpenPrefs->setCursor( Qt::PointingHandCursor );
  btnOpenPrefs->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  btnOpenPrefs->setToolButtonStyle( Qt::ToolButtonTextOnly );

  // store pointer to vlayer in data of QAction
  QAction *act = new QAction( btnOpenPrefs );
  act->setData( QVariant( QMetaType::QObjectStar, &vlayer ) );
  act->setText( tr( "Open labeling dialog" ) );
  btnOpenPrefs->addAction( act );
  btnOpenPrefs->setDefaultAction( act );
  btnOpenPrefs->setToolTip( QLatin1String( "" ) );
  connect( btnOpenPrefs, &QToolButton::triggered, this, &QgisApp::labelingDialogFontNotFound );

  // no timeout set, since notice needs attention and is only shown first time layer is labeled
  QgsMessageBarItem *fontMsg = new QgsMessageBarItem(
    tr( "Labeling" ),
    tr( "Font for layer <b><u>%1</u></b> was not found (<i>%2</i>). %3" ).arg( vlayer->name(), fontfamily, substitute ),
    btnOpenPrefs,
    QgsMessageBar::WARNING,
    0,
    messageBar() );
  messageBar()->pushItem( fontMsg );
}

void QgisApp::commitError( QgsVectorLayer *vlayer )
{
  QgsMessageViewer *mv = new QgsMessageViewer();
  mv->setWindowTitle( tr( "Commit Errors" ) );
  mv->setMessageAsPlainText( tr( "Could not commit changes to layer %1" ).arg( vlayer->name() )
                             + "\n\n"
                             + tr( "Errors: %1\n" ).arg( vlayer->commitErrors().join( QStringLiteral( "\n  " ) ) )
                           );

  QToolButton *showMore = new QToolButton();
  // store pointer to vlayer in data of QAction
  QAction *act = new QAction( showMore );
  act->setData( QVariant( QMetaType::QObjectStar, &vlayer ) );
  act->setText( tr( "Show more" ) );
  showMore->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" ) );
  showMore->setCursor( Qt::PointingHandCursor );
  showMore->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  showMore->addAction( act );
  showMore->setDefaultAction( act );
  connect( showMore, &QToolButton::triggered, mv, &QDialog::exec );
  connect( showMore, &QToolButton::triggered, showMore, &QObject::deleteLater );

  // no timeout set, since notice needs attention and is only shown first time layer is labeled
  QgsMessageBarItem *errorMsg = new QgsMessageBarItem(
    tr( "Commit errors" ),
    tr( "Could not commit changes to layer %1" ).arg( vlayer->name() ),
    showMore,
    QgsMessageBar::WARNING,
    0,
    messageBar() );
  messageBar()->pushItem( errorMsg );
}

void QgisApp::labelingDialogFontNotFound( QAction *act )
{
  if ( !act )
  {
    return;
  }

  // get base pointer to layer
  QObject *obj = qvariant_cast<QObject *>( act->data() );

  // remove calling messagebar widget
  messageBar()->popWidget();

  if ( !obj )
  {
    return;
  }

  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( obj );
  if ( layer && setActiveLayer( layer ) )
  {
    labeling();
  }
}

void QgisApp::labeling()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
  {
    return;
  }

  mapStyleDock( true );
  mMapStyleWidget->setCurrentPage( QgsLayerStylingWidget::VectorLabeling );
}

void QgisApp::setMapStyleDockLayer( QgsMapLayer *layer )
{
  if ( !layer )
  {
    return;
  }

  mMapStyleWidget->setEnabled( true );
  // We don't set the layer if the dock isn't open mainly to save
  // the extra work if it's not needed
  if ( mMapStylingDock->isVisible() )
  {
    mMapStyleWidget->setLayer( layer );
  }
}

void QgisApp::mapStyleDock( bool enabled )
{
  mMapStylingDock->setUserVisible( enabled );
  setMapStyleDockLayer( activeLayer() );
}

void QgisApp::diagramProperties()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage( tr( "Diagram Properties" ),
                               tr( "Please select a vector layer first" ),
                               QgsMessageBar::INFO,
                               messageTimeout() );
    return;
  }

  QDialog dlg;
  dlg.setWindowTitle( tr( "Layer Diagram Properties" ) );
  QgsDiagramProperties *gui = new QgsDiagramProperties( vlayer, &dlg, mMapCanvas );
  gui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout *layout = new QVBoxLayout( &dlg );
  layout->addWidget( gui );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
    Qt::Horizontal, &dlg );
  layout->addWidget( buttonBox );

  dlg.setLayout( layout );

  connect( buttonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked,
           &dlg, &QDialog::accept );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked,
           &dlg, &QDialog::reject );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked,
           gui, &QgsDiagramProperties::apply );

  if ( dlg.exec() )
    gui->apply();

  activateDeactivateLayerRelatedActions( vlayer );
}

void QgisApp::setCadDockVisible( bool visible )
{
  mAdvancedDigitizingDockWidget->setVisible( visible );
}

void QgisApp::fieldCalculator()
{
  QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !myLayer )
  {
    return;
  }

  QgsFieldCalculator calc( myLayer, this );
  if ( calc.exec() )
  {
    myLayer->triggerRepaint();
  }
}

void QgisApp::attributeTable( QgsAttributeTableFilterModel::FilterMode filter )
{
  QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !myLayer )
  {
    return;
  }

  QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog( myLayer, filter );
  mDialog->show();
  // the dialog will be deleted by itself on close
}

void QgisApp::saveAsRasterFile( QgsRasterLayer *rasterLayer )
{
  if ( !rasterLayer )
    rasterLayer = qobject_cast<QgsRasterLayer *>( activeLayer() );

  if ( !rasterLayer )
  {
    return;
  }

  QgsRasterLayerSaveAsDialog d( rasterLayer, rasterLayer->dataProvider(),
                                mMapCanvas->extent(), rasterLayer->crs(),
                                mMapCanvas->mapSettings().destinationCrs(),
                                this );
  if ( d.exec() == QDialog::Rejected )
    return;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/lastRasterFileDir" ), QFileInfo( d.outputFileName() ).absolutePath() );

  QgsRasterFileWriter fileWriter( d.outputFileName() );
  if ( d.tileMode() )
  {
    fileWriter.setTiledMode( true );
    fileWriter.setMaxTileWidth( d.maximumTileSizeX() );
    fileWriter.setMaxTileHeight( d.maximumTileSizeY() );
  }
  else
  {
    fileWriter.setOutputFormat( d.outputFormat() );
  }

  // TODO: show error dialogs
  // TODO: this code should go somewhere else, but probably not into QgsRasterFileWriter
  // clone pipe/provider is not really necessary, ready for threads
  std::unique_ptr<QgsRasterPipe> pipe( nullptr );

  if ( d.mode() == QgsRasterLayerSaveAsDialog::RawDataMode )
  {
    QgsDebugMsg( "Writing raw data" );
    //QgsDebugMsg( QString( "Writing raw data" ).arg( pos ) );
    pipe.reset( new QgsRasterPipe() );
    if ( !pipe->set( rasterLayer->dataProvider()->clone() ) )
    {
      QgsDebugMsg( "Cannot set pipe provider" );
      return;
    }

    QgsRasterNuller *nuller = new QgsRasterNuller();
    for ( int band = 1; band <= rasterLayer->dataProvider()->bandCount(); band ++ )
    {
      nuller->setNoData( band, d.noData() );
    }
    if ( !pipe->insert( 1, nuller ) )
    {
      QgsDebugMsg( "Cannot set pipe nuller" );
      return;
    }

    // add projector if necessary
    if ( d.outputCrs() != rasterLayer->crs() )
    {
      QgsRasterProjector *projector = new QgsRasterProjector;
      projector->setCrs( rasterLayer->crs(), d.outputCrs() );
      if ( !pipe->insert( 2, projector ) )
      {
        QgsDebugMsg( "Cannot set pipe projector" );
        return;
      }
    }
  }
  else // RenderedImageMode
  {
    // clone the whole pipe
    QgsDebugMsg( "Writing rendered image" );
    pipe.reset( new QgsRasterPipe( *rasterLayer->pipe() ) );
    QgsRasterProjector *projector = pipe->projector();
    if ( !projector )
    {
      QgsDebugMsg( "Cannot get pipe projector" );
      return;
    }
    projector->setCrs( rasterLayer->crs(), d.outputCrs() );
  }

  if ( !pipe->last() )
  {
    return;
  }
  fileWriter.setCreateOptions( d.createOptions() );

  fileWriter.setBuildPyramidsFlag( d.buildPyramidsFlag() );
  fileWriter.setPyramidsList( d.pyramidsList() );
  fileWriter.setPyramidsResampling( d.pyramidsResamplingMethod() );
  fileWriter.setPyramidsFormat( d.pyramidsFormat() );
  fileWriter.setPyramidsConfigOptions( d.pyramidsConfigOptions() );

  bool tileMode = d.tileMode();
  bool addToCanvas = d.addToCanvas();
  QPointer< QgsRasterLayer > rlWeakPointer( rasterLayer );

  QgsRasterFileWriterTask *writerTask = new QgsRasterFileWriterTask( fileWriter, pipe.release(), d.nColumns(), d.nRows(),
      d.outputRectangle(), d.outputCrs() );

  // when writer is successful:

  connect( writerTask, &QgsRasterFileWriterTask::writeComplete, this, [this, tileMode, addToCanvas, rlWeakPointer ]( const QString & newFilename )
  {
    QString fileName = newFilename;
    if ( tileMode )
    {
      QFileInfo outputInfo( fileName );
      fileName = QStringLiteral( "%1/%2.vrt" ).arg( fileName, outputInfo.fileName() );
    }

    if ( addToCanvas )
    {
      addRasterLayers( QStringList( fileName ) );
    }
    if ( rlWeakPointer )
      emit layerSavedAs( rlWeakPointer, fileName );

    messageBar()->pushMessage( tr( "Saving done" ),
                               tr( "Export to raster file has been completed" ),
                               QgsMessageBar::INFO, messageTimeout() );
  } );

  // when an error occurs:
  connect( writerTask, &QgsRasterFileWriterTask::errorOccurred, this, [ = ]( int error )
  {
    if ( error != QgsRasterFileWriter::WriteCanceled )
    {
      QMessageBox::warning( this, tr( "Error" ),
                            tr( "Cannot write raster error code: %1" ).arg( error ),
                            QMessageBox::Ok );
    }
  } );

  QgsApplication::taskManager()->addTask( writerTask );
}


void QgisApp::saveAsFile( QgsMapLayer *layer )
{
  if ( !layer )
    layer = activeLayer();

  if ( !layer )
    return;

  QgsMapLayer::LayerType layerType = layer->type();
  if ( layerType == QgsMapLayer::RasterLayer )
  {
    saveAsRasterFile( qobject_cast<QgsRasterLayer *>( layer ) );
  }
  else if ( layerType == QgsMapLayer::VectorLayer )
  {
    saveAsVectorFileGeneral( qobject_cast<QgsVectorLayer *>( layer ) );
  }
}

void QgisApp::saveAsLayerDefinition()
{
  QString path = QFileDialog::getSaveFileName( this, QStringLiteral( "Save as Layer Definition File" ), QDir::home().path(), QStringLiteral( "*.qlr" ) );
  QgsDebugMsg( path );
  if ( path.isEmpty() )
    return;

  QString errorMessage;
  bool saved = QgsLayerDefinition::exportLayerDefinition( path, mLayerTreeView->selectedNodes(), errorMessage );
  if ( !saved )
  {
    messageBar()->pushMessage( tr( "Error saving layer definition file" ), errorMessage, QgsMessageBar::WARNING );
  }
}

///@cond PRIVATE

/**
 * Field value converter for export as vector layer
 * \note Not available in Python bindings
 */
class QgisAppFieldValueConverter : public QgsVectorFileWriter::FieldValueConverter
{
  public:
    QgisAppFieldValueConverter( QgsVectorLayer *vl, const QgsAttributeList &attributesAsDisplayedValues );

    QgsField fieldDefinition( const QgsField &field ) override;

    QVariant convert( int idx, const QVariant &value ) override;

    QgisAppFieldValueConverter *clone() const override;

  private:
    QPointer< QgsVectorLayer > mLayer;
    QgsAttributeList mAttributesAsDisplayedValues;
};

QgisAppFieldValueConverter::QgisAppFieldValueConverter( QgsVectorLayer *vl, const QgsAttributeList &attributesAsDisplayedValues )
  : mLayer( vl )
  , mAttributesAsDisplayedValues( attributesAsDisplayedValues )
{
}

QgsField QgisAppFieldValueConverter::fieldDefinition( const QgsField &field )
{
  if ( !mLayer )
    return field;

  int idx = mLayer->fields().indexFromName( field.name() );
  if ( mAttributesAsDisplayedValues.contains( idx ) )
  {
    return QgsField( field.name(), QVariant::String );
  }
  return field;
}

QVariant QgisAppFieldValueConverter::convert( int idx, const QVariant &value )
{
  if ( !mLayer || !mAttributesAsDisplayedValues.contains( idx ) )
  {
    return value;
  }
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mLayer->fields().field( idx ).name() );
  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
  return fieldFormatter->representValue( mLayer, idx, setup.config(), QVariant(), value );
}

QgisAppFieldValueConverter *QgisAppFieldValueConverter::clone() const
{
  return new QgisAppFieldValueConverter( *this );
}

///@endcond

void QgisApp::saveAsVectorFileGeneral( QgsVectorLayer *vlayer, bool symbologyOption )
{
  if ( !vlayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() ); // FIXME: output of multiple layers at once?
  }

  if ( !vlayer )
    return;

  QgsCoordinateReferenceSystem destCRS;

  int options = QgsVectorLayerSaveAsDialog::AllOptions;
  if ( !symbologyOption )
  {
    options &= ~QgsVectorLayerSaveAsDialog::Symbology;
  }

  QgsVectorLayerSaveAsDialog *dialog = new QgsVectorLayerSaveAsDialog( vlayer, options, this );

  dialog->setMapCanvas( mMapCanvas );
  dialog->setIncludeZ( QgsWkbTypes::hasZ( vlayer->wkbType() ) );

  if ( dialog->exec() == QDialog::Accepted )
  {
    QString encoding = dialog->encoding();
    QString vectorFilename = dialog->filename();
    QString format = dialog->format();
    QStringList datasourceOptions = dialog->datasourceOptions();
    bool autoGeometryType = dialog->automaticGeometryType();
    QgsWkbTypes::Type forcedGeometryType = dialog->geometryType();

    QgsCoordinateTransform ct;
    destCRS = QgsCoordinateReferenceSystem::fromSrsId( dialog->crs() );

    if ( destCRS.isValid() && destCRS != vlayer->crs() )
    {
      //ask user about datum transformation
      QgsSettings settings;
      QgsDatumTransformDialog dlg( vlayer->crs(), destCRS );
      if ( dlg.availableTransformationCount() > 1 &&
           settings.value( QStringLiteral( "Projections/showDatumTransformDialog" ), false ).toBool() )
      {
        dlg.exec();
      }
      ct = QgsCoordinateTransform( vlayer->crs(), destCRS, QgsProject::instance() );
    }

    QgsRectangle filterExtent = dialog->filterExtent();
    QgisAppFieldValueConverter converter( vlayer, dialog->attributesAsDisplayedValues() );
    QgisAppFieldValueConverter *converterPtr = nullptr;
    // No need to use the converter if there is nothing to convert
    if ( !dialog->attributesAsDisplayedValues().isEmpty() )
      converterPtr = &converter;

    QgsVectorFileWriter::SaveVectorOptions options;
    options.driverName = format;
    options.layerName = dialog->layername();
    options.actionOnExistingFile = dialog->creationActionOnExistingFile();
    options.fileEncoding = encoding;
    options.ct = ct;
    options.onlySelectedFeatures = dialog->onlySelected();
    options.datasourceOptions = datasourceOptions;
    options.layerOptions = dialog->layerOptions();
    options.skipAttributeCreation = dialog->selectedAttributes().isEmpty();
    options.symbologyExport = static_cast< QgsVectorFileWriter::SymbologyExport >( dialog->symbologyExport() );
    options.symbologyScale = dialog->scale();
    if ( dialog->hasFilterExtent() )
      options.filterExtent = filterExtent;
    options.overrideGeometryType = autoGeometryType ? QgsWkbTypes::Unknown : forcedGeometryType;
    options.forceMulti = dialog->forceMulti();
    options.includeZ = dialog->includeZ();
    options.attributes = dialog->selectedAttributes();
    options.fieldValueConverter = converterPtr;

    bool addToCanvas = dialog->addToCanvas();
    QString layerName = dialog->layername();
    QgsVectorFileWriterTask *writerTask = new QgsVectorFileWriterTask( vlayer, vectorFilename, options );

    // when writer is successful:
    connect( writerTask, &QgsVectorFileWriterTask::writeComplete, this, [this, addToCanvas, layerName, encoding, vectorFilename, vlayer]( const QString & newFilename )
    {
      if ( addToCanvas )
      {
        QString uri( newFilename );
        if ( !layerName.isEmpty() )
          uri += "|layername=" + layerName;
        this->addVectorLayers( QStringList( uri ), encoding, QStringLiteral( "file" ) );
      }
      this->emit layerSavedAs( vlayer, vectorFilename );
      this->messageBar()->pushMessage( tr( "Saving done" ),
                                       tr( "Export to vector file has been completed" ),
                                       QgsMessageBar::INFO, messageTimeout() );
    }
           );

    // when an error occurs:
    connect( writerTask, &QgsVectorFileWriterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
    {
      if ( error != QgsVectorFileWriter::Canceled )
      {
        QgsMessageViewer *m = new QgsMessageViewer( nullptr );
        m->setWindowTitle( tr( "Save Error" ) );
        m->setMessageAsPlainText( tr( "Export to vector file failed.\nError: %1" ).arg( errorMessage ) );
        m->exec();
      }
    }
           );

    QgsApplication::taskManager()->addTask( writerTask );
  }

  delete dialog;
}

void QgisApp::layerProperties()
{
  showLayerProperties( activeLayer() );
}

void QgisApp::deleteSelected( QgsMapLayer *layer, QWidget *parent, bool promptConfirmation )
{
  if ( !layer )
  {
    layer = mLayerTreeView->currentLayer();
  }

  if ( !parent )
  {
    parent = this;
  }

  if ( !layer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To delete features, you must select a vector layer in the legend" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    messageBar()->pushMessage( tr( "No Vector Layer Selected" ),
                               tr( "Deleting features only works on vector layers" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  if ( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    messageBar()->pushMessage( tr( "Provider does not support deletion" ),
                               tr( "Data provider does not support deleting features" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    messageBar()->pushMessage( tr( "Layer not editable" ),
                               tr( "The current layer is not editable. Choose 'Start editing' in the digitizing toolbar." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  //validate selection
  int numberOfSelectedFeatures = vlayer->selectedFeatureIds().size();
  if ( numberOfSelectedFeatures == 0 )
  {
    messageBar()->pushMessage( tr( "No Features Selected" ),
                               tr( "The current layer has no selected features" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }
  //display a warning
  if ( promptConfirmation && QMessageBox::warning( parent, tr( "Delete features" ), tr( "Delete %n feature(s)?", "number of features to delete", numberOfSelectedFeatures ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  vlayer->beginEditCommand( tr( "Features deleted" ) );
  int deletedCount = 0;
  if ( !vlayer->deleteSelectedFeatures( &deletedCount ) )
  {
    messageBar()->pushMessage( tr( "Problem deleting features" ),
                               tr( "A problem occurred during deletion of %1 feature(s)" ).arg( numberOfSelectedFeatures - deletedCount ),
                               QgsMessageBar::WARNING );
  }
  else
  {
    showStatusMessage( tr( "%n feature(s) deleted.", "number of features deleted", numberOfSelectedFeatures ) );
  }

  vlayer->endEditCommand();
}

void QgisApp::moveFeature()
{
  mMapCanvas->setMapTool( mMapTools.mMoveFeature );
}

void QgisApp::moveFeatureCopy()
{
  mMapCanvas->setMapTool( mMapTools.mMoveFeatureCopy );
}

void QgisApp::offsetCurve()
{
  mMapCanvas->setMapTool( mMapTools.mOffsetCurve );
}

void QgisApp::simplifyFeature()
{
  mMapCanvas->setMapTool( mMapTools.mSimplifyFeature );
}

void QgisApp::deleteRing()
{
  mMapCanvas->setMapTool( mMapTools.mDeleteRing );
}

void QgisApp::deletePart()
{
  mMapCanvas->setMapTool( mMapTools.mDeletePart );
}

QgsGeometry QgisApp::unionGeometries( const QgsVectorLayer *vl, QgsFeatureList &featureList, bool &canceled )
{
  canceled = false;
  if ( !vl || featureList.size() < 2 )
  {
    return QgsGeometry();
  }

  if ( !featureList.at( 0 ).hasGeometry() )
    return QgsGeometry();

  QgsGeometry unionGeom = featureList.at( 0 ).geometry();

  QProgressDialog progress( tr( "Merging features..." ), tr( "Abort" ), 0, featureList.size(), this );
  progress.setWindowModality( Qt::WindowModal );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int i = 1; i < featureList.size(); ++i )
  {
    if ( progress.wasCanceled() )
    {
      QApplication::restoreOverrideCursor();
      canceled = true;
      return QgsGeometry();
    }
    progress.setValue( i );
    QgsGeometry currentGeom = featureList.at( i ).geometry();
    if ( !currentGeom.isNull() )
    {
      unionGeom = unionGeom.combine( currentGeom );
      if ( unionGeom.isNull() )
      {
        QApplication::restoreOverrideCursor();
        return QgsGeometry();
      }
    }
  }

  //convert unionGeom to a multipart geometry in case it is necessary to match the layer type
  if ( QgsWkbTypes::isMultiType( vl->wkbType() ) && !unionGeom.isMultipart() )
  {
    unionGeom.convertToMultiType();
  }

  QApplication::restoreOverrideCursor();
  progress.setValue( featureList.size() );
  return unionGeom;
}

bool QgisApp::uniqueLayoutTitle( QWidget *parent, QString &title, bool acceptEmpty, QgsMasterLayoutInterface::Type type, const QString &currentTitle )
{
  if ( !parent )
  {
    parent = this;
  }
  bool ok = false;
  bool titleValid = false;
  QString newTitle = QString( currentTitle );

  QString typeString;
  switch ( type )
  {
    case QgsMasterLayoutInterface::PrintLayout:
      typeString = tr( "print layout" );
      break;
    case QgsMasterLayoutInterface::Report:
      typeString = tr( "report" );
      break;
  }

  QString chooseMsg = tr( "Enter a unique %1 title" ).arg( typeString );
  if ( acceptEmpty )
  {
    chooseMsg += '\n' + tr( "(a title will be automatically generated if left empty)" );
  }
  QString titleMsg = chooseMsg;

  QStringList layoutNames;
  layoutNames << newTitle;
  const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    layoutNames << l->name();
  }
  while ( !titleValid )
  {
    newTitle = QInputDialog::getText( parent,
                                      tr( "Create %1 title" ).arg( typeString ),
                                      titleMsg,
                                      QLineEdit::Normal,
                                      newTitle,
                                      &ok );
    if ( !ok )
    {
      return false;
    }

    if ( newTitle.isEmpty() )
    {
      if ( !acceptEmpty )
      {
        titleMsg = chooseMsg + "\n\n" + tr( "Title can not be empty!" );
      }
      else
      {
        titleValid = true;
        newTitle = QgsProject::instance()->layoutManager()->generateUniqueTitle( type );
      }
    }
    else if ( layoutNames.indexOf( newTitle, 1 ) >= 0 )
    {
      layoutNames[0] = QString(); // clear non-unique name
      titleMsg = chooseMsg + "\n\n" + tr( "Title already exists!" );
    }
    else
    {
      titleValid = true;
    }
  }

  title = newTitle;

  return true;
}

QgsLayoutDesignerDialog *QgisApp::createNewPrintLayout( const QString &t )
{
  QString title = t;
  if ( title.isEmpty() )
  {
    title = QgsProject::instance()->layoutManager()->generateUniqueTitle( QgsMasterLayoutInterface::PrintLayout );
  }
  //create new layout object
  QgsPrintLayout *layout = new QgsPrintLayout( QgsProject::instance() );
  layout->setName( title );
  layout->initializeDefaults();
  QgsProject::instance()->layoutManager()->addLayout( layout );
  return openLayoutDesignerDialog( layout );
}

QgsLayoutDesignerDialog *QgisApp::createNewReport( QString title )
{
  if ( title.isEmpty() )
  {
    title = QgsProject::instance()->layoutManager()->generateUniqueTitle( QgsMasterLayoutInterface::Report );
  }
  //create new report
  std::unique_ptr< QgsReport > report = qgis::make_unique< QgsReport >( QgsProject::instance() );
  report->setName( title );
  QgsMasterLayoutInterface *layout = report.get();
  QgsProject::instance()->layoutManager()->addLayout( report.release() );
  return openLayoutDesignerDialog( layout );
}

QgsLayoutDesignerDialog *QgisApp::openLayoutDesignerDialog( QgsMasterLayoutInterface *layout )
{
  // maybe a designer already open for this layout
  Q_FOREACH ( QgsLayoutDesignerDialog *designer, mLayoutDesignerDialogs )
  {
    if ( designer->masterLayout() == layout )
    {
      designer->show();
      designer->activate();
      designer->raise();
      return designer;
    }
  }

  //nope, so make a new one
  QgsLayoutDesignerDialog *newDesigner = new QgsLayoutDesignerDialog( this );
  newDesigner->setMasterLayout( layout );
  connect( newDesigner, &QgsLayoutDesignerDialog::aboutToClose, this, [this, newDesigner]
  {
    emit layoutDesignerWillBeClosed( newDesigner->iface() );
    mLayoutDesignerDialogs.remove( newDesigner );
    emit layoutDesignerClosed();
  } );

  //add it to the map of existing print designers
  mLayoutDesignerDialogs.insert( newDesigner );

  newDesigner->open();
  emit layoutDesignerOpened( newDesigner->iface() );

  return newDesigner;
}

QgsLayoutDesignerDialog *QgisApp::duplicateLayout( QgsMasterLayoutInterface *layout, const QString &t )
{
  QString title = t;
  if ( title.isEmpty() )
  {
    // TODO: inject a bit of randomness in auto-titles?
    title = tr( "%1 copy" ).arg( layout->name() );
  }

  QgsMasterLayoutInterface *newLayout = QgsProject::instance()->layoutManager()->duplicateLayout( layout, title );
  QgsLayoutDesignerDialog *dlg = openLayoutDesignerDialog( newLayout );
  dlg->activate();
  return dlg;
}

void QgisApp::deleteLayoutDesigners()
{
  // need a copy, since mLayoutDesignerDialogs will be modified as we iterate
  const QSet<QgsLayoutDesignerDialog *> dialogs = mLayoutDesignerDialogs;
  for ( QgsLayoutDesignerDialog *dlg : dialogs )
  {
    dlg->close(); // will trigger delete
  }
}

void QgisApp::setupLayoutManagerConnections()
{
  QgsLayoutManager *manager = QgsProject::instance()->layoutManager();
  connect( manager, &QgsLayoutManager::layoutAdded, this, [ = ]( const QString & name )
  {
    QgsMasterLayoutInterface *l = QgsProject::instance()->layoutManager()->layoutByName( name );
    if ( !l )
      return;
    QgsPrintLayout *pl = dynamic_cast< QgsPrintLayout *>( l );
    if ( !pl )
      return;

    mAtlasFeatureActions.insert( pl, nullptr );
    connect( pl, &QgsPrintLayout::nameChanged, this, [this, pl]( const QString & name )
    {
      QgsMapLayerAction *action = mAtlasFeatureActions.value( pl );
      if ( action )
      {
        action->setText( QString( tr( "Set as atlas feature for %1" ) ).arg( name ) );
      }
    } );

    connect( pl->atlas(), &QgsLayoutAtlas::coverageLayerChanged, this, [this, pl]( QgsVectorLayer * coverageLayer )
    {
      setupAtlasMapLayerAction( pl, static_cast< bool >( coverageLayer ) );
    } );

    connect( pl->atlas(), &QgsLayoutAtlas::toggled, this, [this, pl]( bool enabled )
    {
      setupAtlasMapLayerAction( pl, enabled );
    } );

    setupAtlasMapLayerAction( pl, pl->atlas()->enabled() && pl->atlas()->coverageLayer() );
  } );

  connect( manager, &QgsLayoutManager::layoutAboutToBeRemoved, this, [ = ]( const QString & name )
  {
    QgsMasterLayoutInterface *l = QgsProject::instance()->layoutManager()->layoutByName( name );
    if ( l )
    {
      QgsPrintLayout *pl = dynamic_cast< QgsPrintLayout * >( l );
      if ( pl )
      {
        QgsMapLayerAction *action = mAtlasFeatureActions.value( pl );
        if ( action )
        {
          QgsGui::mapLayerActionRegistry()->removeMapLayerAction( action );
          delete action;
          mAtlasFeatureActions.remove( pl );
        }
      }
    }
  } );
}

void QgisApp::setupDuplicateFeaturesAction()
{
  QgsMapLayerAction *action = new QgsMapLayerAction( QString( tr( "Duplicate feature" ) ),
      this, QgsMapLayerAction::AllActions,
      QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateFeature.svg" ) ) );

  QgsGui::mapLayerActionRegistry()->addMapLayerAction( action );
  connect( action, &QgsMapLayerAction::triggeredForFeature, this, [this]( QgsMapLayer * layer, const QgsFeature & feat )
  {
    duplicateFeatures( layer, feat );
  }
         );

  action = new QgsMapLayerAction( QString( tr( "Duplicate feature and digitize" ) ),
                                  this, QgsMapLayerAction::AllActions,
                                  QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateFeatureDigitized.svg" ) ) );

  QgsGui::mapLayerActionRegistry()->addMapLayerAction( action );
  connect( action, &QgsMapLayerAction::triggeredForFeature, this, [this]( QgsMapLayer * layer, const QgsFeature & feat )
  {
    duplicateFeatureDigitized( layer, feat );
  }
         );
}

void QgisApp::setupAtlasMapLayerAction( QgsPrintLayout *layout, bool enableAction )
{
  QgsMapLayerAction *action = mAtlasFeatureActions.value( layout );
  if ( action )
  {
    QgsGui::mapLayerActionRegistry()->removeMapLayerAction( action );
    delete action;
    action = nullptr;
    mAtlasFeatureActions.remove( layout );
  }

  if ( enableAction )
  {
    action = new QgsMapLayerAction( QString( tr( "Set as atlas feature for %1" ) ).arg( layout->name() ),
                                    this, layout->atlas()->coverageLayer(), QgsMapLayerAction::SingleFeature,
                                    QgsApplication::getThemeIcon( QStringLiteral( "/mIconAtlas.svg" ) ) );
    mAtlasFeatureActions.insert( layout, action );
    QgsGui::mapLayerActionRegistry()->addMapLayerAction( action );
    connect( action, &QgsMapLayerAction::triggeredForFeature, this, [this, layout]( QgsMapLayer * layer, const QgsFeature & feat )
    {
      setLayoutAtlasFeature( layout, layer, feat );
    }
           );
  }
}

void QgisApp::setLayoutAtlasFeature( QgsPrintLayout *layout, QgsMapLayer *layer, const QgsFeature &feat )
{
  QgsLayoutDesignerDialog *designer = openLayoutDesignerDialog( layout );
  designer->setAtlasFeature( layer, feat );
}

void QgisApp::layoutsMenuAboutToShow()
{
  populateLayoutsMenu( mLayoutsMenu );
}

void QgisApp::populateLayoutsMenu( QMenu *menu )
{
  menu->clear();
  QList<QAction *> acts;
  const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
  for ( QgsMasterLayoutInterface *layout : layouts )
  {
    QAction *a = new QAction( layout->name(), menu );
    connect( a, &QAction::triggered, this, [this, layout]
    {
      openLayoutDesignerDialog( layout );
    } );
    acts << a;
  }
  if ( acts.size() > 1 )
  {
    // sort actions by text
    std::sort( acts.begin(), acts.end(), cmpByText_ );
  }
  menu->addActions( acts );
}

void QgisApp::showPinnedLabels( bool show )
{
  qobject_cast<QgsMapToolPinLabels *>( mMapTools.mPinLabels )->showPinnedLabels( show );
}

void QgisApp::pinLabels()
{
  mActionShowPinnedLabels->setChecked( true );
  mMapCanvas->setMapTool( mMapTools.mPinLabels );
}

void QgisApp::showHideLabels()
{
  mMapCanvas->setMapTool( mMapTools.mShowHideLabels );
}

void QgisApp::moveLabel()
{
  mMapCanvas->setMapTool( mMapTools.mMoveLabel );
}

void QgisApp::rotateFeature()
{
  mMapCanvas->setMapTool( mMapTools.mRotateFeature );
}

void QgisApp::rotateLabel()
{
  mMapCanvas->setMapTool( mMapTools.mRotateLabel );
}

void QgisApp::changeLabelProperties()
{
  mMapCanvas->setMapTool( mMapTools.mChangeLabelProperties );
}

QList<QgsMapCanvasAnnotationItem *> QgisApp::annotationItems()
{
  QList<QgsMapCanvasAnnotationItem *> itemList;

  if ( !mMapCanvas )
  {
    return itemList;
  }

  if ( mMapCanvas )
  {
    QList<QGraphicsItem *> graphicsItems = mMapCanvas->items();
    QList<QGraphicsItem *>::iterator gIt = graphicsItems.begin();
    for ( ; gIt != graphicsItems.end(); ++gIt )
    {
      QgsMapCanvasAnnotationItem *currentItem = dynamic_cast<QgsMapCanvasAnnotationItem *>( *gIt );
      if ( currentItem )
      {
        itemList.push_back( currentItem );
      }
    }
  }
  return itemList;
}

QList<QgsMapCanvas *> QgisApp::mapCanvases()
{
  return findChildren< QgsMapCanvas * >();
}

void QgisApp::removeAnnotationItems()
{
  if ( !mMapCanvas )
  {
    return;
  }
  QGraphicsScene *scene = mMapCanvas->scene();
  if ( !scene )
  {
    return;
  }
  QList<QgsMapCanvasAnnotationItem *> itemList = annotationItems();
  Q_FOREACH ( QgsMapCanvasAnnotationItem *item, itemList )
  {
    if ( item )
    {
      scene->removeItem( item );
      delete item;
    }
  }
}

void QgisApp::mergeAttributesOfSelectedFeatures()
{
  //get active layer (hopefully vector)
  QgsMapLayer *activeMapLayer = activeLayer();
  if ( !activeMapLayer )
  {
    messageBar()->pushMessage( tr( "No active layer" ),
                               tr( "No active layer found. Please select a layer in the layer list" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if ( !vl )
  {
    messageBar()->pushMessage(
      tr( "Layer not editable" ),
      tr( "The merge features tool only works on vector layers." ),
      QgsMessageBar::WARNING );
    return;
  }

  if ( !vl->isEditable() )
  {
    messageBar()->pushMessage(
      tr( "Layer not editable" ),
      tr( "Merging features can only be done for layers in editing mode." ),
      QgsMessageBar::WARNING );

    return;
  }

  //get selected feature ids (as a QSet<int> )
  const QgsFeatureIds &featureIdSet = vl->selectedFeatureIds();
  if ( featureIdSet.size() < 2 )
  {
    messageBar()->pushMessage(
      tr( "Not enough features selected" ),
      tr( "The merge tool requires at least two selected features." ),
      QgsMessageBar::WARNING );
    return;
  }

  //get initial selection (may be altered by attribute merge dialog later)
  QgsFeatureList featureList = vl->selectedFeatures();  //get QList<QgsFeature>

  //merge the attributes together
  QgsMergeAttributesDialog d( featureList, vl, mapCanvas() );
  //initialize dialog with all columns set to skip
  d.setAllToSkip();
  if ( d.exec() == QDialog::Rejected )
  {
    return;
  }

  vl->beginEditCommand( tr( "Merged feature attributes" ) );

  QgsAttributes merged = d.mergedAttributes();
  QSet<int> toSkip = d.skippedAttributeIndexes();

  bool firstFeature = true;
  Q_FOREACH ( QgsFeatureId fid, vl->selectedFeatureIds() )
  {
    for ( int i = 0; i < merged.count(); ++i )
    {
      if ( toSkip.contains( i ) )
        continue;

      QVariant val = merged.at( i );
      QgsField fld( vl->fields().at( i ) );
      bool isDefaultValue = vl->fields().fieldOrigin( i ) == QgsFields::OriginProvider &&
                            vl->dataProvider() &&
                            vl->dataProvider()->defaultValueClause( vl->fields().fieldOriginIndex( i ) ) == val;

      // convert to destination data type
      if ( !isDefaultValue && !fld.convertCompatible( val ) )
      {
        if ( firstFeature )
        {
          //only warn on first feature
          messageBar()->pushMessage(
            tr( "Invalid result" ),
            tr( "Could not store value '%1' in field of type %2" ).arg( merged.at( i ).toString(), fld.typeName() ),
            QgsMessageBar::WARNING );
        }
      }
      else
      {
        vl->changeAttributeValue( fid, i, val );
      }
    }
    firstFeature = false;
  }

  vl->endEditCommand();

  vl->triggerRepaint();
}

void QgisApp::modifyAttributesOfSelectedFeatures()
{
  QgsMapLayer *activeMapLayer = activeLayer();
  if ( !activeMapLayer )
  {
    messageBar()->pushMessage(
      tr( "No active layer" ),
      tr( "Please select a layer in the layer list" ),
      QgsMessageBar::WARNING );
    return;
  }

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if ( !vl )
  {
    messageBar()->pushMessage(
      tr( "Invalid layer" ),
      tr( "The merge features tool only works on vector layers." ),
      QgsMessageBar::WARNING );
    return;
  }
  if ( !vl->isEditable() )
  {
    messageBar()->pushMessage(
      tr( "Layer not editable" ),
      tr( "Modifying features can only be done for layers in editing mode." ),
      QgsMessageBar::WARNING );

    return;
  }

  //dummy feature
  QgsFeature f;
  QgsAttributeEditorContext context;
  context.setAllowCustomUi( false );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( vl, &f, false, this, true, context );
  dialog->setMode( QgsAttributeForm::MultiEditMode );
  dialog->exec();
}

void QgisApp::mergeSelectedFeatures()
{
  //get active layer (hopefully vector)
  QgsMapLayer *activeMapLayer = activeLayer();
  if ( !activeMapLayer )
  {
    messageBar()->pushMessage(
      tr( "No active layer" ),
      tr( "Please select a layer in the layer list" ),
      QgsMessageBar::WARNING );
    return;
  }
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if ( !vl )
  {
    messageBar()->pushMessage(
      tr( "Invalid layer" ),
      tr( "The merge features tool only works on vector layers." ),
      QgsMessageBar::WARNING );
    return;
  }
  if ( !vl->isEditable() )
  {
    messageBar()->pushMessage(
      tr( "Layer not editable" ),
      tr( "Merging features can only be done for layers in editing mode." ),
      QgsMessageBar::WARNING );

    return;
  }

  //get selected feature ids (as a QSet<int> )
  const QgsFeatureIds &featureIdSet = vl->selectedFeatureIds();
  if ( featureIdSet.size() < 2 )
  {
    messageBar()->pushMessage(
      tr( "Not enough features selected" ),
      tr( "The merge tool requires at least two selected features" ),
      QgsMessageBar::WARNING );
    return;
  }

  //get initial selection (may be altered by attribute merge dialog later)
  QgsFeatureIds featureIds = vl->selectedFeatureIds();
  QgsFeatureList featureList = vl->selectedFeatures();  //get QList<QgsFeature>
  bool canceled;
  QgsGeometry unionGeom = unionGeometries( vl, featureList, canceled );
  if ( unionGeom.isNull() )
  {
    if ( !canceled )
    {
      messageBar()->pushMessage(
        tr( "Merge failed" ),
        tr( "An error occurred during the merge operation." ),
        QgsMessageBar::CRITICAL );
    }
    return;
  }

  //merge the attributes together
  QgsMergeAttributesDialog d( featureList, vl, mapCanvas() );
  if ( d.exec() == QDialog::Rejected )
  {
    return;
  }

  QgsFeatureIds featureIdsAfter = vl->selectedFeatureIds();

  if ( featureIdsAfter.size() < 2 )
  {
    messageBar()->pushMessage(
      tr( "Not enough features selected" ),
      tr( "The merge tool requires at least two selected features" ),
      QgsMessageBar::WARNING );
    return;
  }

  //if the user changed the feature selection in the merge dialog, we need to repeat the union and check the type
  if ( featureIds.size() != featureIdsAfter.size() )
  {
    bool canceled;
    QgsFeatureList featureListAfter = vl->selectedFeatures();
    unionGeom = unionGeometries( vl, featureListAfter, canceled );
    if ( unionGeom.isNull() )
    {
      if ( !canceled )
      {
        messageBar()->pushMessage(
          tr( "Merge failed" ),
          tr( "An error occurred during the merge operation." ),
          QgsMessageBar::CRITICAL );
      }
      return;
    }
  }

  vl->beginEditCommand( tr( "Merged features" ) );

  //create new feature
  QgsFeature newFeature;
  newFeature.setGeometry( unionGeom );

  QgsAttributes attrs = d.mergedAttributes();
  for ( int i = 0; i < attrs.count(); ++i )
  {
    QVariant val = attrs.at( i );
    bool isDefaultValue = vl->fields().fieldOrigin( i ) == QgsFields::OriginProvider &&
                          vl->dataProvider() &&
                          vl->dataProvider()->defaultValueClause( vl->fields().fieldOriginIndex( i ) ) == val;

    // convert to destination data type
    if ( !isDefaultValue && !vl->fields().at( i ).convertCompatible( val ) )
    {
      messageBar()->pushMessage(
        tr( "Invalid result" ),
        tr( "Could not store value '%1' in field of type %2." ).arg( attrs.at( i ).toString(), vl->fields().at( i ).typeName() ),
        QgsMessageBar::WARNING );
    }
    attrs[i] = val;
  }
  newFeature.setAttributes( attrs );

  QgsFeatureIds::const_iterator feature_it = featureIdsAfter.constBegin();
  for ( ; feature_it != featureIdsAfter.constEnd(); ++feature_it )
  {
    vl->deleteFeature( *feature_it );
  }

  vl->addFeature( newFeature );

  vl->endEditCommand();

  vl->triggerRepaint();
}

void QgisApp::nodeTool()
{
  mMapCanvas->setMapTool( mMapTools.mNodeTool );
}

void QgisApp::rotatePointSymbols()
{
  mMapCanvas->setMapTool( mMapTools.mRotatePointSymbolsTool );
}

void QgisApp::offsetPointSymbol()
{
  mMapCanvas->setMapTool( mMapTools.mOffsetPointSymbolTool );
}

void QgisApp::snappingOptions()
{
  mSnappingDialogContainer->show();
}

void QgisApp::splitFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mSplitFeatures );
}

void QgisApp::splitParts()
{
  mMapCanvas->setMapTool( mMapTools.mSplitParts );
}

void QgisApp::reshapeFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mReshapeFeatures );
}

void QgisApp::addFeature()
{
  mMapCanvas->setMapTool( mMapTools.mAddFeature );
}

void QgisApp::setMapTool( QgsMapTool *tool, bool clean )
{
  mMapCanvas->setMapTool( tool, clean );
}

void QgisApp::selectFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mSelectFeatures );
}

void QgisApp::selectByPolygon()
{
  mMapCanvas->setMapTool( mMapTools.mSelectPolygon );
}

void QgisApp::selectByFreehand()
{
  mMapCanvas->setMapTool( mMapTools.mSelectFreehand );
}

void QgisApp::selectByRadius()
{
  mMapCanvas->setMapTool( mMapTools.mSelectRadius );
}

void QgisApp::deselectAll()
{
  // Turn off rendering to improve speed.
  bool wasFrozen = mMapCanvas->isFrozen();
  freezeCanvases();

  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( !vl )
      continue;

    vl->removeSelection();
  }

  // Turn on rendering (if it was on previously)
  if ( !wasFrozen )
    freezeCanvases( false );
  refreshMapCanvas();
}

void QgisApp::invertSelection()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage(
      tr( "No active vector layer" ),
      tr( "To invert selection, choose a vector layer in the legend" ),
      QgsMessageBar::INFO,
      messageTimeout() );
    return;
  }

  vlayer->invertSelection();
}

void QgisApp::selectAll()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage(
      tr( "No active vector layer" ),
      tr( "To select all, choose a vector layer in the legend." ),
      QgsMessageBar::INFO,
      messageTimeout() );
    return;
  }

  vlayer->selectAll();
}

void QgisApp::selectByExpression()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage(
      tr( "No active vector layer" ),
      tr( "To select features, choose a vector layer in the legend." ),
      QgsMessageBar::INFO,
      messageTimeout() );
    return;
  }

  QgsExpressionSelectionDialog *dlg = new QgsExpressionSelectionDialog( vlayer, QString(), this );
  dlg->setMessageBar( messageBar() );
  dlg->setMapCanvas( mapCanvas() );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgisApp::selectByForm()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage(
      tr( "No active vector layer" ),
      tr( "To select features, choose a vector layer in the legend." ),
      QgsMessageBar::INFO,
      messageTimeout() );
    return;
  }
  QgsDistanceArea myDa;

  myDa.setSourceCrs( vlayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  QgsAttributeEditorContext context;
  context.setDistanceArea( myDa );
  context.setVectorLayerTools( mVectorLayerTools );

  QgsSelectByFormDialog *dlg = new QgsSelectByFormDialog( vlayer, context, this );
  dlg->setMessageBar( messageBar() );
  dlg->setMapCanvas( mapCanvas() );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgisApp::addRing()
{
  mMapCanvas->setMapTool( mMapTools.mAddRing );
}

void QgisApp::fillRing()
{
  mMapCanvas->setMapTool( mMapTools.mFillRing );
}


void QgisApp::addPart()
{
  mMapCanvas->setMapTool( mMapTools.mAddPart );
}


void QgisApp::cutSelectionToClipboard( QgsMapLayer *layerContainingSelection )
{
  // Test for feature support in this layer
  QgsVectorLayer *selectionVectorLayer = qobject_cast<QgsVectorLayer *>( layerContainingSelection ? layerContainingSelection : activeLayer() );
  if ( !selectionVectorLayer )
    return;

  clipboard()->replaceWithCopyOf( selectionVectorLayer );

  selectionVectorLayer->beginEditCommand( tr( "Features cut" ) );
  selectionVectorLayer->deleteSelectedFeatures();
  selectionVectorLayer->endEditCommand();
}

void QgisApp::copySelectionToClipboard( QgsMapLayer *layerContainingSelection )
{
  QgsVectorLayer *selectionVectorLayer = qobject_cast<QgsVectorLayer *>( layerContainingSelection ? layerContainingSelection : activeLayer() );
  if ( !selectionVectorLayer )
    return;

  // Test for feature support in this layer
  clipboard()->replaceWithCopyOf( selectionVectorLayer );
}

void QgisApp::clipboardChanged()
{
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::pasteFromClipboard( QgsMapLayer *destinationLayer )
{
  QgsVectorLayer *pasteVectorLayer = qobject_cast<QgsVectorLayer *>( destinationLayer ? destinationLayer : activeLayer() );
  if ( !pasteVectorLayer )
    return;

  pasteVectorLayer->beginEditCommand( tr( "Features pasted" ) );
  QgsFeatureList features = clipboard()->transformedCopyOf( pasteVectorLayer->crs(), pasteVectorLayer->fields() );
  int nTotalFeatures = features.count();

  QHash<int, int> remap;
  QgsFields fields = clipboard()->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    int dst = pasteVectorLayer->fields().lookupField( fields.at( idx ).name() );
    if ( dst < 0 )
      continue;

    remap.insert( idx, dst );
  }

  QgsExpressionContext context = pasteVectorLayer->createExpressionContext();


  QgsFeatureList newFeatures;
  QgsFeatureList::const_iterator featureIt = features.constBegin();
  while ( featureIt != features.constEnd() )
  {
    QgsAttributes srcAttr = featureIt->attributes();
    QgsAttributeMap dstAttr;

    for ( int src = 0; src < srcAttr.count(); ++src )
    {
      int dst = remap.value( src, -1 );
      if ( dst < 0 )
        continue;

      dstAttr[ dst ] = srcAttr.at( src );
    }

    QgsGeometry geom = featureIt->geometry();
    if ( featureIt->hasGeometry() )
    {
      // convert geometry to match destination layer
      QgsWkbTypes::GeometryType destType = pasteVectorLayer->geometryType();
      bool destIsMulti = QgsWkbTypes::isMultiType( pasteVectorLayer->wkbType() );
      if ( pasteVectorLayer->dataProvider() &&
           !pasteVectorLayer->dataProvider()->doesStrictFeatureTypeCheck() )
      {
        // force destination to multi if provider doesn't do a feature strict check
        destIsMulti = true;
      }

      if ( destType != QgsWkbTypes::UnknownGeometry )
      {
        QgsGeometry newGeometry = geom.convertToType( destType, destIsMulti );
        if ( newGeometry.isNull() )
        {
          continue;
        }
        geom = newGeometry;
      }
      // avoid intersection if enabled in digitize settings
      geom.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );
    }

    // now create new feature using pasted feature as a template. This automatically handles default
    // values and field constraints
    newFeatures << QgsVectorLayerUtils::createFeature( pasteVectorLayer, geom, dstAttr, &context );
    ++featureIt;
  }

  pasteVectorLayer->addFeatures( newFeatures );
  QgsFeatureIds newIds;
  for ( const QgsFeature &f : qgis::as_const( newFeatures ) )
  {
    newIds << f.id();
  }


  pasteVectorLayer->selectByIds( newIds );
  pasteVectorLayer->endEditCommand();
  pasteVectorLayer->updateExtents();

  int nCopiedFeatures = features.count();
  if ( nCopiedFeatures == 0 )
  {
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "no features could be successfully pasted." ),
                               QgsMessageBar::WARNING, messageTimeout() );

  }
  else if ( nCopiedFeatures == nTotalFeatures )
  {
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "%1 features were successfully pasted." ).arg( nCopiedFeatures ),
                               QgsMessageBar::INFO, messageTimeout() );
  }
  else
  {
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "%1 of %2 features could be successfully pasted." ).arg( nCopiedFeatures ).arg( nTotalFeatures ),
                               QgsMessageBar::WARNING, messageTimeout() );
  }

  pasteVectorLayer->triggerRepaint();
}

void QgisApp::pasteAsNewVector()
{

  QgsVectorLayer *layer = pasteToNewMemoryVector();
  if ( !layer )
    return;

  saveAsVectorFileGeneral( layer, false );

  delete layer;
}

QgsVectorLayer *QgisApp::pasteAsNewMemoryVector( const QString &layerName )
{

  QString layerNameCopy = layerName;

  if ( layerNameCopy.isEmpty() )
  {
    bool ok;
    QString defaultName = tr( "Pasted" );
    layerNameCopy = QInputDialog::getText( this, tr( "New temporary scratch layer name" ),
                                           tr( "Layer name" ), QLineEdit::Normal,
                                           defaultName, &ok );
    if ( !ok )
      return nullptr;

    if ( layerNameCopy.isEmpty() )
    {
      layerNameCopy = defaultName;
    }
  }

  QgsVectorLayer *layer = pasteToNewMemoryVector();
  if ( !layer )
    return nullptr;

  layer->setName( layerNameCopy );

  freezeCanvases();

  QgsProject::instance()->addMapLayer( layer );

  freezeCanvases( false );
  refreshMapCanvas();

  return layer;
}

QgsVectorLayer *QgisApp::pasteToNewMemoryVector()
{
  QgsFields fields = clipboard()->fields();

  // Decide geometry type from features, switch to multi type if at least one multi is found
  QMap<QgsWkbTypes::Type, int> typeCounts;
  QgsFeatureList features = clipboard()->copyOf( fields );
  for ( int i = 0; i < features.size(); i++ )
  {
    QgsFeature &feature = features[i];
    if ( !feature.hasGeometry() )
      continue;

    QgsWkbTypes::Type type = feature.geometry().wkbType();

    if ( type == QgsWkbTypes::Unknown || type == QgsWkbTypes::NoGeometry )
      continue;

    if ( QgsWkbTypes::isSingleType( type ) )
    {
      if ( typeCounts.contains( QgsWkbTypes::multiType( type ) ) )
      {
        typeCounts[ QgsWkbTypes::multiType( type )] = typeCounts[ QgsWkbTypes::multiType( type )] + 1;
      }
      else
      {
        typeCounts[ type ] = typeCounts[ type ] + 1;
      }
    }
    else if ( QgsWkbTypes::isMultiType( type ) )
    {
      if ( typeCounts.contains( QgsWkbTypes::singleType( type ) ) )
      {
        // switch to multi type
        typeCounts[type] = typeCounts[ QgsWkbTypes::singleType( type )];
        typeCounts.remove( QgsWkbTypes::singleType( type ) );
      }
      typeCounts[type] = typeCounts[type] + 1;
    }
  }

  QgsWkbTypes::Type wkbType = !typeCounts.isEmpty() ? typeCounts.keys().value( 0 ) : QgsWkbTypes::NoGeometry;

  if ( features.isEmpty() )
  {
    // should not happen
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "No features in clipboard." ),
                               QgsMessageBar::WARNING, messageTimeout() );
    return nullptr;
  }
  else if ( typeCounts.size() > 1 )
  {
    QString typeName = wkbType != QgsWkbTypes::NoGeometry ? QgsWkbTypes::displayString( wkbType ) : QStringLiteral( "none" );
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "Multiple geometry types found, features with geometry different from %1 will be created without geometry." ).arg( typeName ),
                               QgsMessageBar::INFO, messageTimeout() );
  }

  QgsVectorLayer *layer = QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "pasted_features" ), QgsFields(), wkbType, clipboard()->crs() );

  if ( !layer->isValid() || !layer->dataProvider() )
  {
    delete layer;
    messageBar()->pushMessage( tr( "Paste features" ),
                               tr( "Cannot create new layer." ),
                               QgsMessageBar::WARNING, messageTimeout() );
    return nullptr;
  }

  layer->startEditing();
  Q_FOREACH ( QgsField f, clipboard()->fields().toList() )
  {
    QgsDebugMsg( QString( "field %1 (%2)" ).arg( f.name(), QVariant::typeToName( f.type() ) ) );
    if ( !layer->addAttribute( f ) )
    {
      messageBar()->pushMessage( tr( "Paste features" ),
                                 tr( "Cannot create field %1 (%2,%3)" ).arg( f.name(), f.typeName(), QVariant::typeToName( f.type() ) ),
                                 QgsMessageBar::WARNING, messageTimeout() );
      delete layer;
      return nullptr;
    }
  }

  // Convert to multi if necessary
  for ( int i = 0; i < features.size(); i++ )
  {
    QgsFeature &feature = features[i];
    if ( !feature.hasGeometry() )
      continue;

    QgsWkbTypes::Type type = feature.geometry().wkbType();
    if ( type == QgsWkbTypes::Unknown || type == QgsWkbTypes::NoGeometry )
      continue;

    if ( QgsWkbTypes::singleType( wkbType ) != QgsWkbTypes::singleType( type ) )
    {
      feature.clearGeometry();
    }

    if ( QgsWkbTypes::isMultiType( wkbType ) &&  QgsWkbTypes::isSingleType( type ) )
    {
      QgsGeometry g = feature.geometry();
      g.convertToMultiType();
      feature.setGeometry( g );
    }
  }
  if ( ! layer->addFeatures( features ) || !layer->commitChanges() )
  {
    QgsDebugMsg( "Cannot add features or commit changes" );
    delete layer;
    return nullptr;
  }

  QgsDebugMsg( QString( "%1 features pasted to temporary scratch layer" ).arg( layer->featureCount() ) );
  return layer;
}

void QgisApp::copyStyle( QgsMapLayer *sourceLayer )
{
  QgsMapLayer *selectionLayer = sourceLayer ? sourceLayer : activeLayer();

  if ( selectionLayer )
  {
    QString errorMsg;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    selectionLayer->exportNamedStyle( doc, errorMsg );


    if ( !errorMsg.isEmpty() )
    {
      messageBar()->pushMessage( tr( "Cannot copy style" ),
                                 errorMsg,
                                 QgsMessageBar::CRITICAL, messageTimeout() );
      return;
    }
    // Copies data in text form as well, so the XML can be pasted into a text editor
    clipboard()->setData( QGSCLIPBOARD_STYLE_MIME, doc.toByteArray(), doc.toString() );
    // Enables the paste menu element
    mActionPasteStyle->setEnabled( true );
  }
}

/**
   \param destinationLayer  The layer that the clipboard will be pasted to
                            (defaults to the active layer on the legend)
 */


void QgisApp::pasteStyle( QgsMapLayer *destinationLayer )
{
  QgsMapLayer *selectionLayer = destinationLayer ? destinationLayer : activeLayer();
  if ( selectionLayer )
  {
    if ( clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
    {
      QDomDocument doc( QStringLiteral( "qgis" ) );
      QString errorMsg;
      int errorLine, errorColumn;
      if ( !doc.setContent( clipboard()->data( QGSCLIPBOARD_STYLE_MIME ), false, &errorMsg, &errorLine, &errorColumn ) )
      {

        messageBar()->pushMessage( tr( "Cannot parse style" ),
                                   errorMsg,
                                   QgsMessageBar::CRITICAL, messageTimeout() );
        return;
      }

      bool isVectorStyle = doc.elementsByTagName( QStringLiteral( "pipe" ) ).isEmpty();
      if ( ( selectionLayer->type() == QgsMapLayer::RasterLayer && isVectorStyle ) ||
           ( selectionLayer->type() == QgsMapLayer::VectorLayer && !isVectorStyle ) )
      {
        return;
      }

      if ( !selectionLayer->importNamedStyle( doc, errorMsg ) )
      {
        messageBar()->pushMessage( tr( "Cannot paste style" ),
                                   errorMsg,
                                   QgsMessageBar::CRITICAL, messageTimeout() );
        return;
      }

      mLayerTreeView->refreshLayerSymbology( selectionLayer->id() );
      selectionLayer->triggerRepaint();
    }
  }
}

void QgisApp::copyFeatures( QgsFeatureStore &featureStore )
{
  clipboard()->replaceWithCopyOf( featureStore );
}

void QgisApp::refreshMapCanvas()
{
  Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
  {
    //stop any current rendering
    canvas->stopRendering();
    canvas->refreshAllLayers();
  }
}

void QgisApp::canvasRefreshStarted()
{
  mLastRenderTime.restart();
  // if previous render took less than 0.5 seconds, delay the appearance of the
  // render in progress status bar by 0.5 seconds - this avoids the status bar
  // rapidly appearing and then disappearing for very fast renders
  if ( mLastRenderTimeSeconds > 0 && mLastRenderTimeSeconds < 0.5 )
  {
    mRenderProgressBarTimer.setSingleShot( true );
    mRenderProgressBarTimer.setInterval( 500 );
    disconnect( mRenderProgressBarTimerConnection );
    mRenderProgressBarTimerConnection = connect( &mRenderProgressBarTimer, &QTimer::timeout, this, [ = ]()
    {
      showProgress( -1, 0 );
    }
                                               );
    mRenderProgressBarTimer.start();
  }
  else
  {
    showProgress( -1, 0 ); // trick to make progress bar show busy indicator
  }
}

void QgisApp::canvasRefreshFinished()
{
  mRenderProgressBarTimer.stop();
  mLastRenderTimeSeconds = mLastRenderTime.elapsed() / 1000.0;
  showProgress( 0, 0 ); // stop the busy indicator
}

void QgisApp::toggleMapTips( bool enabled )
{
  mMapTipsVisible = enabled;
  // Store if maptips are active
  QgsSettings().setValue( QStringLiteral( "/qgis/enableMapTips" ), mMapTipsVisible );

  // if off, stop the timer
  if ( !mMapTipsVisible )
  {
    mpMapTipsTimer->stop();
  }

  if ( mActionMapTips->isChecked() != mMapTipsVisible )
    mActionMapTips->setChecked( mMapTipsVisible );
}

void QgisApp::toggleEditing()
{
  QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( currentLayer )
  {
    toggleEditing( currentLayer, true );
  }
  else
  {
    // active although there's no layer active!?
    mActionToggleEditing->setChecked( false );
  }
}

bool QgisApp::toggleEditing( QgsMapLayer *layer, bool allowCancel )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return false;
  }

  bool res = true;

  QString connString = QgsDataSourceUri( vlayer->source() ).connectionInfo();
  QString key = vlayer->providerType();

  QMap< QPair< QString, QString>, QgsTransactionGroup *> transactionGroups = QgsProject::instance()->transactionGroups();
  QMap< QPair< QString, QString>, QgsTransactionGroup *>::iterator tIt = transactionGroups .find( qMakePair( key, connString ) );
  QgsTransactionGroup *tg = ( tIt != transactionGroups.end() ? tIt.value() : nullptr );

  bool isModified = false;

  // Assume changes if: a) the layer reports modifications or b) its transaction group was modified
  if ( vlayer->isModified() || ( tg && tg->layers().contains( vlayer ) && tg->modified() ) )
    isModified  = true;

  if ( !vlayer->isEditable() && !vlayer->readOnly() )
  {
    if ( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
    {
      mActionToggleEditing->setChecked( false );
      mActionToggleEditing->setEnabled( false );
      messageBar()->pushMessage( tr( "Start editing failed" ),
                                 tr( "Provider cannot be opened for editing" ),
                                 QgsMessageBar::INFO, messageTimeout() );
      return false;
    }

    vlayer->startEditing();

    QgsSettings settings;
    QString markerType = settings.value( QStringLiteral( "qgis/digitizing/marker_style" ), "Cross" ).toString();
    bool markSelectedOnly = settings.value( QStringLiteral( "qgis/digitizing/marker_only_for_selected" ), true ).toBool();

    // redraw only if markers will be drawn
    if ( ( !markSelectedOnly || vlayer->selectedFeatureCount() > 0 ) &&
         ( markerType == QLatin1String( "Cross" ) || markerType == QLatin1String( "SemiTransparentCircle" ) ) )
    {
      vlayer->triggerRepaint();
    }
  }
  else if ( isModified )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if ( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch ( QMessageBox::information( nullptr,
                                       tr( "Stop editing" ),
                                       tr( "Do you want to save the changes to layer %1?" ).arg( vlayer->name() ),
                                       buttons ) )
    {
      case QMessageBox::Cancel:
        res = false;
        break;

      case QMessageBox::Save:
        QApplication::setOverrideCursor( Qt::WaitCursor );

        if ( !vlayer->commitChanges() )
        {
          commitError( vlayer );
          // Leave the in-memory editing state alone,
          // to give the user a chance to enter different values
          // and try the commit again later
          res = false;
        }

        vlayer->triggerRepaint();

        QApplication::restoreOverrideCursor();
        break;

      case QMessageBox::Discard:
        QApplication::setOverrideCursor( Qt::WaitCursor );

        freezeCanvases();
        if ( !vlayer->rollBack() )
        {
          messageBar()->pushMessage( tr( "Error" ),
                                     tr( "Problems during roll back" ),
                                     QgsMessageBar::CRITICAL );
          res = false;
        }
        freezeCanvases( false );

        vlayer->triggerRepaint();

        QApplication::restoreOverrideCursor();
        break;

      default:
        break;
    }
  }
  else //layer not modified
  {
    freezeCanvases();
    vlayer->rollBack();
    freezeCanvases( false );
    res = true;
    vlayer->triggerRepaint();
  }

  if ( !res && layer == activeLayer() )
  {
    // while also called when layer sends editingStarted/editingStopped signals,
    // this ensures correct restoring of gui state if toggling was canceled
    // or layer commit/rollback functions failed
    activateDeactivateLayerRelatedActions( layer );
  }

  return res;
}

void QgisApp::saveActiveLayerEdits()
{
  saveEdits( activeLayer(), true, true );
}

void QgisApp::saveEdits( QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isEditable() || !vlayer->isModified() )
    return;

  if ( vlayer == activeLayer() )
    mSaveRollbackInProgress = true;

  if ( !vlayer->commitChanges() )
  {
    mSaveRollbackInProgress = false;
    commitError( vlayer );
  }

  if ( leaveEditable )
  {
    vlayer->startEditing();
  }
  if ( triggerRepaint )
  {
    vlayer->triggerRepaint();
  }
}

void QgisApp::cancelEdits( QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isEditable() )
    return;

  if ( vlayer == activeLayer() && leaveEditable )
    mSaveRollbackInProgress = true;

  freezeCanvases();
  if ( !vlayer->rollBack( !leaveEditable ) )
  {
    mSaveRollbackInProgress = false;
    QMessageBox::information( nullptr,
                              tr( "Error" ),
                              tr( "Could not %1 changes to layer %2\n\nErrors: %3\n" )
                              .arg( leaveEditable ? tr( "rollback" ) : tr( "cancel" ),
                                    vlayer->name(),
                                    vlayer->commitErrors().join( QStringLiteral( "\n  " ) ) ) );
  }
  freezeCanvases( false );

  if ( leaveEditable )
  {
    vlayer->startEditing();
  }
  if ( triggerRepaint )
  {
    vlayer->triggerRepaint();
  }
}

void QgisApp::saveEdits()
{
  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    saveEdits( layer, true, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::saveAllEdits( bool verifyAction )
{
  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Save" ), tr( "all" ) ) )
      return;
  }

  Q_FOREACH ( QgsMapLayer *layer, editableLayers( true ) )
  {
    saveEdits( layer, true, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::rollbackEdits()
{
  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    cancelEdits( layer, true, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::rollbackAllEdits( bool verifyAction )
{
  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Rollback" ), tr( "all" ) ) )
      return;
  }

  Q_FOREACH ( QgsMapLayer *layer, editableLayers( true ) )
  {
    cancelEdits( layer, true, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::cancelEdits()
{
  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    cancelEdits( layer, false, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::cancelAllEdits( bool verifyAction )
{
  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Cancel" ), tr( "all" ) ) )
      return;
  }

  Q_FOREACH ( QgsMapLayer *layer, editableLayers() )
  {
    cancelEdits( layer, false, false );
  }
  refreshMapCanvas();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

bool QgisApp::verifyEditsActionDialog( const QString &act, const QString &upon )
{
  bool res = false;
  switch ( QMessageBox::information( nullptr,
                                     tr( "Current edits" ),
                                     tr( "%1 current changes for %2 layer(s)?" )
                                     .arg( act,
                                         upon ),
                                     QMessageBox::Cancel | QMessageBox::Ok ) )
  {
    case QMessageBox::Ok:
      res = true;
      break;
    default:
      break;
  }
  return res;
}

void QgisApp::updateLayerModifiedActions()
{
  bool enableSaveLayerEdits = false;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( vlayer )
  {
    QgsVectorDataProvider *dprovider = vlayer->dataProvider();
    if ( dprovider )
    {
      enableSaveLayerEdits = ( dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues
                               && vlayer->isEditable()
                               && vlayer->isModified() );
    }
  }
  mActionSaveLayerEdits->setEnabled( enableSaveLayerEdits );

  QList<QgsLayerTreeLayer *> selectedLayerNodes = mLayerTreeView ? mLayerTreeView->selectedLayerNodes() : QList<QgsLayerTreeLayer *>();

  mActionSaveEdits->setEnabled( QgsLayerTreeUtils::layersModified( selectedLayerNodes ) );
  mActionRollbackEdits->setEnabled( QgsLayerTreeUtils::layersModified( selectedLayerNodes ) );
  mActionCancelEdits->setEnabled( QgsLayerTreeUtils::layersEditable( selectedLayerNodes ) );

  bool hasEditLayers = !editableLayers().isEmpty();
  mActionAllEdits->setEnabled( hasEditLayers );
  mActionCancelAllEdits->setEnabled( hasEditLayers );

  bool hasModifiedLayers = !editableLayers( true ).isEmpty();
  mActionSaveAllEdits->setEnabled( hasModifiedLayers );
  mActionRollbackAllEdits->setEnabled( hasModifiedLayers );
}

QList<QgsMapLayer *> QgisApp::editableLayers( bool modified ) const
{
  QList<QgsMapLayer *> editLayers;
  // use legend layers (instead of registry) so QList mirrors its order
  Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, mLayerTreeView->layerTreeModel()->rootGroup()->findLayers() )
  {
    if ( !nodeLayer->layer() )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() && ( !modified || vl->isModified() ) )
      editLayers << vl;
  }
  return editLayers;
}

void QgisApp::duplicateVectorStyle( QgsVectorLayer *srcLayer, QgsVectorLayer *destLayer )
{
  // copy symbology, if possible
  if ( srcLayer->geometryType() == destLayer->geometryType() )
  {
    QDomImplementation DomImplementation;
    QDomDocumentType documentType =
      DomImplementation.createDocumentType(
        QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
    QDomDocument doc( documentType );
    QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
    rootNode.setAttribute( QStringLiteral( "version" ), Qgis::QGIS_VERSION );
    doc.appendChild( rootNode );
    QString errorMsg;
    srcLayer->writeSymbology( rootNode, doc, errorMsg, QgsReadWriteContext() );
    destLayer->readSymbology( rootNode, errorMsg, QgsReadWriteContext() );
  }
}

void QgisApp::layerSubsetString()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  bool joins = !vlayer->vectorJoins().isEmpty();
  if ( vlayer->vectorJoins().size() == 1 )
  {
    QgsVectorLayerJoinInfo info = vlayer->vectorJoins()[0];
    joins = !vlayer->joinBuffer()->isAuxiliaryJoin( info );
  }

  if ( joins )
  {
    if ( QMessageBox::question( nullptr, tr( "Filter on joined fields" ),
                                tr( "You are about to set a subset filter on a layer that has joined fields. "
                                    "Joined fields cannot be filtered, unless you convert the layer to a virtual layer first. "
                                    "Would you like to create a virtual layer out of this layer first?" ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
    {
      QgsVirtualLayerDefinition def = QgsVirtualLayerDefinitionUtils::fromJoinedLayer( vlayer );
      QgsVectorLayer *newLayer = new QgsVectorLayer( def.toString(), vlayer->name() + " (virtual)", QStringLiteral( "virtual" ) );
      if ( newLayer->isValid() )
      {
        duplicateVectorStyle( vlayer, newLayer );
        QgsProject::instance()->addMapLayer( newLayer, /*addToLegend*/ false, /*takeOwnership*/ true );
        QgsLayerTreeUtils::insertLayerBelow( QgsProject::instance()->layerTreeRoot(), vlayer, newLayer );
        mLayerTreeView->setCurrentLayer( newLayer );
        // hide the old layer
        QgsLayerTreeLayer *vLayerTreeLayer = QgsProject::instance()->layerTreeRoot()->findLayer( vlayer->id() );
        if ( vLayerTreeLayer )
          vLayerTreeLayer->setItemVisibilityChecked( false );
        vlayer = newLayer;
      }
      else
      {
        delete newLayer;
      }
    }
  }

  // launch the query builder
  std::unique_ptr<QgsQueryBuilder> qb( new QgsQueryBuilder( vlayer, this ) );
  QString subsetBefore = vlayer->subsetString();

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  qb->setSql( vlayer->subsetString() );
  // Open the query builder and refresh symbology if sql has changed
  // Note: repaintRequested is emitted directly from QgsQueryBuilder
  //       when the sql is set in the layer.
  if ( qb->exec() && ( subsetBefore != qb->sql() ) && mLayerTreeView )
  {
    mLayerTreeView->refreshLayerSymbology( vlayer->id() );
    activateDeactivateLayerRelatedActions( vlayer );
  }
}

void QgisApp::saveLastMousePosition( const QgsPointXY &p )
{
  if ( mMapTipsVisible )
  {
    // store the point, we need it for when the maptips timer fires
    mLastMapPosition = p;

    // we use this slot to control the timer for maptips since it is fired each time
    // the mouse moves.
    if ( mMapCanvas->underMouse() )
    {
      // Clear the maptip (this is done conditionally)
      mpMaptip->clear( mMapCanvas );
      // don't start the timer if the mouse is not over the map canvas
      mpMapTipsTimer->start();
      //QgsDebugMsg("Started maptips timer");
    }
  }
}


void QgisApp::showScale( double scale )
{
  mScaleWidget->setScale( scale );

  // Not sure if the lines below do anything meaningful /Homann
  if ( mScaleWidget->width() > mScaleWidget->minimumWidth() )
  {
    mScaleWidget->setMinimumWidth( mScaleWidget->width() );
  }
}


void QgisApp::userRotation()
{
  double degrees = mRotationEdit->value();
  mMapCanvas->setRotation( degrees );
  mMapCanvas->refresh();
}

void QgisApp::projectCrsChanged()
{
  updateCrsStatusBar();
  QgsDebugMsgLevel( QString( "QgisApp::setupConnections -1- : QgsProject::instance()->crs().description[%1]ellipsoid[%2]" ).arg( QgsProject::instance()->crs().description() ).arg( QgsProject::instance()->crs().ellipsoidAcronym() ), 3 );
  mMapCanvas->setDestinationCrs( QgsProject::instance()->crs() );

  // handle datum transforms
  QList<QgsCoordinateReferenceSystem> transformsToAskFor = QList<QgsCoordinateReferenceSystem>();
  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( !transformsToAskFor.contains( it.value()->crs() ) &&
         it.value()->crs() != QgsProject::instance()->crs() &&
         !QgsProject::instance()->transformContext().hasTransform( it.value()->crs(), QgsProject::instance()->crs() ) &&
         QgsDatumTransform::datumTransformations( it.value()->crs(), QgsProject::instance()->crs() ).count() > 1 )
    {
      transformsToAskFor.append( it.value()->crs() );
    }
  }
  if ( transformsToAskFor.count() == 1 )
  {
    askUserForDatumTransform( transformsToAskFor.at( 0 ),
                              QgsProject::instance()->crs() );
  }
  else if ( transformsToAskFor.count() > 1 )
  {
    bool ask = QgsSettings().value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool();
    if ( ask )
    {
      messageBar()->pushMessage( tr( "Datum transforms" ),
                                 tr( "Project CRS changed and datum transforms might need to be adapted." ),
                                 QgsMessageBar::WARNING,
                                 5 );
    }
  }


}

// toggle overview status
void QgisApp::isInOverview()
{
  mLayerTreeView->defaultActions()->showInOverview();
}

void QgisApp::removingLayers( const QStringList &layers )
{
  Q_FOREACH ( const QString &layerId, layers )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(
                               QgsProject::instance()->mapLayer( layerId ) );
    if ( !vlayer || !vlayer->isEditable() )
      return;

    toggleEditing( vlayer, false );
  }
}

void QgisApp::removeLayer()
{
  if ( !mLayerTreeView )
  {
    return;
  }

  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer && vlayer->isEditable() && !toggleEditing( vlayer, true ) )
      return;
  }

  QStringList activeTaskDescriptions;
  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    QList< QgsTask * > tasks = QgsApplication::taskManager()->tasksDependentOnLayer( layer );
    if ( !tasks.isEmpty() )
    {
      Q_FOREACH ( QgsTask *task, tasks )
      {
        activeTaskDescriptions << tr( " • %1" ).arg( task->description() );
      }
    }
  }

  if ( !activeTaskDescriptions.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Active tasks" ),
                          tr( "The following tasks are currently running which depend on this layer:\n\n%1\n\nPlease cancel these tasks and retry." ).arg( activeTaskDescriptions.join( QStringLiteral( "\n" ) ) ) );
    return;
  }

  QList<QgsLayerTreeNode *> selectedNodes = mLayerTreeView->selectedNodes( true );

  //validate selection
  if ( selectedNodes.isEmpty() )
  {
    messageBar()->pushMessage( tr( "No legend entries selected" ),
                               tr( "Select the layers and groups you want to remove in the legend." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  bool promptConfirmation = QgsSettings().value( QStringLiteral( "qgis/askToDeleteLayers" ), true ).toBool();

  // Don't show prompt to remove a empty group.
  if ( selectedNodes.count() == 1
       && selectedNodes.at( 0 )->nodeType() == QgsLayerTreeNode::NodeGroup
       && selectedNodes.at( 0 )->children().count() == 0 )
  {
    promptConfirmation = false;
  }

  bool shiftHeld = QApplication::queryKeyboardModifiers().testFlag( Qt::ShiftModifier );
  //display a warning
  if ( !shiftHeld && promptConfirmation && QMessageBox::warning( this, tr( "Remove layers and groups" ), tr( "Remove %n legend entries?", "number of legend items to remove", selectedNodes.count() ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  Q_FOREACH ( QgsLayerTreeNode *node, selectedNodes )
  {
    QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( node->parent() );
    if ( parentGroup )
      parentGroup->removeChildNode( node );
  }

  showStatusMessage( tr( "%n legend entries removed.", "number of removed legend entries", selectedNodes.count() ) );

  refreshMapCanvas();
}

void QgisApp::duplicateLayers( const QList<QgsMapLayer *> &lyrList )
{
  if ( !mLayerTreeView )
  {
    return;
  }

  const QList<QgsMapLayer *> selectedLyrs = lyrList.empty() ? mLayerTreeView->selectedLayers() : lyrList;
  if ( selectedLyrs.empty() )
  {
    return;
  }

  freezeCanvases();
  QgsMapLayer *dupLayer = nullptr;
  QString layerDupName, unSppType;
  QList<QgsMessageBarItem *> msgBars;

  Q_FOREACH ( QgsMapLayer *selectedLyr, selectedLyrs )
  {
    dupLayer = nullptr;
    unSppType.clear();
    layerDupName = selectedLyr->name() + ' ' + tr( "copy" );

    if ( selectedLyr->type() == QgsMapLayer::PluginLayer )
    {
      unSppType = tr( "Plugin layer" );
    }

    // duplicate the layer's basic parameters

    if ( unSppType.isEmpty() )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( selectedLyr );
      // TODO: add other layer types that can be duplicated
      // currently memory and plugin layers are skipped
      if ( vlayer && vlayer->storageType() == QLatin1String( "Memory storage" ) )
      {
        unSppType = tr( "Memory layer" );
      }
      else if ( vlayer )
      {
        if ( vlayer->auxiliaryLayer() )
          vlayer->auxiliaryLayer()->save();

        dupLayer = vlayer->clone();
      }
    }

    if ( unSppType.isEmpty() && !dupLayer )
    {
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( selectedLyr );
      if ( rlayer )
      {
        dupLayer = rlayer->clone();
      }
    }

    if ( unSppType.isEmpty() && dupLayer && !dupLayer->isValid() )
    {
      msgBars.append( new QgsMessageBarItem(
                        tr( "Duplicate layer: " ),
                        tr( "%1 (duplication resulted in invalid layer)" ).arg( selectedLyr->name() ),
                        QgsMessageBar::WARNING,
                        0,
                        mInfoBar ) );
      continue;
    }

    if ( !unSppType.isEmpty() || !dupLayer )
    {
      msgBars.append( new QgsMessageBarItem(
                        tr( "Duplicate layer: " ),
                        tr( "%1 (%2 type unsupported)" )
                        .arg( selectedLyr->name(),
                              !unSppType.isEmpty() ? QStringLiteral( "'" ) + unSppType + "' " : QLatin1String( "" ) ),
                        QgsMessageBar::WARNING,
                        0,
                        mInfoBar ) );
      continue;
    }

    // add layer to layer registry and legend
    QList<QgsMapLayer *> myList;
    myList << dupLayer;
    QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( false );
    QgsProject::instance()->addMapLayers( myList );
    QgsProject::instance()->layerTreeRegistryBridge()->setEnabled( true );

    QgsLayerTreeLayer *nodeSelectedLyr = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( selectedLyr->id() );
    Q_ASSERT( nodeSelectedLyr );
    Q_ASSERT( QgsLayerTree::isGroup( nodeSelectedLyr->parent() ) );
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( nodeSelectedLyr->parent() );

    QgsLayerTreeLayer *nodeDupLayer = parentGroup->insertLayer( parentGroup->children().indexOf( nodeSelectedLyr ) + 1, dupLayer );

    // always set duplicated layers to not visible so layer can be configured before being turned on
    nodeDupLayer->setItemVisibilityChecked( false );

    // duplicate the layer style
    QString errMsg;
    QDomDocument style;
    selectedLyr->exportNamedStyle( style, errMsg );
    if ( errMsg.isEmpty() )
      dupLayer->importNamedStyle( style, errMsg );
    if ( !errMsg.isEmpty() )
      messageBar()->pushMessage( errMsg,
                                 tr( "Cannot copy style to duplicated layer." ),
                                 QgsMessageBar::CRITICAL, messageTimeout() );
  }

  dupLayer = nullptr;

  freezeCanvases( false );

  // display errors in message bar after duplication of layers
  Q_FOREACH ( QgsMessageBarItem *msgBar, msgBars )
  {
    mInfoBar->pushItem( msgBar );
  }
}

void QgisApp::setLayerScaleVisibility()
{
  if ( !mLayerTreeView )
    return;

  QList<QgsMapLayer *> layers = mLayerTreeView->selectedLayers();

  if ( layers.length() < 1 )
    return;

  QgsScaleVisibilityDialog *dlg = new QgsScaleVisibilityDialog( this, tr( "Set scale visibility for selected layers" ), mMapCanvas );
  QgsMapLayer *layer = mLayerTreeView->currentLayer();
  if ( layer )
  {
    dlg->setScaleVisiblity( layer->hasScaleBasedVisibility() );
    dlg->setMinimumScale( layer->minimumScale() );
    dlg->setMaximumScale( layer->maximumScale() );
  }
  if ( dlg->exec() )
  {
    freezeCanvases();
    Q_FOREACH ( QgsMapLayer *layer, layers )
    {
      layer->setScaleBasedVisibility( dlg->hasScaleVisibility() );
      layer->setMaximumScale( dlg->maximumScale() );
      layer->setMinimumScale( dlg->minimumScale() );
    }
    freezeCanvases( false );
    refreshMapCanvas();
  }
  delete dlg;
}

void QgisApp::zoomToLayerScale()
{
  if ( !mLayerTreeView )
    return;

  QList<QgsMapLayer *> layers = mLayerTreeView->selectedLayers();

  if ( layers.length() < 1 )
    return;

  QgsMapLayer *layer = mLayerTreeView->currentLayer();
  if ( layer && layer->hasScaleBasedVisibility() )
  {
    const double scale = mMapCanvas->scale();
    if ( scale > layer->minimumScale() && layer->minimumScale() > 0 )
    {
      mMapCanvas->zoomScale( layer->minimumScale() * Qgis::SCALE_PRECISION );
    }
    else if ( scale <= layer->maximumScale() && layer->maximumScale() > 0 )
    {
      mMapCanvas->zoomScale( layer->maximumScale() );
    }
  }
}

void QgisApp::setLayerCrs()
{
  if ( !( mLayerTreeView && mLayerTreeView->currentLayer() ) )
  {
    return;
  }

  QgsProjectionSelectionDialog mySelector( this );
  mySelector.setCrs( mLayerTreeView->currentLayer()->crs() );
  mySelector.setMessage( QString() );
  if ( !mySelector.exec() )
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  QgsCoordinateReferenceSystem crs = mySelector.crs();

  Q_FOREACH ( QgsLayerTreeNode *node, mLayerTreeView->selectedNodes() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      Q_FOREACH ( QgsLayerTreeLayer *child, QgsLayerTree::toGroup( node )->findLayers() )
      {
        if ( child->layer() )
        {
          askUserForDatumTransform( crs, QgsProject::instance()->crs() );
          child->layer()->setCrs( crs );
          child->layer()->triggerRepaint();
        }
      }
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
      {
        askUserForDatumTransform( crs, QgsProject::instance()->crs() );
        nodeLayer->layer()->setCrs( crs );
        nodeLayer->layer()->triggerRepaint();
      }
    }
  }

  refreshMapCanvas();
}

void QgisApp::setProjectCrsFromLayer()
{
  if ( !( mLayerTreeView && mLayerTreeView->currentLayer() ) )
  {
    return;
  }

  QgsCoordinateReferenceSystem crs = mLayerTreeView->currentLayer()->crs();
  mMapCanvas->freeze();
  QgsProject::instance()->setCrs( crs );
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
}


void QgisApp::legendLayerZoomNative()
{
  if ( !mLayerTreeView )
    return;

  //find current Layer
  QgsMapLayer *currentLayer = mLayerTreeView->currentLayer();
  if ( !currentLayer )
    return;

  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer );
  if ( layer )
  {
    QgsDebugMsg( "Raster units per pixel  : " + QString::number( layer->rasterUnitsPerPixelX() ) );
    QgsDebugMsg( "MapUnitsPerPixel before : " + QString::number( mMapCanvas->mapUnitsPerPixel() ) );

    // get length of central canvas pixel width in source raster crs
    QgsRectangle e = mMapCanvas->extent();
    QSize s = mMapCanvas->mapSettings().outputSize();
    QgsPointXY p1( e.center().x(), e.center().y() );
    QgsPointXY p2( e.center().x() + e.width() / s.width(), e.center().y() + e.height() / s.height() );
    QgsCoordinateTransform ct( mMapCanvas->mapSettings().destinationCrs(), layer->crs(), QgsProject::instance() );
    p1 = ct.transform( p1 );
    p2 = ct.transform( p2 );
    double width = std::sqrt( p1.sqrDist( p2 ) ); // width (actually the diagonal) of reprojected pixel
    mMapCanvas->zoomByFactor( std::sqrt( layer->rasterUnitsPerPixelX() * layer->rasterUnitsPerPixelX() + layer->rasterUnitsPerPixelY() * layer->rasterUnitsPerPixelY() ) / width );

    mMapCanvas->refresh();
    QgsDebugMsg( "MapUnitsPerPixel after  : " + QString::number( mMapCanvas->mapUnitsPerPixel() ) );
  }
}

void QgisApp::legendLayerStretchUsingCurrentExtent()
{
  if ( !mLayerTreeView )
    return;

  //find current Layer
  QgsMapLayer *currentLayer = mLayerTreeView->currentLayer();
  if ( !currentLayer )
    return;

  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer );
  if ( layer )
  {
    QgsRectangle myRectangle;
    myRectangle = mMapCanvas->mapSettings().outputExtentToLayerExtent( layer, mMapCanvas->extent() );
    layer->refreshContrastEnhancement( myRectangle );

    mLayerTreeView->refreshLayerSymbology( layer->id() );
    refreshMapCanvas();
  }
}

void QgisApp::applyStyleToGroup()
{
  if ( !mLayerTreeView )
    return;

  Q_FOREACH ( QgsLayerTreeNode *node, mLayerTreeView->selectedNodes() )
  {
    if ( QgsLayerTree::isGroup( node ) )
    {
      Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, QgsLayerTree::toGroup( node )->findLayers() )
      {
        if ( nodeLayer->layer() )
        {
          pasteStyle( nodeLayer->layer() );
        }
      }
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
      {
        pasteStyle( nodeLayer->layer() );
      }
    }
  }
}

void QgisApp::legendGroupSetCrs()
{
  if ( !mMapCanvas )
  {
    return;
  }

  QgsLayerTreeGroup *currentGroup = mLayerTreeView->currentGroupNode();
  if ( !currentGroup )
    return;

  QgsProjectionSelectionDialog mySelector( this );
  mySelector.setMessage( QString() );
  if ( !mySelector.exec() )
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  QgsCoordinateReferenceSystem crs = mySelector.crs();
  Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, currentGroup->findLayers() )
  {
    if ( nodeLayer->layer() )
    {
      nodeLayer->layer()->setCrs( crs );
      nodeLayer->layer()->triggerRepaint();
    }
  }
}

void QgisApp::legendGroupSetWmsData()
{
  QgsLayerTreeGroup *currentGroup = mLayerTreeView->currentGroupNode();
  if ( !currentGroup )
    return;
  QgsGroupWmsDataDialog *dlg = new QgsGroupWmsDataDialog( this );
  dlg->setGroupShortName( currentGroup->customProperty( QStringLiteral( "wmsShortName" ) ).toString() );
  dlg->setGroupTitle( currentGroup->customProperty( QStringLiteral( "wmsTitle" ) ).toString() );
  dlg->setGroupTitle( currentGroup->customProperty( QStringLiteral( "wmsAbstract" ) ).toString() );
  if ( dlg->exec() )
  {
    currentGroup->setCustomProperty( QStringLiteral( "wmsShortName" ), dlg->groupShortName() );
    currentGroup->setCustomProperty( QStringLiteral( "wmsTitle" ), dlg->groupTitle() );
    currentGroup->setCustomProperty( QStringLiteral( "wmsAbstract" ), dlg->groupAbstract() );
  }
  delete dlg;
}

void QgisApp::zoomToLayerExtent()
{
  mLayerTreeView->defaultActions()->zoomToLayer( mMapCanvas );
}

void QgisApp::showPluginManager()
{
#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // Call pluginManagerInterface()->showPluginManager() as soon as the plugin installer says the remote data is fetched.
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().showPluginManagerWhenReady()" ) );
  }
  else
#endif
  {
    // Call the pluginManagerInterface directly
    mQgisInterface->pluginManagerInterface()->showPluginManager();
  }
}


// implementation of the python runner
class QgsPythonRunnerImpl : public QgsPythonRunner
{
  public:
    explicit QgsPythonRunnerImpl( QgsPythonUtils *pythonUtils ) : mPythonUtils( pythonUtils ) {}

    bool runCommand( QString command, QString messageOnError = QString() ) override
    {
#ifdef WITH_BINDINGS
      if ( mPythonUtils && mPythonUtils->isEnabled() )
      {
        return mPythonUtils->runString( command, messageOnError, false );
      }
#else
      Q_UNUSED( command );
      Q_UNUSED( messageOnError );
#endif
      return false;
    }

    bool evalCommand( QString command, QString &result ) override
    {
#ifdef WITH_BINDINGS
      if ( mPythonUtils && mPythonUtils->isEnabled() )
      {
        return mPythonUtils->evalString( command, result );
      }
#else
      Q_UNUSED( command );
      Q_UNUSED( result );
#endif
      return false;
    }

  protected:
    QgsPythonUtils *mPythonUtils = nullptr;
};

void QgisApp::loadPythonSupport()
{
  QString pythonlibName( QStringLiteral( "qgispython" ) );
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = QStringLiteral( "%1.%2.%3" ).arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 ).arg( Qgis::QGIS_VERSION_INT % 100 );
  QgsDebugMsg( QString( "load library %1 (%2)" ).arg( pythonlibName, version ) );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      QgsMessageLog::logMessage( tr( "Couldn't load Python support library: %1" ).arg( pythonlib.errorString() ) );
      return;
    }
  }

#ifdef WITH_BINDINGS

  //QgsDebugMsg("Python support library loaded successfully.");
  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = reinterpret_cast< inst >( cast_to_fptr( pythonlib.resolve( "instance" ) ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsMessageLog::logMessage( tr( "Couldn't resolve python support library's instance() symbol." ) );
    return;
  }

  //QgsDebugMsg("Python support library's instance() symbol resolved.");
  mPythonUtils = pythonlib_inst();
  if ( mPythonUtils )
  {
    mPythonUtils->initPython( mQgisInterface );
  }

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsPluginRegistry::instance()->setPythonUtils( mPythonUtils );

    // init python runner
    QgsPythonRunner::setInstance( new QgsPythonRunnerImpl( mPythonUtils ) );

    QgsMessageLog::logMessage( tr( "Python support ENABLED :-) " ), QString(), QgsMessageLog::INFO );
  }
#endif
}

void QgisApp::checkQgisVersion()
{
  QgsVersionInfo *versionInfo = new QgsVersionInfo();
  QApplication::setOverrideCursor( Qt::WaitCursor );

  connect( versionInfo, &QgsVersionInfo::versionInfoAvailable, this, &QgisApp::versionReplyFinished );
  versionInfo->checkVersion();
}

void QgisApp::versionReplyFinished()
{
  QApplication::restoreOverrideCursor();

  QgsVersionInfo *versionInfo = qobject_cast<QgsVersionInfo *>( sender() );
  Q_ASSERT( versionInfo );

  if ( versionInfo->error() == QNetworkReply::NoError )
  {
    QString info;

    if ( versionInfo->newVersionAvailable() )
    {
      info = tr( "There is a new version of QGIS available" );
    }
    else if ( versionInfo->isDevelopmentVersion() )
    {
      info = tr( "You are running a development version of QGIS" );
    }
    else
    {
      info = tr( "You are running the current version of QGIS" );
    }

    info = QStringLiteral( "<b>%1</b>" ).arg( info );

    info += "<br>" + versionInfo->downloadInfo();

    QMessageBox mb( QMessageBox::Information, tr( "QGIS Version Information" ), info );
    mb.setInformativeText( versionInfo->html() );
    mb.exec();
  }
  else
  {
    QMessageBox mb( QMessageBox::Warning, tr( "QGIS Version Information" ), tr( "Unable to get current version information from server" ) );
    mb.setDetailedText( versionInfo->errorString() );
    mb.exec();
  }
}

void QgisApp::configureShortcuts()
{
  QgsConfigureShortcutsDialog dlg( this );
  dlg.exec();
}

void QgisApp::customize()
{
  QgsCustomization::instance()->openDialog( this );
}

void QgisApp::options()
{
  showOptionsDialog( this );
}

void QgisApp::showOptionsDialog( QWidget *parent, const QString &currentPage )
{
  QgsSettings mySettings;
  QString oldScales = mySettings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString();

  QList< QgsOptionsWidgetFactory * > factories;
  Q_FOREACH ( const QPointer< QgsOptionsWidgetFactory > &f, mOptionsWidgetFactories )
  {
    // remove any deleted factories
    if ( f )
      factories << f;
  }
  std::unique_ptr< QgsOptions > optionsDialog( new QgsOptions( parent, QgsGuiUtils::ModalDialogFlags, factories ) );
  if ( !currentPage.isEmpty() )
  {
    optionsDialog->setCurrentPage( currentPage );
  }

  if ( optionsDialog->exec() )
  {
    QgsProject::instance()->layerTreeRegistryBridge()->setNewLayersVisible( mySettings.value( QStringLiteral( "qgis/new_layers_visible" ), true ).toBool() );

    setupLayerTreeViewFromSettings();

    Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
    {
      applyDefaultSettingsToCanvas( canvas );
    }

    //update any open compositions so they reflect new composer settings
    //we have to push the changes to the compositions here, because compositions
    //have no access to qgisapp and accordingly can't listen in to changes
    Q_FOREACH ( QgsComposition *c, QgsProject::instance()->layoutManager()->compositions() )
    {
      c->updateSettings();
    }

    //do we need this? TS
    Q_FOREACH ( QgsMapCanvas *canvas, mapCanvases() )
    {
      canvas->refresh();
    }

    mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

    if ( oldScales != mySettings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString() )
    {
      mScaleWidget->updateScales();
    }

    qobject_cast<QgsMeasureTool *>( mMapTools.mMeasureDist )->updateSettings();
    qobject_cast<QgsMeasureTool *>( mMapTools.mMeasureArea )->updateSettings();
    qobject_cast<QgsMapToolMeasureAngle *>( mMapTools.mMeasureAngle )->updateSettings();

    double factor = mySettings.value( QStringLiteral( "qgis/magnifier_factor_default" ), 1.0 ).toDouble();
    mMagnifierWidget->setDefaultFactor( factor );
    mMagnifierWidget->updateMagnification( factor );
  }
}

void QgisApp::fullHistogramStretch()
{
  histogramStretch( false, QgsRasterMinMaxOrigin::MinMax );
}

void QgisApp::localHistogramStretch()
{
  histogramStretch( true, QgsRasterMinMaxOrigin::MinMax );
}

void QgisApp::fullCumulativeCutStretch()
{
  histogramStretch( false, QgsRasterMinMaxOrigin::CumulativeCut );
}

void QgisApp::localCumulativeCutStretch()
{
  histogramStretch( true, QgsRasterMinMaxOrigin::CumulativeCut );
}

void QgisApp::histogramStretch( bool visibleAreaOnly, QgsRasterMinMaxOrigin::Limits limits )
{
  QgsMapLayer *myLayer = mLayerTreeView->currentLayer();

  if ( !myLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To perform a full histogram stretch, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsRasterLayer *myRasterLayer = qobject_cast<QgsRasterLayer *>( myLayer );
  if ( !myRasterLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To perform a full histogram stretch, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsRectangle myRectangle;
  if ( visibleAreaOnly )
    myRectangle = mMapCanvas->mapSettings().outputExtentToLayerExtent( myRasterLayer, mMapCanvas->extent() );

  myRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, limits, myRectangle );

  myRasterLayer->triggerRepaint();
}

void QgisApp::increaseBrightness()
{
  int step = 1;
  if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
  {
    step = 10;
  }
  adjustBrightnessContrast( step );
}

void QgisApp::decreaseBrightness()
{
  int step = -1;
  if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
  {
    step = -10;
  }
  adjustBrightnessContrast( step );
}

void QgisApp::increaseContrast()
{
  int step = 1;
  if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
  {
    step = 10;
  }
  adjustBrightnessContrast( step, false );
}

void QgisApp::decreaseContrast()
{
  int step = -1;
  if ( QgsApplication::keyboardModifiers() == Qt::ShiftModifier )
  {
    step = -10;
  }
  adjustBrightnessContrast( step, false );
}

void QgisApp::adjustBrightnessContrast( int delta, bool updateBrightness )
{
  Q_FOREACH ( QgsMapLayer *layer, mLayerTreeView->selectedLayers() )
  {
    if ( !layer )
    {
      messageBar()->pushMessage( tr( "No Layer Selected" ),
                                 tr( "To change brightness or contrast, you need to have a raster layer selected." ),
                                 QgsMessageBar::INFO, messageTimeout() );
      return;
    }

    QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
    if ( !rasterLayer )
    {
      messageBar()->pushMessage( tr( "No Layer Selected" ),
                                 tr( "To change brightness or contrast, you need to have a raster layer selected." ),
                                 QgsMessageBar::INFO, messageTimeout() );
      return;
    }

    QgsBrightnessContrastFilter *brightnessFilter = rasterLayer->brightnessFilter();

    if ( updateBrightness )
    {
      brightnessFilter->setBrightness( brightnessFilter->brightness() + delta );
    }
    else
    {
      brightnessFilter->setContrast( brightnessFilter->contrast() + delta );
    }

    rasterLayer->triggerRepaint();
  }
}


void QgisApp::helpContents()
{
  QgsHelp::openHelp( QStringLiteral( "index.html" ) );
}

void QgisApp::apiDocumentation()
{
  if ( QFileInfo::exists( QgsApplication::pkgDataPath() + "/doc/api/index.html" ) )
  {
    openURL( QStringLiteral( "api/index.html" ) );
  }
  else
  {
    QgsSettings settings;
    QString QgisApiUrl = settings.value( QStringLiteral( "qgis/QgisApiUrl" ),
                                         QStringLiteral( "https://qgis.org/api/" ) ).toString();
    openURL( QgisApiUrl, false );
  }
}

void QgisApp::reportaBug()
{
  QgsSettings settings;
  QString reportaBugUrl = settings.value( QStringLiteral( "qgis/reportaBugUrl" ),
                                          tr( "https://qgis.org/en/site/getinvolved/development/bugreporting.html" ) ).toString();
  openURL( reportaBugUrl, false );
}

void QgisApp::supportProviders()
{
  QgsSettings settings;
  QString supportProvidersUrl = settings.value( QStringLiteral( "qgis/supportProvidersUrl" ),
                                tr( "https://qgis.org/en/site/forusers/commercial_support.html" ) ).toString();
  openURL( supportProvidersUrl, false );
}

void QgisApp::helpQgisHomePage()
{
  QgsSettings settings;
  QString  helpQgisHomePageUrl = settings.value( QStringLiteral( "qgis/helpQgisHomePageUrl" ),
                                 QStringLiteral( "https://qgis.org" ) ).toString();
  openURL( helpQgisHomePageUrl, false );
}

void QgisApp::openURL( QString url, bool useQgisDocDirectory )
{
  // open help in user browser
  if ( useQgisDocDirectory )
  {
    url = "file://" + QgsApplication::pkgDataPath() + "/doc/" + url;
  }
#ifdef Q_OS_MACX
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes( kCFAllocatorDefault,
                                          reinterpret_cast<const UInt8 *>( url.toUtf8().data() ), url.length(),
                                          kCFStringEncodingUTF8, nullptr );
  OSStatus status = LSOpenCFURLRef( urlRef, nullptr );
  status = 0; //avoid compiler warning
  CFRelease( urlRef );
#elif defined(Q_OS_WIN)
  if ( url.startsWith( "file://", Qt::CaseInsensitive ) )
    ShellExecute( 0, 0, url.mid( 7 ).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL );
  else
    QDesktopServices::openUrl( url );
#else
  QDesktopServices::openUrl( url );
#endif
}

void QgisApp::registerMapLayerPropertiesFactory( QgsMapLayerConfigWidgetFactory *factory )
{
  mMapLayerPanelFactories << factory;
  if ( mMapStyleWidget )
    mMapStyleWidget->setPageFactories( mMapLayerPanelFactories );
}

void QgisApp::unregisterMapLayerPropertiesFactory( QgsMapLayerConfigWidgetFactory *factory )
{
  mMapLayerPanelFactories.removeAll( factory );
  if ( mMapStyleWidget )
    mMapStyleWidget->setPageFactories( mMapLayerPanelFactories );
}

void QgisApp::registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  mOptionsWidgetFactories << factory;
}

void QgisApp::unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  mOptionsWidgetFactories.removeAll( factory );
}

QgsMapLayer *QgisApp::activeLayer()
{
  return mLayerTreeView ? mLayerTreeView->currentLayer() : nullptr;
}

QSize QgisApp::iconSize( bool dockedToolbar ) const
{
  QgsSettings s;
  int size = s.value( QStringLiteral( "/IconSize" ), 32 ).toInt();

  if ( dockedToolbar )
  {
    size = dockedToolbarIconSize( size );
  }

  return QSize( size, size );
}

bool QgisApp::setActiveLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  if ( !mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( layer->id() ) )
    return false;

  mLayerTreeView->setCurrentLayer( layer );
  return true;
}

void QgisApp::reloadConnections()
{
  emit connectionsChanged( );
}

void QgisApp::showLayoutManager()
{
  if ( !mLayoutManagerDialog )
  {
    mLayoutManagerDialog = new QgsLayoutManagerDialog( this, Qt::Window );
    mLayoutManagerDialog->setAttribute( Qt::WA_DeleteOnClose );
  }
  mLayoutManagerDialog->show();
  mLayoutManagerDialog->activate();
}

QgsVectorLayer *QgisApp::addVectorLayer( const QString &vectorLayerPath, const QString &name, const QString &providerKey )
{
  bool wasfrozen = mMapCanvas->isFrozen();

  freezeCanvases();

  QString baseName = QgsMapLayer::formatLayerName( name );

  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
  QgsDebugMsg( "Creating new vector layer using " + vectorLayerPath
               + " with baseName of " + baseName
               + " and providerKey of " + providerKey );

  // if the layer needs authentication, ensure the master password is set
  bool authok = true;
  QRegExp rx( "authcfg=([a-z]|[A-Z]|[0-9]){7}" );
  if ( rx.indexIn( vectorLayerPath ) != -1 )
  {
    authok = false;
    if ( !QgsAuthGuiUtils::isDisabled( messageBar(), messageTimeout() ) )
    {
      authok = QgsApplication::authManager()->setMasterPassword( true );
    }
  }

  // create the layer
  QgsVectorLayer::LayerOptions options;
  options.loadDefaultStyle = false;
  QgsVectorLayer *layer = new QgsVectorLayer( vectorLayerPath, baseName, providerKey, options );

  if ( authok && layer && layer->isValid() )
  {
    QStringList sublayers = layer->dataProvider()->subLayers();
    QgsDebugMsg( QString( "got valid layer with %1 sublayers" ).arg( sublayers.count() ) );

    // If the newly created layer has more than 1 layer of data available, we show the
    // sublayers selection dialog so the user can select the sublayers to actually load.
    if ( sublayers.count() > 1 &&
         ! vectorLayerPath.contains( QStringLiteral( "layerid=" ) ) &&
         ! vectorLayerPath.contains( QStringLiteral( "layername=" ) ) )
    {
      askUserForOGRSublayers( layer );

      // The first layer loaded is not useful in that case. The user can select it in
      // the list if he wants to load it.
      delete layer;
      layer = nullptr;
    }
    else
    {
      // Register this layer with the layers registry
      QList<QgsMapLayer *> myList;

      //set friendly name for datasources with only one layer
      QStringList sublayers = layer->dataProvider()->subLayers();
      if ( !sublayers.isEmpty() )
      {
        QStringList elements = sublayers.at( 0 ).split( QgsDataProvider::SUBLAYER_SEPARATOR );
        QString subLayerNameFormatted = elements.size() >= 2 ? QgsMapLayer::formatLayerName( elements.at( 1 ) ) : QString();

        if ( elements.size() >= 4 && layer->name().compare( elements.at( 1 ), Qt::CaseInsensitive ) != 0
             && layer->name().compare( subLayerNameFormatted, Qt::CaseInsensitive ) != 0 )
        {
          layer->setName( QStringLiteral( "%1 %2" ).arg( layer->name(), elements.at( 1 ) ) );
        }
      }

      myList << layer;
      QgsProject::instance()->addMapLayers( myList );
      bool ok;
      layer->loadDefaultStyle( ok );
      layer->loadDefaultMetadata( ok );
    }
  }
  else
  {
    QString message = layer->dataProvider()->error().message( QgsErrorMessage::Text );
    QString msg = tr( "The layer %1 is not a valid layer and can not be added to the map. Reason: %2" ).arg( vectorLayerPath ).arg( message );
    messageBar()->pushMessage( tr( "Layer is not valid" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );

    delete layer;
    freezeCanvases( false );
    return nullptr;
  }

  // Only update the map if we frozen in this method
  // Let the caller do it otherwise
  if ( !wasfrozen )
  {
    freezeCanvases( false );
    refreshMapCanvas();
  }

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  return layer;

} // QgisApp::addVectorLayer



void QgisApp::addMapLayer( QgsMapLayer *mapLayer )
{
  freezeCanvases();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  if ( mapLayer->isValid() )
  {
    // Register this layer with the layers registry
    QList<QgsMapLayer *> myList;
    myList << mapLayer;
    QgsProject::instance()->addMapLayers( myList );
    // add it to the mapcanvas collection
    // not necessary since adding to registry adds to canvas mMapCanvas->addLayer(theMapLayer);
  }
  else
  {
    QString msg = tr( "The layer is not a valid layer and can not be added to the map" );
    messageBar()->pushMessage( tr( "Layer is not valid" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
  }

  // draw the map
  freezeCanvases( false );
  refreshMapCanvas();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

}

void QgisApp::embedLayers()
{
  //dialog to select groups/layers from other project files
  QgsProjectLayerGroupDialog d( this );
  if ( d.exec() == QDialog::Accepted && d.isValid() )
  {
    freezeCanvases();

    QString projectFile = d.selectedProjectFile();

    //groups
    QStringList groups = d.selectedGroups();
    QStringList::const_iterator groupIt = groups.constBegin();
    for ( ; groupIt != groups.constEnd(); ++groupIt )
    {
      QgsLayerTreeGroup *newGroup = QgsProject::instance()->createEmbeddedGroup( *groupIt, projectFile, QStringList() );

      if ( newGroup )
        QgsProject::instance()->layerTreeRoot()->addChildNode( newGroup );
    }

    //layer ids
    QList<QDomNode> brokenNodes;

    // resolve dependencies
    QgsLayerDefinition::DependencySorter depSorter( projectFile );
    QStringList sortedIds = depSorter.sortedLayerIds();
    QStringList layerIds = d.selectedLayerIds();
    Q_FOREACH ( const QString &id, sortedIds )
    {
      Q_FOREACH ( const QString &selId, layerIds )
      {
        if ( selId == id )
          QgsProject::instance()->createEmbeddedLayer( selId, projectFile, brokenNodes );
      }
    }

    freezeCanvases( false );
    if ( !groups.isEmpty() || !layerIds.isEmpty() )
    {
      refreshMapCanvas();
    }
  }
}

void QgisApp::newMapCanvas()
{
  int i = 1;

  bool existing = true;
  QList< QgsMapCanvas * > existingCanvases = mapCanvases();
  QString name;
  while ( existing )
  {
    name = tr( "Map %1" ).arg( i++ );
    existing = false;
    Q_FOREACH ( QgsMapCanvas *canvas, existingCanvases )
    {
      if ( canvas->objectName() == name )
      {
        existing = true;
        break;
      }
    }
  }

  QgsMapCanvasDockWidget *dock = createNewMapCanvasDock( name );
  if ( dock )
  {
    setupDockWidget( dock, true );
    dock->mapCanvas()->setLayers( mMapCanvas->layers() );
    dock->mapCanvas()->setExtent( mMapCanvas->extent() );
    QgsDebugMsgLevel( QString( "QgisApp::newMapCanvas() -4- : QgsProject::instance()->crs().description[%1] ellipsoid[%2]" ).arg( QgsProject::instance()->crs().description() ).arg( QgsProject::instance()->crs().ellipsoidAcronym() ), 3 );
    dock->mapCanvas()->setDestinationCrs( QgsProject::instance()->crs() );
    dock->mapCanvas()->freeze( false );
  }
}

void QgisApp::init3D()
{
#ifdef HAVE_3D
  // register 3D renderers
  QgsApplication::instance()->renderer3DRegistry()->addRenderer( new QgsVectorLayer3DRendererMetadata );
#else
  mActionNew3DMapCanvas->setVisible( false );
#endif
}

void QgisApp::initNativeProcessing()
{
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
#ifdef HAVE_3D
  QgsApplication::processingRegistry()->addProvider( new Qgs3DAlgorithms( QgsApplication::processingRegistry() ) );
#endif
}

void QgisApp::initLayouts()
{
  QgsLayoutAppUtils::registerGuiForKnownItemTypes();

  mLayoutQptDropHandler = new QgsLayoutQptDropHandler( this );
  registerCustomLayoutDropHandler( mLayoutQptDropHandler );
}

void QgisApp::new3DMapCanvas()
{
#ifdef HAVE_3D

  // initialize from project
  QgsProject *prj = QgsProject::instance();
  QgsRectangle fullExtent = mMapCanvas->fullExtent();

  // some layers may go crazy and make full extent unusable
  // we can't go any further - invalid extent would break everything
  if ( fullExtent.isEmpty() || !fullExtent.isFinite() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Project extent is not valid." ) );
    return;
  }

  if ( mMapCanvas->mapSettings().destinationCrs().isGeographic() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "3D view currently does not support unprojected coordinate reference systems (CRS).\nPlease switch project's CRS to a projected CRS." ) );
    return;
  }

  int i = 1;

  bool existing = true;
  const QList< Qgs3DMapCanvas * > existingCanvases = findChildren< Qgs3DMapCanvas * >();
  QString name;
  while ( existing )
  {
    name = tr( "3D Map %1" ).arg( i++ );
    existing = false;
    for ( Qgs3DMapCanvas *canvas : existingCanvases )
    {
      if ( canvas->objectName() == name )
      {
        existing = true;
        break;
      }
    }
  }

  Qgs3DMapCanvasDockWidget *dock = createNew3DMapCanvasDock( name );
  if ( dock )
  {
    setupDockWidget( dock, true );

    Qgs3DMapSettings *map = new Qgs3DMapSettings;
    map->setCrs( prj->crs() );
    map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
    map->setSelectionColor( mMapCanvas->selectionColor() );
    map->setBackgroundColor( mMapCanvas->canvasColor() );
    map->setLayers( mMapCanvas->layers() );
    map->setTransformContext( QgsProject::instance()->transformContext() );
    connect( QgsProject::instance(), &QgsProject::transformContextChanged, map, [map]
    {
      map->setTransformContext( QgsProject::instance()->transformContext() );
    } );

    QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
    flatTerrain->setCrs( map->crs() );
    flatTerrain->setExtent( fullExtent );
    map->setTerrainGenerator( flatTerrain );

    dock->setMapSettings( map );

    QgsRectangle extent = mMapCanvas->extent();
    float dist = qMax( extent.width(), extent.height() );
    dock->mapCanvas3D()->setViewFromTop( mMapCanvas->extent().center(), dist, mMapCanvas->rotation() );
  }
#endif
}

Qgs3DMapCanvasDockWidget *QgisApp::createNew3DMapCanvasDock( const QString &name )
{
#ifdef HAVE_3D
  const QList<Qgs3DMapCanvas *> mapCanvases = findChildren<Qgs3DMapCanvas *>();
  for ( Qgs3DMapCanvas *canvas : mapCanvases )
  {
    if ( canvas->objectName() == name )
    {
      QgsDebugMsg( tr( "A map canvas with name '%1' already exists!" ).arg( name ) );
      return nullptr;
    }
  }

  Qgs3DMapCanvasDockWidget *map3DWidget = new Qgs3DMapCanvasDockWidget( this );
  map3DWidget->setAllowedAreas( Qt::AllDockWidgetAreas );
  map3DWidget->setWindowTitle( name );
  map3DWidget->mapCanvas3D()->setObjectName( name );
  map3DWidget->setMainCanvas( mMapCanvas );
  return map3DWidget;
#else
  Q_UNUSED( name );
  return nullptr;
#endif
}

void QgisApp::setExtent( const QgsRectangle &rect )
{
  mMapCanvas->setExtent( rect );
}

/**
  Prompt and save if project has been modified.
  @return true if saved or discarded, false if canceled
 */
bool QgisApp::saveDirty()
{
  QString whyDirty;
  bool hasUnsavedEdits = false;
  // extra check to see if there are any vector layers with unsaved provider edits
  // to ensure user has opportunity to save any editing
  if ( QgsProject::instance()->count() > 0 )
  {
    QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
    for ( QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
      if ( !vl )
      {
        continue;
      }

      hasUnsavedEdits = ( vl->isEditable() && vl->isModified() );
      if ( hasUnsavedEdits )
      {
        break;
      }
    }

    if ( hasUnsavedEdits )
    {
      markDirty();
      whyDirty = QStringLiteral( "<p style='color:darkred;'>" );
      whyDirty += tr( "Project has layer(s) in edit mode with unsaved edits, which will NOT be saved!" );
      whyDirty += QLatin1String( "</p>" );
    }
  }

  QMessageBox::StandardButton answer( QMessageBox::Discard );
  freezeCanvases();

  //QgsDebugMsg(QString("Layer count is %1").arg(mMapCanvas->layerCount()));
  //QgsDebugMsg(QString("Project is %1dirty").arg( QgsProject::instance()->isDirty() ? "" : "not "));
  //QgsDebugMsg(QString("Map canvas is %1dirty").arg(mMapCanvas->isDirty() ? "" : "not "));

  QgsSettings settings;
  bool askThem = settings.value( QStringLiteral( "qgis/askToSaveProjectChanges" ), true ).toBool();

  if ( askThem && QgsProject::instance()->isDirty() && QgsProject::instance()->count() > 0 )
  {
    // flag project as dirty since dirty state of canvas is reset if "dirty"
    // is based on a zoom or pan
    markDirty();

    // old code: mProjectIsDirtyFlag = true;

    // prompt user to save
    answer = QMessageBox::information( this, tr( "Save Project?" ),
                                       tr( "Do you want to save the current project? %1" )
                                       .arg( whyDirty ),
                                       QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard,
                                       hasUnsavedEdits ? QMessageBox::Cancel : QMessageBox::Save );
    if ( QMessageBox::Save == answer )
    {
      if ( !fileSave() )
        answer = QMessageBox::Cancel;
    }
  }

  freezeCanvases( false );

  return answer != QMessageBox::Cancel;
}

bool QgisApp::checkTasksDependOnProject()
{
  QSet< QString > activeTaskDescriptions;
  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();

  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QList< QgsTask * > tasks = QgsApplication::taskManager()->tasksDependentOnLayer( layerIt.value() );
    if ( !tasks.isEmpty() )
    {
      Q_FOREACH ( QgsTask *task, tasks )
      {
        activeTaskDescriptions.insert( trUtf8( " • %1" ).arg( task->description() ) );
      }
    }
  }

  if ( !activeTaskDescriptions.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Active Tasks" ),
                          tr( "The following tasks are currently running which depend on layers in this project:\n\n%1\n\nPlease cancel these tasks and retry." ).arg( activeTaskDescriptions.toList().join( QStringLiteral( "\n" ) ) ) );
    return true;
  }
  return false;
}

void QgisApp::closeProject()
{
  // unload the project macros before changing anything
  if ( mTrustedMacros )
  {
    QgsPythonRunner::run( QStringLiteral( "qgis.utils.unloadProjectMacros();" ) );
  }

  mTrustedMacros = false;

  mLegendExpressionFilterButton->setExpressionText( QLatin1String( "" ) );
  mLegendExpressionFilterButton->setChecked( false );
  mActionFilterLegend->setChecked( false );

  closeAdditionalMapCanvases();
  closeAdditional3DMapCanvases();

  deleteLayoutDesigners();

  // ensure layout widgets are fully deleted
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  removeAnnotationItems();

  // clear out any stuff from project
  mMapCanvas->freeze( true );
  mMapCanvas->setLayers( QList<QgsMapLayer *>() );
  mMapCanvas->clearCache();
  mOverviewCanvas->setLayers( QList<QgsMapLayer *>() );
  mMapCanvas->freeze( false );
  QgsProject::instance()->clear();
}


void QgisApp::changeEvent( QEvent *event )
{
  QMainWindow::changeEvent( event );
#ifdef Q_OS_MAC
  switch ( event->type() )
  {
    case QEvent::ActivationChange:
      if ( QApplication::activeWindow() == this )
      {
        mWindowAction->setChecked( true );
      }
      // this should not be necessary since the action is part of an action group
      // however this check is not cleared if PrintComposer is closed and reopened
      else
      {
        mWindowAction->setChecked( false );
      }
      break;

    case QEvent::WindowTitleChange:
      mWindowAction->setText( windowTitle() );
      break;

    default:
      break;
  }
#endif
}

void QgisApp::closeEvent( QCloseEvent *event )
{
  // We'll close in our own good time, thank you very much
  event->ignore();
  // Do the usual checks and ask if they want to save, etc
  fileExit();
}


void QgisApp::whatsThis()
{
  QWhatsThis::enterWhatsThisMode();
} // QgisApp::whatsThis()

QMenu *QgisApp::getPluginMenu( const QString &menuName )
{
  /* Plugin menu items are below the plugin separator (which may not exist yet
   * if no plugins are loaded) and above the python separator. If python is not
   * present, there is no python separator and the plugin list is at the bottom
   * of the menu.
   */

  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QAction *before = mActionPluginSeparator2;  // python separator or end of list
  if ( !mActionPluginSeparator1 )
  {
    // First plugin - create plugin list separator
    mActionPluginSeparator1 = mPluginMenu->insertSeparator( before );
  }
  else
  {
    QString dst = cleanedMenuName;
    dst.remove( QChar( '&' ) );

    // Plugins exist - search between plugin separator and python separator or end of list
    QList<QAction *> actions = mPluginMenu->actions();
    int end = mActionPluginSeparator2 ? actions.indexOf( mActionPluginSeparator2 ) : actions.count();
    for ( int i = actions.indexOf( mActionPluginSeparator1 ) + 1; i < end; i++ )
    {
      QString src = actions.at( i )->text();
      src.remove( QChar( '&' ) );

      int comp = dst.localeAwareCompare( src );
      if ( comp < 0 )
      {
        // Add item before this one
        before = actions.at( i );
        break;
      }
      else if ( comp == 0 )
      {
        // Plugin menu item already exists
        return actions.at( i )->menu();
      }
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, this );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  // Where to put it? - we worked that out above...
  mPluginMenu->insertMenu( before, menu );

  return menu;
}

void QgisApp::addPluginToMenu( const QString &name, QAction *action )
{
  QMenu *menu = getPluginMenu( name );
  menu->addAction( action );
}

void QgisApp::removePluginMenu( const QString &name, QAction *action )
{
  QMenu *menu = getPluginMenu( name );
  menu->removeAction( action );
  if ( menu->actions().isEmpty() )
  {
    mPluginMenu->removeAction( menu->menuAction() );
  }
  // Remove separator above plugins in Plugin menu if no plugins remain
  QList<QAction *> actions = mPluginMenu->actions();
  int end = mActionPluginSeparator2 ? actions.indexOf( mActionPluginSeparator2 ) : actions.count();
  if ( actions.indexOf( mActionPluginSeparator1 ) + 1 == end )
  {
    mPluginMenu->removeAction( mActionPluginSeparator1 );
    mActionPluginSeparator1 = nullptr;
  }
}

QMenu *QgisApp::getDatabaseMenu( const QString &menuName )
{
  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString dst = cleanedMenuName;
  dst.remove( QChar( '&' ) );

  QAction *before = nullptr;
  QList<QAction *> actions = mDatabaseMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, this );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  if ( before )
    mDatabaseMenu->insertMenu( before, menu );
  else
    mDatabaseMenu->addMenu( menu );

  return menu;
}

QMenu *QgisApp::getRasterMenu( const QString &menuName )
{
  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif

  QAction *before = nullptr;
  if ( !mActionRasterSeparator )
  {
    // First plugin - create plugin list separator
    mActionRasterSeparator = mRasterMenu->insertSeparator( before );
  }
  else
  {
    QString dst = cleanedMenuName;
    dst.remove( QChar( '&' ) );
    // Plugins exist - search between plugin separator and python separator or end of list
    QList<QAction *> actions = mRasterMenu->actions();
    for ( int i = actions.indexOf( mActionRasterSeparator ) + 1; i < actions.count(); i++ )
    {
      QString src = actions.at( i )->text();
      src.remove( QChar( '&' ) );

      int comp = dst.localeAwareCompare( src );
      if ( comp < 0 )
      {
        // Add item before this one
        before = actions.at( i );
        break;
      }
      else if ( comp == 0 )
      {
        // Plugin menu item already exists
        return actions.at( i )->menu();
      }
    }
  }

  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, this );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  if ( before )
    mRasterMenu->insertMenu( before, menu );
  else
    mRasterMenu->addMenu( menu );

  return menu;
}

QMenu *QgisApp::getVectorMenu( const QString &menuName )
{
  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString dst = cleanedMenuName;
  dst.remove( QChar( '&' ) );

  QAction *before = nullptr;
  QList<QAction *> actions = mVectorMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, this );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  if ( before )
    mVectorMenu->insertMenu( before, menu );
  else
    mVectorMenu->addMenu( menu );

  return menu;
}

QMenu *QgisApp::getWebMenu( const QString &menuName )
{
  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString dst = cleanedMenuName;
  dst.remove( QChar( '&' ) );

  QAction *before = nullptr;
  QList<QAction *> actions = mWebMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, this );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  if ( before )
    mWebMenu->insertMenu( before, menu );
  else
    mWebMenu->addMenu( menu );

  return menu;
}

void QgisApp::insertAddLayerAction( QAction *action )
{
  mAddLayerMenu->insertAction( mActionAddLayerSeparator, action );
}

void QgisApp::removeAddLayerAction( QAction *action )
{
  mAddLayerMenu->removeAction( action );
}

void QgisApp::addPluginToDatabaseMenu( const QString &name, QAction *action )
{
  QMenu *menu = getDatabaseMenu( name );
  menu->addAction( action );

  // add the Database menu to the menuBar if not added yet
  if ( mDatabaseMenu->actions().count() != 1 )
    return;

  QAction *before = nullptr;
  QList<QAction *> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mDatabaseMenu )
      return;

    // goes before Web menu, if present
    if ( actions.at( i )->menu() == mWebMenu )
    {
      before = actions.at( i );
      break;
    }
  }
  for ( int i = 0; i < actions.count(); i++ )
  {
    // defaults to after Raster menu, which is already in qgisapp.ui
    if ( actions.at( i )->menu() == mRasterMenu )
    {
      if ( !before )
      {
        before = actions.at( i += 1 );
        break;
      }
    }
  }
  if ( before )
    menuBar()->insertMenu( before, mDatabaseMenu );
  else
    // fallback insert
    menuBar()->insertMenu( firstRightStandardMenu()->menuAction(), mDatabaseMenu );
}

void QgisApp::addPluginToRasterMenu( const QString &name, QAction *action )
{
  QMenu *menu = getRasterMenu( name );
  menu->addAction( action );
}

void QgisApp::addPluginToVectorMenu( const QString &name, QAction *action )
{
  QMenu *menu = getVectorMenu( name );
  menu->addAction( action );
}

void QgisApp::addPluginToWebMenu( const QString &name, QAction *action )
{
  QMenu *menu = getWebMenu( name );
  menu->addAction( action );

  // add the Vector menu to the menuBar if not added yet
  if ( mWebMenu->actions().count() != 1 )
    return;

  QAction *before = nullptr;
  QList<QAction *> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    // goes after Database menu, if present
    if ( actions.at( i )->menu() == mDatabaseMenu )
    {
      before = actions.at( i += 1 );
      // don't break here
    }

    if ( actions.at( i )->menu() == mWebMenu )
      return;
  }
  for ( int i = 0; i < actions.count(); i++ )
  {
    // defaults to after Raster menu, which is already in qgisapp.ui
    if ( actions.at( i )->menu() == mRasterMenu )
    {
      if ( !before )
      {
        before = actions.at( i += 1 );
        break;
      }
    }
  }

  if ( before )
    menuBar()->insertMenu( before, mWebMenu );
  else
    // fallback insert
    menuBar()->insertMenu( firstRightStandardMenu()->menuAction(), mWebMenu );
}

void QgisApp::removePluginDatabaseMenu( const QString &name, QAction *action )
{
  QMenu *menu = getDatabaseMenu( name );
  menu->removeAction( action );
  if ( menu->actions().isEmpty() )
  {
    mDatabaseMenu->removeAction( menu->menuAction() );
  }

  // remove the Database menu from the menuBar if there are no more actions
  if ( !mDatabaseMenu->actions().isEmpty() )
    return;

  QList<QAction *> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mDatabaseMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

void QgisApp::removePluginRasterMenu( const QString &name, QAction *action )
{
  QMenu *menu = getRasterMenu( name );
  menu->removeAction( action );
  if ( menu->actions().isEmpty() )
  {
    mRasterMenu->removeAction( menu->menuAction() );
  }

  // Remove separator above plugins in Raster menu if no plugins remain
  QList<QAction *> actions = mRasterMenu->actions();
  if ( actions.indexOf( mActionRasterSeparator ) + 1 == actions.count() )
  {
    mRasterMenu->removeAction( mActionRasterSeparator );
    mActionRasterSeparator = nullptr;
  }
}

void QgisApp::removePluginVectorMenu( const QString &name, QAction *action )
{
  QMenu *menu = getVectorMenu( name );
  menu->removeAction( action );
  if ( menu->actions().isEmpty() )
  {
    mVectorMenu->removeAction( menu->menuAction() );
  }

  // remove the Vector menu from the menuBar if there are no more actions
  if ( !mVectorMenu->actions().isEmpty() )
    return;

  QList<QAction *> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mVectorMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

void QgisApp::removePluginWebMenu( const QString &name, QAction *action )
{
  QMenu *menu = getWebMenu( name );
  menu->removeAction( action );
  if ( menu->actions().isEmpty() )
  {
    mWebMenu->removeAction( menu->menuAction() );
  }

  // remove the Web menu from the menuBar if there are no more actions
  if ( !mWebMenu->actions().isEmpty() )
    return;

  QList<QAction *> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mWebMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

int QgisApp::addPluginToolBarIcon( QAction *qAction )
{
  mPluginToolBar->addAction( qAction );
  return 0;
}

QAction *QgisApp::addPluginToolBarWidget( QWidget *widget )
{
  return mPluginToolBar->addWidget( widget );
}

void QgisApp::removePluginToolBarIcon( QAction *qAction )
{
  mPluginToolBar->removeAction( qAction );
}

int QgisApp::addRasterToolBarIcon( QAction *qAction )
{
  mRasterToolBar->addAction( qAction );
  return 0;
}

QAction *QgisApp::addRasterToolBarWidget( QWidget *widget )
{
  return mRasterToolBar->addWidget( widget );
}

void QgisApp::removeRasterToolBarIcon( QAction *qAction )
{
  mRasterToolBar->removeAction( qAction );
}

int QgisApp::addVectorToolBarIcon( QAction *qAction )
{
  mVectorToolBar->addAction( qAction );
  return 0;
}

QAction *QgisApp::addVectorToolBarWidget( QWidget *widget )
{
  return mVectorToolBar->addWidget( widget );
}

void QgisApp::removeVectorToolBarIcon( QAction *qAction )
{
  mVectorToolBar->removeAction( qAction );
}

int QgisApp::addDatabaseToolBarIcon( QAction *qAction )
{
  mDatabaseToolBar->addAction( qAction );
  return 0;
}

QAction *QgisApp::addDatabaseToolBarWidget( QWidget *widget )
{
  return mDatabaseToolBar->addWidget( widget );
}

void QgisApp::removeDatabaseToolBarIcon( QAction *qAction )
{
  mDatabaseToolBar->removeAction( qAction );
}

int QgisApp::addWebToolBarIcon( QAction *qAction )
{
  mWebToolBar->addAction( qAction );
  return 0;
}

QAction *QgisApp::addWebToolBarWidget( QWidget *widget )
{
  return mWebToolBar->addWidget( widget );
}

void QgisApp::removeWebToolBarIcon( QAction *qAction )
{
  mWebToolBar->removeAction( qAction );
}

void QgisApp::updateCrsStatusBar()
{
  if ( QgsProject::instance()->crs().isValid() )
  {
    mOnTheFlyProjectionStatusButton->setText( QgsProject::instance()->crs().authid() );

    mOnTheFlyProjectionStatusButton->setToolTip(
      tr( "Current CRS: %1" ).arg( QgsProject::instance()->crs().description() ) );
    mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconProjectionEnabled.svg" ) ) );
  }
  else
  {
    mOnTheFlyProjectionStatusButton->setText( QString() );
    mOnTheFlyProjectionStatusButton->setToolTip( tr( "No projection" ) );
    mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconProjectionDisabled.svg" ) ) );
  }
}

// slot to update the progress bar in the status bar
void QgisApp::showProgress( int progress, int totalSteps )
{
  if ( progress == totalSteps )
  {
    mProgressBar->reset();
    mProgressBar->hide();
  }
  else
  {
    //only call show if not already hidden to reduce flicker
    if ( !mProgressBar->isVisible() )
    {
      mProgressBar->show();
    }
    mProgressBar->setMaximum( totalSteps );
    mProgressBar->setValue( progress );

    if ( mProgressBar->maximum() == 0 )
    {
      // for busy indicator (when minimum equals to maximum) the oxygen Qt style (used in KDE)
      // has some issues and does not start busy indicator animation. This is an ugly fix
      // that forces creation of a temporary progress bar that somehow resumes the animations.
      // Caution: looking at the code below may introduce mild pain in stomach.
      if ( strcmp( QApplication::style()->metaObject()->className(), "Oxygen::Style" ) == 0 )
      {
        QProgressBar pb;
        pb.setAttribute( Qt::WA_DontShowOnScreen ); // no visual annoyance
        pb.setMaximum( 0 );
        pb.show();
        qApp->processEvents();
      }
    }

  }
}

void QgisApp::mapToolChanged( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  if ( oldTool )
  {
    disconnect( oldTool, &QgsMapTool::messageEmitted, this, &QgisApp::displayMapToolMessage );
    disconnect( oldTool, &QgsMapTool::messageEmitted, this, &QgisApp::displayMapToolMessage );
    disconnect( oldTool, &QgsMapTool::messageDiscarded, this, &QgisApp::removeMapToolMessage );
  }

  if ( newTool )
  {
    if ( !( newTool->flags() & QgsMapTool::EditTool ) )
    {
      mNonEditMapTool = newTool;
    }

    connect( newTool, &QgsMapTool::messageEmitted, this, &QgisApp::displayMapToolMessage );
    connect( newTool, &QgsMapTool::messageEmitted, this, &QgisApp::displayMapToolMessage );
    connect( newTool, &QgsMapTool::messageDiscarded, this, &QgisApp::removeMapToolMessage );
  }
}

void QgisApp::showMapCanvas()
{
  // Map layers changed -> switch to map canvas
  if ( mCentralContainer )
    mCentralContainer->setCurrentIndex( 0 );
}

void QgisApp::markDirty()
{
  // notify the project that there was a change
  QgsProject::instance()->setDirty( true );
}

void QgisApp::extentChanged()
{
  // allow symbols in the legend update their preview if they use map units
  mLayerTreeView->layerTreeModel()->setLegendMapViewData( mMapCanvas->mapUnitsPerPixel(), mMapCanvas->mapSettings().outputDpi(), mMapCanvas->scale() );
}

void QgisApp::layersWereAdded( const QList<QgsMapLayer *> &layers )
{
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    QgsDataProvider *provider = nullptr;

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer )
    {
      // notify user about any font family substitution, but only when rendering labels (i.e. not when opening settings dialog)
      connect( vlayer, &QgsVectorLayer::labelingFontNotFound, this, &QgisApp::labelingFontNotFound );

      QgsVectorDataProvider *vProvider = vlayer->dataProvider();
      // Do not check for layer editing capabilities because they may change
      // (for example when subsetString is added/removed) and signals need to
      // be in place in order to update the GUI
      connect( vlayer, &QgsVectorLayer::layerModified, this, &QgisApp::updateLayerModifiedActions );
      connect( vlayer, &QgsVectorLayer::editingStarted, this, &QgisApp::layerEditStateChanged );
      connect( vlayer, &QgsVectorLayer::editingStopped, this, &QgisApp::layerEditStateChanged );
      connect( vlayer, &QgsVectorLayer::readOnlyChanged, this, &QgisApp::layerEditStateChanged );
      connect( vlayer, &QgsVectorLayer::raiseError, this, &QgisApp::onLayerError );

      provider = vProvider;
    }

    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
    if ( rlayer )
    {
      // connect up any request the raster may make to update the statusbar message
      connect( rlayer, &QgsRasterLayer::statusChanged, this, &QgisApp::showStatusMessage );

      provider = rlayer->dataProvider();
    }

    if ( provider )
    {
      connect( provider, &QgsDataProvider::dataChanged, layer, [layer] { layer->triggerRepaint(); } );
      connect( provider, &QgsDataProvider::dataChanged, this, &QgisApp::refreshMapCanvas );
    }
  }
}

void QgisApp::showRotation()
{
  // update the statusbar with the current rotation.
  double myrotation = mMapCanvas->rotation();
  mRotationEdit->setValue( myrotation );
} // QgisApp::showRotation


void QgisApp::updateMouseCoordinatePrecision()
{
  mCoordsEdit->setMouseCoordinatesPrecision( QgsCoordinateUtils::calculateCoordinatePrecision( mapCanvas()->mapUnitsPerPixel(), mapCanvas()->mapSettings().destinationCrs() ) );
}

void QgisApp::showStatusMessage( const QString &message )
{
  mStatusBar->showMessage( message );
}

void QgisApp::displayMapToolMessage( const QString &message, QgsMessageBar::MessageLevel level )
{
  // remove previous message
  messageBar()->popWidget( mLastMapToolMessage );

  QgsMapTool *tool = mapCanvas()->mapTool();

  if ( tool )
  {
    mLastMapToolMessage = new QgsMessageBarItem( tool->toolName(), message, level, messageTimeout() );
    messageBar()->pushItem( mLastMapToolMessage );
  }
}

void QgisApp::displayMessage( const QString &title, const QString &message, QgsMessageBar::MessageLevel level )
{
  messageBar()->pushMessage( title, message, level, messageTimeout() );
}

void QgisApp::removeMapToolMessage()
{
  // remove previous message
  messageBar()->popWidget( mLastMapToolMessage );
}


// Show the maptip using tooltip
void QgisApp::showMapTip()
{
  QPoint myPointerPos = mMapCanvas->mouseLastXY();

  //  Make sure there is an active layer before proceeding
  QgsMapLayer *mypLayer = mMapCanvas->currentLayer();
  if ( mypLayer )
  {
    //QgsDebugMsg("Current layer for maptip display is: " + mypLayer->source());
    // only process vector layers
    if ( mypLayer->type() == QgsMapLayer::VectorLayer )
    {
      // Show the maptip if the maptips button is depressed
      if ( mMapTipsVisible )
      {
        mpMaptip->showMapTip( mypLayer, mLastMapPosition, myPointerPos, mMapCanvas );
      }
    }
  }
}

void QgisApp::projectPropertiesProjections()
{
  // Driver to display the project props dialog and switch to the
  // projections tab
  mShowProjectionTab = true;
  projectProperties();
}

void QgisApp::projectProperties()
{
  /* Display the property sheet for the Project */
  // set wait cursor since construction of the project properties
  // dialog results in the construction of the spatial reference
  // system QMap
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QgsProjectProperties *pp = new QgsProjectProperties( mMapCanvas, this );
  // if called from the status bar, show the projection tab
  if ( mShowProjectionTab )
  {
    pp->showProjectionsTab();
    mShowProjectionTab = false;
  }
  qApp->processEvents();
  // Be told if the mouse display precision may have changed by the user
  // changing things in the project properties dialog box
  connect( pp, &QgsProjectProperties::displayPrecisionChanged, this,
           &QgisApp::updateMouseCoordinatePrecision );

  connect( pp, &QgsProjectProperties::scalesChanged, mScaleWidget,
           &QgsStatusBarScaleWidget::updateScales );
  QApplication::restoreOverrideCursor();

  // Display the modal dialog box.
  pp->exec();

  qobject_cast<QgsMeasureTool *>( mMapTools.mMeasureDist )->updateSettings();
  qobject_cast<QgsMeasureTool *>( mMapTools.mMeasureArea )->updateSettings();
  qobject_cast<QgsMapToolMeasureAngle *>( mMapTools.mMeasureAngle )->updateSettings();

  // Set the window title.
  setTitleBarText_( *this );

  // delete the property sheet object
  delete pp;
}


QgsClipboard *QgisApp::clipboard()
{
  return mInternalClipboard;
}

void QgisApp::selectionChanged( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
  {
    showStatusMessage( tr( "%n feature(s) selected on layer %1.", "number of selected features", vlayer->selectedFeatureCount() ).arg( vlayer->name() ) );
  }
  if ( layer == activeLayer() )
  {
    activateDeactivateLayerRelatedActions( layer );
  }
}

void QgisApp::legendLayerSelectionChanged()
{
  QList<QgsLayerTreeLayer *> selectedLayers = mLayerTreeView ? mLayerTreeView->selectedLayerNodes() : QList<QgsLayerTreeLayer *>();

  mActionDuplicateLayer->setEnabled( !selectedLayers.isEmpty() );
  mActionSetLayerScaleVisibility->setEnabled( !selectedLayers.isEmpty() );
  mActionSetLayerCRS->setEnabled( !selectedLayers.isEmpty() );
  mActionSetProjectCRSFromLayer->setEnabled( selectedLayers.count() == 1 );

  mActionSaveEdits->setEnabled( QgsLayerTreeUtils::layersModified( selectedLayers ) );
  mActionRollbackEdits->setEnabled( QgsLayerTreeUtils::layersModified( selectedLayers ) );
  mActionCancelEdits->setEnabled( QgsLayerTreeUtils::layersEditable( selectedLayers ) );

  mLegendExpressionFilterButton->setEnabled( false );
  mLegendExpressionFilterButton->setVectorLayer( nullptr );
  if ( selectedLayers.size() == 1 )
  {
    QgsLayerTreeLayer *l = selectedLayers.front();
    if ( l->layer() && l->layer()->type() == QgsMapLayer::VectorLayer )
    {
      mLegendExpressionFilterButton->setEnabled( true );
      bool exprEnabled;
      QString expr = QgsLayerTreeUtils::legendFilterByExpression( *l, &exprEnabled );
      mLegendExpressionFilterButton->setExpressionText( expr );
      mLegendExpressionFilterButton->setVectorLayer( qobject_cast<QgsVectorLayer *>( l->layer() ) );
      mLegendExpressionFilterButton->setChecked( exprEnabled );
    }
  }
}

void QgisApp::layerEditStateChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( layer && layer == activeLayer() )
  {
    activateDeactivateLayerRelatedActions( layer );
    mSaveRollbackInProgress = false;
  }
}

void QgisApp::updateLabelToolButtons()
{
  bool enableMove = false, enableRotate = false, enablePin = false, enableShowHide = false, enableChange = false;

  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( vlayer && ( vlayer->diagramsEnabled() || vlayer->labelsEnabled() ) )
    {
      enablePin = true;
      enableShowHide = true;
      enableMove = true;
      enableRotate = true;
      enableChange = true;

      break;
    }
  }

  mActionPinLabels->setEnabled( enablePin );
  mActionShowHideLabels->setEnabled( enableShowHide );
  mActionMoveLabel->setEnabled( enableMove );
  mActionRotateLabel->setEnabled( enableRotate );
  mActionChangeLabelProperties->setEnabled( enableChange );
}

void QgisApp::activateDeactivateLayerRelatedActions( QgsMapLayer *layer )
{
  updateLabelToolButtons();

  mMenuPasteAs->setEnabled( clipboard() && !clipboard()->isEmpty() );
  mActionPasteAsNewVector->setEnabled( clipboard() && !clipboard()->isEmpty() );
  mActionPasteAsNewMemoryVector->setEnabled( clipboard() && !clipboard()->isEmpty() );

  updateLayerModifiedActions();

  if ( !layer )
  {
    mActionSelectFeatures->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionIdentify->setEnabled( QgsSettings().value( QStringLiteral( "/Map/identifyMode" ), 0 ).toInt() != 0 );
    mActionSelectByExpression->setEnabled( false );
    mActionSelectByForm->setEnabled( false );
    mActionLabeling->setEnabled( false );
    mActionOpenTable->setEnabled( false );
    mActionSelectAll->setEnabled( false );
    mActionInvertSelection->setEnabled( false );
    mActionOpenFieldCalc->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionToggleEditing->setChecked( false );
    mActionSaveLayerEdits->setEnabled( false );
    mActionSaveLayerDefinition->setEnabled( false );
    mActionLayerSaveAs->setEnabled( false );
    mActionLayerProperties->setEnabled( false );
    mActionLayerSubsetString->setEnabled( false );
    mActionAddToOverview->setEnabled( false );
    mActionFeatureAction->setEnabled( false );
    mActionAddFeature->setEnabled( false );
    mActionCircularStringCurvePoint->setEnabled( false );
    mActionCircularStringRadius->setEnabled( false );
    mActionCircle2Points->setEnabled( false );
    mActionCircle3Points->setEnabled( false );
    mActionCircle3Tangents->setEnabled( false );
    mActionCircle2TangentsPoint->setEnabled( false );
    mActionCircleCenterPoint->setEnabled( false );
    mActionEllipseCenter2Points->setEnabled( false );
    mActionEllipseCenterPoint->setEnabled( false );
    mActionEllipseExtent->setEnabled( false );
    mActionEllipseFoci->setEnabled( false );
    mActionRectangleCenterPoint->setEnabled( false );
    mActionRectangleExtent->setEnabled( false );
    mActionRegularPolygon2Points->setEnabled( false );
    mActionRegularPolygonCenterPoint->setEnabled( false );
    mActionRegularPolygonCenterCorner->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionMoveFeatureCopy->setEnabled( false );
    mActionRotateFeature->setEnabled( false );
    mActionOffsetCurve->setEnabled( false );
    mActionNodeTool->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );
    mActionCopyStyle->setEnabled( false );
    mActionPasteStyle->setEnabled( false );

    mUndoDock->widget()->setEnabled( false );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionFillRing->setEnabled( false );
    mActionAddPart->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );
    mActionSplitParts->setEnabled( false );
    mActionMergeFeatures->setEnabled( false );
    mActionMergeFeatureAttributes->setEnabled( false );
    mActionMultiEditAttributes->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );
    mActionOffsetPointSymbol->setEnabled( false );

    mActionPinLabels->setEnabled( false );
    mActionShowHideLabels->setEnabled( false );
    mActionMoveLabel->setEnabled( false );
    mActionRotateLabel->setEnabled( false );
    mActionChangeLabelProperties->setEnabled( false );

    mActionDiagramProperties->setEnabled( false );

    mActionLocalHistogramStretch->setEnabled( false );
    mActionFullHistogramStretch->setEnabled( false );
    mActionLocalCumulativeCutStretch->setEnabled( false );
    mActionFullCumulativeCutStretch->setEnabled( false );
    mActionIncreaseBrightness->setEnabled( false );
    mActionDecreaseBrightness->setEnabled( false );
    mActionIncreaseContrast->setEnabled( false );
    mActionDecreaseContrast->setEnabled( false );
    mActionZoomActualSize->setEnabled( false );
    mActionZoomToLayer->setEnabled( false );
    return;
  }

  mActionLayerProperties->setEnabled( QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() );
  mActionAddToOverview->setEnabled( true );
  mActionZoomToLayer->setEnabled( true );

  mActionCopyStyle->setEnabled( true );
  mActionPasteStyle->setEnabled( clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) );

  /***********Vector layers****************/
  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    QgsVectorDataProvider *dprovider = vlayer->dataProvider();
    QString addFeatureText;

    bool isEditable = vlayer->isEditable();
    bool layerHasSelection = vlayer->selectedFeatureCount() > 0;
    bool layerHasActions = !vlayer->actions()->actions( QStringLiteral( "Canvas" ) ).isEmpty() || !QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer ).isEmpty();
    bool isSpatial = vlayer->isSpatial();

    mActionLocalHistogramStretch->setEnabled( false );
    mActionFullHistogramStretch->setEnabled( false );
    mActionLocalCumulativeCutStretch->setEnabled( false );
    mActionFullCumulativeCutStretch->setEnabled( false );
    mActionIncreaseBrightness->setEnabled( false );
    mActionDecreaseBrightness->setEnabled( false );
    mActionIncreaseContrast->setEnabled( false );
    mActionDecreaseContrast->setEnabled( false );
    mActionZoomActualSize->setEnabled( false );
    mActionZoomToLayer->setEnabled( isSpatial );
    mActionZoomToSelected->setEnabled( isSpatial );
    mActionLabeling->setEnabled( isSpatial );
    mActionDiagramProperties->setEnabled( isSpatial );

    mActionSelectFeatures->setEnabled( isSpatial );
    mActionSelectPolygon->setEnabled( isSpatial );
    mActionSelectFreehand->setEnabled( isSpatial );
    mActionSelectRadius->setEnabled( isSpatial );
    mActionIdentify->setEnabled( isSpatial );
    mActionSelectByExpression->setEnabled( true );
    mActionSelectByForm->setEnabled( true );
    mActionOpenTable->setEnabled( true );
    mActionSelectAll->setEnabled( true );
    mActionInvertSelection->setEnabled( true );
    mActionSaveLayerDefinition->setEnabled( true );
    mActionLayerSaveAs->setEnabled( true );
    mActionCopyFeatures->setEnabled( layerHasSelection );
    mActionFeatureAction->setEnabled( layerHasActions );

    if ( !isEditable && mMapCanvas && mMapCanvas->mapTool()
         && ( mMapCanvas->mapTool()->flags() & QgsMapTool::EditTool ) && !mSaveRollbackInProgress )
    {
      mMapCanvas->setMapTool( mNonEditMapTool );
    }

    if ( dprovider )
    {
      bool canChangeAttributes = dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
      bool canDeleteFeatures = dprovider->capabilities() & QgsVectorDataProvider::DeleteFeatures;
      bool canAddFeatures = dprovider->capabilities() & QgsVectorDataProvider::AddFeatures;
      bool canSupportEditing = dprovider->capabilities() & QgsVectorDataProvider::EditingCapabilities;
      bool canChangeGeometry = isSpatial && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries;

      mActionLayerSubsetString->setEnabled( !isEditable && dprovider->supportsSubsetString() );

      mActionToggleEditing->setEnabled( canSupportEditing && !vlayer->readOnly() );
      mActionToggleEditing->setChecked( canSupportEditing && isEditable );
      mActionSaveLayerEdits->setEnabled( canSupportEditing && isEditable && vlayer->isModified() );
      mUndoDock->widget()->setEnabled( canSupportEditing && isEditable );
      mActionUndo->setEnabled( canSupportEditing );
      mActionRedo->setEnabled( canSupportEditing );

      //start editing/stop editing
      if ( canSupportEditing )
      {
        updateUndoActions();
      }

      mActionPasteFeatures->setEnabled( isEditable && canAddFeatures && !clipboard()->isEmpty() );

      mActionAddFeature->setEnabled( isEditable && canAddFeatures );

      bool enableCircularTools;
      bool enableShapeTools;
      enableCircularTools = isEditable && ( canAddFeatures || canChangeGeometry )
                            && ( vlayer->geometryType() == QgsWkbTypes::LineGeometry || vlayer->geometryType() == QgsWkbTypes::PolygonGeometry );
      enableShapeTools = enableCircularTools;
      mActionCircularStringCurvePoint->setEnabled( enableCircularTools );
      mActionCircularStringRadius->setEnabled( enableCircularTools );
      mActionCircle2Points->setEnabled( enableShapeTools );
      mActionCircle3Points->setEnabled( enableShapeTools );
      mActionCircle3Tangents->setEnabled( enableShapeTools );
      mActionCircle2TangentsPoint->setEnabled( enableShapeTools );
      mActionCircleCenterPoint->setEnabled( enableShapeTools );
      mActionEllipseCenter2Points->setEnabled( enableShapeTools );
      mActionEllipseCenterPoint->setEnabled( enableShapeTools );
      mActionEllipseExtent->setEnabled( enableShapeTools );
      mActionEllipseFoci->setEnabled( enableShapeTools );
      mActionRectangleCenterPoint->setEnabled( enableShapeTools );
      mActionRectangleExtent->setEnabled( enableShapeTools );
      mActionRectangle3Points->setEnabled( enableShapeTools );
      mActionRegularPolygon2Points->setEnabled( enableShapeTools );
      mActionRegularPolygonCenterPoint->setEnabled( enableShapeTools );
      mActionRegularPolygonCenterCorner->setEnabled( enableShapeTools );

      //does provider allow deleting of features?
      mActionDeleteSelected->setEnabled( isEditable && canDeleteFeatures && layerHasSelection );
      mActionCutFeatures->setEnabled( isEditable && canDeleteFeatures && layerHasSelection );

      //merge tool needs editable layer and provider with the capability of adding and deleting features
      if ( isEditable && canChangeAttributes )
      {
        mActionMergeFeatures->setEnabled( layerHasSelection && canDeleteFeatures && canAddFeatures );
        mActionMergeFeatureAttributes->setEnabled( layerHasSelection );
        mActionMultiEditAttributes->setEnabled( layerHasSelection );
      }
      else
      {
        mActionMergeFeatures->setEnabled( false );
        mActionMergeFeatureAttributes->setEnabled( false );
        mActionMultiEditAttributes->setEnabled( false );
      }

      bool isMultiPart = QgsWkbTypes::isMultiType( vlayer->wkbType() ) || !dprovider->doesStrictFeatureTypeCheck();

      // moving enabled if geometry changes are supported
      mActionAddPart->setEnabled( isEditable && canChangeGeometry );
      mActionDeletePart->setEnabled( isEditable && canChangeGeometry );
      mActionMoveFeature->setEnabled( isEditable && canChangeGeometry );
      mActionMoveFeatureCopy->setEnabled( isEditable && canChangeGeometry );
      mActionRotateFeature->setEnabled( isEditable && canChangeGeometry );
      mActionNodeTool->setEnabled( isEditable && canChangeGeometry );

      if ( vlayer->geometryType() == QgsWkbTypes::PointGeometry )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePoint.svg" ) ) );
        addFeatureText = tr( "Add Point Feature" );
        mActionMoveFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeaturePoint.svg" ) ) );
        mActionMoveFeatureCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeatureCopyPoint.svg" ) ) );

        mActionAddRing->setEnabled( false );
        mActionFillRing->setEnabled( false );
        mActionReshapeFeatures->setEnabled( false );
        mActionSplitFeatures->setEnabled( false );
        mActionSplitParts->setEnabled( false );
        mActionSimplifyFeature->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
        mActionRotatePointSymbols->setEnabled( false );
        mActionOffsetPointSymbol->setEnabled( false );
        mActionOffsetCurve->setEnabled( false );

        if ( isEditable && canChangeAttributes )
        {
          if ( QgsMapToolRotatePointSymbols::layerIsRotatable( vlayer ) )
          {
            mActionRotatePointSymbols->setEnabled( true );
          }
          if ( QgsMapToolOffsetPointSymbol::layerIsOffsetable( vlayer ) )
          {
            mActionOffsetPointSymbol->setEnabled( true );
          }
        }
      }
      else if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCaptureLine.svg" ) ) );
        addFeatureText = tr( "Add Line Feature" );
        mActionMoveFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeatureLine.svg" ) ) );
        mActionMoveFeatureCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeatureCopyLine.svg" ) ) );

        mActionReshapeFeatures->setEnabled( isEditable && canChangeGeometry );
        mActionSplitFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSplitParts->setEnabled( isEditable && canChangeGeometry && isMultiPart );
        mActionSimplifyFeature->setEnabled( isEditable && canChangeGeometry );
        mActionOffsetCurve->setEnabled( isEditable && canAddFeatures && canChangeAttributes );

        mActionAddRing->setEnabled( false );
        mActionFillRing->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
      }
      else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePolygon.svg" ) ) );
        addFeatureText = tr( "Add Polygon Feature" );
        mActionMoveFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeature.svg" ) ) );
        mActionMoveFeatureCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMoveFeatureCopy.svg" ) ) );

        mActionAddRing->setEnabled( isEditable && canChangeGeometry );
        mActionFillRing->setEnabled( isEditable && canChangeGeometry );
        mActionReshapeFeatures->setEnabled( isEditable && canChangeGeometry );
        mActionSplitFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSplitParts->setEnabled( isEditable && canChangeGeometry && isMultiPart );
        mActionSimplifyFeature->setEnabled( isEditable && canChangeGeometry );
        mActionDeleteRing->setEnabled( isEditable && canChangeGeometry );
        mActionOffsetCurve->setEnabled( isEditable && canAddFeatures && canChangeAttributes );
      }
      else if ( vlayer->geometryType() == QgsWkbTypes::NullGeometry )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewTableRow.svg" ) ) );
        addFeatureText = tr( "Add Record" );
        mActionAddRing->setEnabled( false );
        mActionFillRing->setEnabled( false );
        mActionReshapeFeatures->setEnabled( false );
        mActionSplitFeatures->setEnabled( false );
        mActionSplitParts->setEnabled( false );
        mActionSimplifyFeature->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
        mActionOffsetCurve->setEnabled( false );
      }

      mActionOpenFieldCalc->setEnabled( true );
      mActionAddFeature->setText( addFeatureText );
      mActionAddFeature->setToolTip( addFeatureText );
      QgsGui::shortcutsManager()->unregisterAction( mActionAddFeature );
      QgsGui::shortcutsManager()->registerAction( mActionAddFeature, mActionAddFeature->shortcut() );
    }
    else
    {
      mUndoDock->widget()->setEnabled( false );
      mActionUndo->setEnabled( false );
      mActionRedo->setEnabled( false );
      mActionLayerSubsetString->setEnabled( false );
    }
  } //end vector layer block
  /*************Raster layers*************/
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( layer );
    if ( rlayer->dataProvider()->dataType( 1 ) != Qgis::ARGB32
         && rlayer->dataProvider()->dataType( 1 ) != Qgis::ARGB32_Premultiplied )
    {
      if ( rlayer->dataProvider()->capabilities() & QgsRasterDataProvider::Size )
      {
        mActionFullHistogramStretch->setEnabled( true );
      }
      else
      {
        // it would hang up reading the data for WMS for example
        mActionFullHistogramStretch->setEnabled( false );
      }
      mActionLocalHistogramStretch->setEnabled( true );
    }
    else
    {
      mActionLocalHistogramStretch->setEnabled( false );
      mActionFullHistogramStretch->setEnabled( false );
    }

    mActionLocalCumulativeCutStretch->setEnabled( true );
    mActionFullCumulativeCutStretch->setEnabled( true );
    mActionIncreaseBrightness->setEnabled( true );
    mActionDecreaseBrightness->setEnabled( true );
    mActionIncreaseContrast->setEnabled( true );
    mActionDecreaseContrast->setEnabled( true );

    mActionLayerSubsetString->setEnabled( false );
    mActionFeatureAction->setEnabled( false );
    mActionSelectFeatures->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionZoomActualSize->setEnabled( true );
    mActionZoomToLayer->setEnabled( true );
    mActionZoomToSelected->setEnabled( false );
    mActionOpenTable->setEnabled( false );
    mActionSelectAll->setEnabled( false );
    mActionInvertSelection->setEnabled( false );
    mActionSelectByExpression->setEnabled( false );
    mActionSelectByForm->setEnabled( false );
    mActionOpenFieldCalc->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionToggleEditing->setChecked( false );
    mActionSaveLayerEdits->setEnabled( false );
    mUndoDock->widget()->setEnabled( false );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
    mActionSaveLayerDefinition->setEnabled( true );
    mActionLayerSaveAs->setEnabled( true );
    mActionAddFeature->setEnabled( false );
    mActionCircularStringCurvePoint->setEnabled( false );
    mActionCircularStringRadius->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionFillRing->setEnabled( false );
    mActionAddPart->setEnabled( false );
    mActionNodeTool->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionMoveFeatureCopy->setEnabled( false );
    mActionRotateFeature->setEnabled( false );
    mActionOffsetCurve->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );
    mActionOffsetPointSymbol->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );
    mActionSplitParts->setEnabled( false );
    mActionLabeling->setEnabled( false );
    mActionDiagramProperties->setEnabled( false );

    //NOTE: This check does not really add any protection, as it is called on load not on layer select/activate
    //If you load a layer with a provider and idenitfy ability then load another without, the tool would be disabled for both

    //Enable the Identify tool ( GDAL datasets draw without a provider )
    //but turn off if data provider exists and has no Identify capabilities
    mActionIdentify->setEnabled( true );

    QgsSettings settings;
    int identifyMode = settings.value( QStringLiteral( "Map/identifyMode" ), 0 ).toInt();
    if ( identifyMode == 0 )
    {
      const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( layer );
      const QgsRasterDataProvider *dprovider = rlayer->dataProvider();
      if ( dprovider )
      {
        // does provider allow the identify map tool?
        if ( dprovider->capabilities() & QgsRasterDataProvider::Identify )
        {
          mActionIdentify->setEnabled( true );
        }
        else
        {
          mActionIdentify->setEnabled( false );
        }
      }
    }
  }

  refreshFeatureActions();
}

void QgisApp::refreshActionFeatureAction()
{
  mActionFeatureAction->setEnabled( false );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  bool layerHasActions = !vlayer->actions()->actions( QStringLiteral( "Canvas" ) ).isEmpty() || !QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer ).isEmpty();
  mActionFeatureAction->setEnabled( layerHasActions );
}

void QgisApp::renameView()
{
  QgsMapCanvasDockWidget *view = qobject_cast< QgsMapCanvasDockWidget * >( sender() );
  if ( !view )
    return;

  // calculate existing names
  QStringList names;
  Q_FOREACH ( QgsMapCanvas *c, mapCanvases() )
  {
    if ( c == view->mapCanvas() )
      continue;

    names << c->objectName();
  }

  QString currentName = view->mapCanvas()->objectName();

  QgsNewNameDialog renameDlg( currentName, currentName, QStringList(), names, QRegExp(), Qt::CaseSensitive, this );
  renameDlg.setWindowTitle( tr( "Map Views" ) );
  //renameDlg.setHintString( tr( "Name of the new view" ) );
  renameDlg.setOverwriteEnabled( false );
  renameDlg.setConflictingNameWarning( tr( "A view with this name already exists" ) );
  if ( renameDlg.exec() || renameDlg.name().isEmpty() )
  {
    QString newName = renameDlg.name();
    view->setWindowTitle( newName );
    view->mapCanvas()->setObjectName( newName );
  }
}

/////////////////////////////////////////////////////////////////
//
//
// Only functions relating to raster layer management in this
// section (look for a similar comment block to this to find
// the end of this section).
//
// Tim Sutton
//
//
/////////////////////////////////////////////////////////////////


// this is a slot for action from GUI to open and add raster layers
void QgisApp::addRasterLayer()
{
  QStringList selectedFiles;
  QString e;//only for parameter correctness
  QString title = tr( "Open a GDAL Supported Raster Data Source" );
  QgsGuiUtils::openFilesRememberingFilter( QStringLiteral( "lastRasterFileFilter" ), mRasterFileFilter, selectedFiles, e,
      title );

  if ( selectedFiles.isEmpty() )
  {
    // no files were selected, so just bail
    return;
  }

  addRasterLayers( selectedFiles );

}

//
// This is the method that does the actual work of adding a raster layer - the others
// here simply create a raster layer object and delegate here. It is the responsibility
// of the calling method to manage things such as the frozen state of the mapcanvas and
// using waitcursors etc. - this method won't and SHOULDN'T do it
//
bool QgisApp::addRasterLayer( QgsRasterLayer *rasterLayer )
{
  Q_CHECK_PTR( rasterLayer );

  if ( ! rasterLayer )
  {
    // XXX insert meaningful whine to the user here; although be
    // XXX mindful that a null layer may mean exhausted memory resources
    return false;
  }

  if ( !rasterLayer->isValid() )
  {
    delete rasterLayer;
    return false;
  }

  // register this layer with the central layers registry
  QList<QgsMapLayer *> myList;
  myList << rasterLayer;
  QgsProject::instance()->addMapLayers( myList );

  return true;
}


// Open a raster layer - this is the generic function which takes all parameters
// this method is a blend of addRasterLayer() functions (with and without provider)
// and addRasterLayers()
QgsRasterLayer *QgisApp::addRasterLayerPrivate(
  const QString &uri, const QString &name, const QString &providerKey,
  bool guiWarning, bool guiUpdate )
{
  if ( guiUpdate )
  {
    // let the user know we're going to possibly be taking a while
    // QApplication::setOverrideCursor( Qt::WaitCursor );
    freezeCanvases();
  }

  QString baseName =  QgsMapLayer::formatLayerName( name );

  QgsDebugMsg( "Creating new raster layer using " + uri
               + " with baseName of " + baseName );

  QgsRasterLayer *layer = nullptr;
  // XXX ya know QgsRasterLayer can snip out the basename on its own;
  // XXX why do we have to pass it in for it?
  // ET : we may not be getting "normal" files here, so we still need the baseName argument
  if ( !providerKey.isEmpty() && uri.endsWith( QLatin1String( ".adf" ), Qt::CaseInsensitive ) )
  {
    QFileInfo fileInfo( uri );
    QString dirName = fileInfo.path();
    layer = new QgsRasterLayer( dirName, QFileInfo( dirName ).completeBaseName(), QStringLiteral( "gdal" ) );
  }
  else if ( providerKey.isEmpty() )
    layer = new QgsRasterLayer( uri, baseName ); // fi.completeBaseName());
  else
    layer = new QgsRasterLayer( uri, baseName, providerKey );

  QgsDebugMsg( "Constructed new layer" );

  QgsError error;
  QString title;
  bool ok = false;

  if ( !layer->isValid() )
  {
    error = layer->error();
    title = tr( "Invalid Layer" );

    if ( shouldAskUserForGDALSublayers( layer ) )
    {
      askUserForGDALSublayers( layer );
      ok = true;

      // The first layer loaded is not useful in that case. The user can select it in
      // the list if he wants to load it.
      delete layer;
      layer = nullptr;
    }
  }
  else
  {
    ok = addRasterLayer( layer );
    if ( !ok )
    {
      error.append( QGS_ERROR_MESSAGE( tr( "Error adding valid layer to map canvas" ),
                                       tr( "Raster layer" ) ) );
      title = tr( "Error" );
    }
  }

  if ( !ok )
  {
    if ( guiUpdate )
      freezeCanvases( false );

    // don't show the gui warning if we are loading from command line
    if ( guiWarning )
    {
      messageBar()->pushMessage( title, error.message( QgsErrorMessage::Text ),
                                 QgsMessageBar::CRITICAL, messageTimeout() );
    }

    if ( layer )
    {
      delete layer;
      layer = nullptr;
    }
  }

  if ( guiUpdate )
  {
    // draw the map
    freezeCanvases( false );
    refreshMapCanvas();
    // Let render() do its own cursor management
    //    QApplication::restoreOverrideCursor();
  }
  layer->loadDefaultMetadata( ok );
  return layer;

} // QgisApp::addRasterLayer


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
QgsRasterLayer *QgisApp::addRasterLayer(
  QString const &rasterFile, QString const &baseName, bool guiWarning )
{
  return addRasterLayerPrivate( rasterFile, baseName, QString(), guiWarning, true );
}


QgsRasterLayer *QgisApp::addRasterLayer(
  QString const &uri, QString const &baseName, QString const &providerKey )
{
  return addRasterLayerPrivate( uri, baseName, providerKey, true, true );
}


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool QgisApp::addRasterLayers( QStringList const &fileNameQStringList, bool guiWarning )
{
  if ( fileNameQStringList.empty() )
  {
    // no files selected so bail out, but
    // allow mMapCanvas to handle events
    // first
    freezeCanvases( false );;
    return false;
  }

  freezeCanvases();

  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for ( QStringList::ConstIterator myIterator = fileNameQStringList.begin();
        myIterator != fileNameQStringList.end();
        ++myIterator )
  {
    QString errMsg;
    bool ok = false;

    // if needed prompt for zipitem layers
    QString vsiPrefix = QgsZipItem::vsiPrefix( *myIterator );
    if ( ! myIterator->startsWith( QLatin1String( "/vsi" ), Qt::CaseInsensitive ) &&
         ( vsiPrefix == QLatin1String( "/vsizip/" ) || vsiPrefix == QLatin1String( "/vsitar/" ) ) )
    {
      if ( askUserForZipItemLayers( *myIterator ) )
        continue;
    }

    if ( QgsRasterLayer::isValidRasterFileName( *myIterator, errMsg ) )
    {
      QFileInfo myFileInfo( *myIterator );

      // try to create the layer
      QgsRasterLayer *layer = addRasterLayerPrivate( *myIterator, myFileInfo.completeBaseName(),
                              QString(), guiWarning, true );
      if ( layer && layer->isValid() )
      {
        //only allow one copy of a ai grid file to be loaded at a
        //time to prevent the user selecting all adfs in 1 dir which
        //actually represent 1 coverate,

        if ( myFileInfo.fileName().toLower().endsWith( QLatin1String( ".adf" ) ) )
        {
          break;
        }
      }
      // if layer is invalid addRasterLayerPrivate() will show the error

    } // valid raster filename
    else
    {
      ok = false;

      // Issue message box warning unless we are loading from cmd line since
      // non-rasters are passed to this function first and then successfully
      // loaded afterwards (see main.cpp)
      if ( guiWarning )
      {
        QString msg = tr( "%1 is not a supported raster data source" ).arg( *myIterator );
        if ( !errMsg.isEmpty() )
          msg += '\n' + errMsg;

        messageBar()->pushMessage( tr( "Unsupported Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
      }
    }
    if ( ! ok )
    {
      returnValue = false;
    }
  }

  freezeCanvases( false );
  refreshMapCanvas();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  return returnValue;

}// QgisApp::addRasterLayer()




///////////////////////////////////////////////////////////////////
//
//
//
//
//    RASTER ONLY RELATED FUNCTIONS BLOCK ENDS....
//
//
//
//
///////////////////////////////////////////////////////////////////


QgsPluginLayer *QgisApp::addPluginLayer( const QString &uri, const QString &baseName, const QString &providerKey )
{
  QgsPluginLayer *layer = QgsApplication::pluginLayerRegistry()->createLayer( providerKey, uri );
  if ( !layer )
    return nullptr;

  layer->setName( baseName );

  QgsProject::instance()->addMapLayer( layer );

  return layer;
}



#ifdef ANDROID
void QgisApp::keyReleaseEvent( QKeyEvent *event )
{
  static bool sAccepted = true;
  if ( event->key() == Qt::Key_Close )
  {
    // do something useful here
    int ret = QMessageBox::question( this, tr( "Exit QGIS" ),
                                     tr( "Do you really want to quit QGIS?" ),
                                     QMessageBox::Yes | QMessageBox::No );
    switch ( ret )
    {
      case QMessageBox::Yes:
        this->close();
        break;

      case QMessageBox::No:
        break;
    }
    event->setAccepted( sAccepted ); // don't close my Top Level Widget !
    sAccepted = false;// close the app next time when the user press back button
  }
  else
  {
    QMainWindow::keyReleaseEvent( event );
  }
}
#endif

void QgisApp::keyPressEvent( QKeyEvent *e )
{
  // The following statement causes a crash on WIN32 and should be
  // enclosed in an #ifdef QGISDEBUG if its really necessary. Its
  // commented out for now. [gsherman]
  // QgsDebugMsg( QString( "%1 (keypress received)" ).arg( e->text() ) );
  emit keyPressed( e );

  //cancel rendering progress with esc key
  if ( e->key() == Qt::Key_Escape )
  {
    stopRendering();
  }
#if defined(_MSC_VER) && defined(QGISDEBUG)
  else if ( e->key() == Qt::Key_Backslash && e->modifiers() & Qt::ControlModifier )
  {
    abort();
  }
#endif
  else
  {
    e->ignore();
  }
}

void QgisApp::newProfile()
{
  QString text = QInputDialog::getText( this, tr( "New profile name" ), tr( "New profile name" ) );
  if ( text.isEmpty() )
    return;

  userProfileManager()->createUserProfile( text );
  userProfileManager()->loadUserProfile( text );
}

void QgisApp::onTaskCompleteShowNotify( long taskId, int status )
{
  if ( status == QgsTask::Complete && !this->isActiveWindow() )
  {
    QgsTask *task = QgsApplication::taskManager()->task( taskId );
    if ( task )
    {
      showSystemNotification( tr( "Task complete" ), task->description() );
    }
  }
}

void QgisApp::onTransactionGroupsChanged()
{
  const auto groups = QgsProject::instance()->transactionGroups();
  for ( auto it = groups.constBegin(); it != groups.constEnd(); ++it )
  {
    connect( it.value(), &QgsTransactionGroup::commitError, this, &QgisApp::transactionGroupCommitError, Qt::UniqueConnection );
  }
}

void QgisApp::onSnappingConfigChanged()
{
  mSnappingUtils->setConfig( QgsProject::instance()->snappingConfig() );
}

void QgisApp::startProfile( const QString &name )
{
  QgsApplication::profiler()->start( name );
}

void QgisApp::endProfile()
{
  QgsApplication::profiler()->end();
}

void QgisApp::functionProfile( void ( QgisApp::*fnc )(), QgisApp *instance, const QString &name )
{
  startProfile( name );
  ( instance->*fnc )();
  endProfile();
}

void QgisApp::mapCanvas_keyPressed( QKeyEvent *e )
{
  // Delete selected features when it is possible and KeyEvent was not managed by current MapTool
  if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) && e->isAccepted() )
  {
    deleteSelected();
  }
}

void QgisApp::customProjection()
{
  // Create an instance of the Custom Projection Designer modeless dialog.
  // Autodelete the dialog when closing since a pointer is not retained.
  QgsCustomProjectionDialog *myDialog = new QgsCustomProjectionDialog( this );
  myDialog->setAttribute( Qt::WA_DeleteOnClose );
  myDialog->show();
}

void QgisApp::newBookmark()
{
  showBookmarks();
  mBookMarksDockWidget->addClicked();
}

void QgisApp::showBookmarks()
{
  mBookMarksDockWidget->show();
  mBookMarksDockWidget->raise();
}

// Slot that gets called when the project file was saved with an older
// version of QGIS

void QgisApp::oldProjectVersionWarning( const QString &oldVersion )
{
  Q_UNUSED( oldVersion );
  QgsSettings settings;

  if ( settings.value( QStringLiteral( "qgis/warnOldProjectVersion" ), QVariant( true ) ).toBool() )
  {
    QString smalltext = tr( "This project file was saved by an older version of QGIS."
                            " When saving this project file, QGIS will update it to the latest version, "
                            "possibly rendering it useless for older versions of QGIS." );

    QString title = tr( "Project file is older" );

    messageBar()->pushMessage( title, smalltext );
  }
}

void QgisApp::updateUndoActions()
{
  bool canUndo = false, canRedo = false;
  QgsMapLayer *layer = activeLayer();
  if ( layer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer && vlayer->isEditable() )
    {
      canUndo = vlayer->undoStack()->canUndo();
      canRedo = vlayer->undoStack()->canRedo();
    }
  }
  mActionUndo->setEnabled( canUndo );
  mActionRedo->setEnabled( canRedo );
}


// add project directory to python path
void QgisApp::projectChanged( const QDomDocument &doc )
{
  Q_UNUSED( doc );
  QgsProject *project = qobject_cast<QgsProject *>( sender() );
  if ( !project )
    return;

  QFileInfo fi( project->fileName() );
  if ( !fi.exists() )
    return;

  static QString sPrevProjectDir = QString();

  if ( sPrevProjectDir == fi.canonicalPath() )
    return;

  QString expr;
  if ( !sPrevProjectDir.isNull() )
  {
    QString prev = sPrevProjectDir;
    expr = QStringLiteral( "sys.path.remove(u'%1'); " ).arg( prev.replace( '\'', QLatin1String( "\\'" ) ) );
  }

  sPrevProjectDir = fi.canonicalPath();

  QString prev = sPrevProjectDir;
  expr += QStringLiteral( "sys.path.append(u'%1')" ).arg( prev.replace( '\'', QLatin1String( "\\'" ) ) );

  QgsPythonRunner::run( expr );
}

void QgisApp::writeProject( QDomDocument &doc )
{
  // QGIS server does not use QgsProject for loading of QGIS project.
  // In order to allow reading of new projects, let's also write the original <legend> tag to the project.
  // Ideally the server should be ported to new layer tree implementation, but that requires
  // non-trivial changes to the server components.
  // The <legend> tag is ignored by QGIS application in >= 2.4 and this way also the new project files
  // can be opened in older versions of QGIS without losing information about layer groups.

  QgsLayerTree *clonedRoot = QgsProject::instance()->layerTreeRoot()->clone();
  QgsLayerTreeUtils::replaceChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( clonedRoot ) );
  QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( clonedRoot ), QgsProject::instance() ); // convert absolute paths to relative paths if required
  QDomElement oldLegendElem = QgsLayerTreeUtils::writeOldLegend( doc, QgsLayerTree::toGroup( clonedRoot ),
                              clonedRoot->hasCustomLayerOrder(), clonedRoot->customLayerOrder() );
  delete clonedRoot;
  QDomElement qgisNode = doc.firstChildElement( QStringLiteral( "qgis" ) );
  qgisNode.appendChild( oldLegendElem );

  QgsProject::instance()->writeEntry( QStringLiteral( "Legend" ), QStringLiteral( "filterByMap" ), static_cast< bool >( layerTreeView()->layerTreeModel()->legendFilterMapSettings() ) );

  // Save the position of the map view docks
  QDomElement mapViewNode = doc.createElement( QStringLiteral( "mapViewDocks" ) );
  Q_FOREACH ( QgsMapCanvasDockWidget *w, findChildren< QgsMapCanvasDockWidget * >() )
  {
    QDomElement node = doc.createElement( QStringLiteral( "view" ) );
    node.setAttribute( QStringLiteral( "name" ), w->mapCanvas()->objectName() );
    node.setAttribute( QStringLiteral( "synced" ), w->isViewCenterSynchronized() );
    node.setAttribute( QStringLiteral( "showCursor" ), w->isCursorMarkerVisible() );
    node.setAttribute( QStringLiteral( "showExtent" ), w->isMainCanvasExtentVisible() );
    node.setAttribute( QStringLiteral( "scaleSynced" ), w->isViewScaleSynchronized() );
    node.setAttribute( QStringLiteral( "scaleFactor" ), w->scaleFactor() );
    node.setAttribute( QStringLiteral( "showLabels" ), w->labelsVisible() );
    writeDockWidgetSettings( w, node );
    mapViewNode.appendChild( node );
  }
  qgisNode.appendChild( mapViewNode );

#ifdef HAVE_3D
  QgsReadWriteContext readWriteContext;
  readWriteContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QDomElement elem3DMaps = doc.createElement( QStringLiteral( "mapViewDocks3D" ) );
  for ( Qgs3DMapCanvasDockWidget *w : findChildren<Qgs3DMapCanvasDockWidget *>() )
  {
    QDomElement elem3DMap = doc.createElement( QStringLiteral( "view" ) );
    elem3DMap.setAttribute( QStringLiteral( "name" ), w->mapCanvas3D()->objectName() );
    QDomElement elem3DMapSettings = w->mapCanvas3D()->map()->writeXml( doc, readWriteContext );
    elem3DMap.appendChild( elem3DMapSettings );
    QDomElement elemCamera = w->mapCanvas3D()->cameraController()->writeXml( doc );
    elem3DMap.appendChild( elemCamera );
    writeDockWidgetSettings( w, elem3DMap );
    elem3DMaps.appendChild( elem3DMap );
  }
  qgisNode.appendChild( elem3DMaps );
#endif

  projectChanged( doc );
}

void QgisApp::writeDockWidgetSettings( QDockWidget *dockWidget, QDomElement &elem )
{
  elem.setAttribute( QStringLiteral( "x" ), dockWidget->x() );
  elem.setAttribute( QStringLiteral( "y" ), dockWidget->y() );
  elem.setAttribute( QStringLiteral( "width" ), dockWidget->width() );
  elem.setAttribute( QStringLiteral( "height" ), dockWidget->height() );
  elem.setAttribute( QStringLiteral( "floating" ), dockWidget->isFloating() );
  elem.setAttribute( QStringLiteral( "area" ), dockWidgetArea( dockWidget ) );
}

bool QgisApp::askUserForDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs )
{
  Q_ASSERT( qApp->thread() == QThread::currentThread() );

  bool ok = false;

  QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
  if ( context.hasTransform( sourceCrs, destinationCrs ) )
  {
    ok = true;
  }
  else
  {
    //if several possibilities:  present dialog
    QgsDatumTransformDialog dlg( sourceCrs, destinationCrs );
    if ( dlg.availableTransformationCount() > 1 )
    {
      bool ask = QgsSettings().value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool();
      if ( !ask )
      {
        ok = false;
      }
      else if ( dlg.exec() )
      {
        QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > dt = dlg.selectedDatumTransforms();
        QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
        context.addSourceDestinationDatumTransform( dt.first.first, dt.second.first, dt.first.second, dt.second.second );
        QgsProject::instance()->setTransformContext( context );
        ok = true;
      }
      else
      {
        ok = false;
      }
    }
    else
    {
      ok = true;
    }
  }
  return ok;
}

void QgisApp::readDockWidgetSettings( QDockWidget *dockWidget, const QDomElement &elem )
{
  int x = elem.attribute( QStringLiteral( "x" ), QStringLiteral( "0" ) ).toInt();
  int y = elem.attribute( QStringLiteral( "y" ), QStringLiteral( "0" ) ).toInt();
  int w = elem.attribute( QStringLiteral( "width" ), QStringLiteral( "400" ) ).toInt();
  int h = elem.attribute( QStringLiteral( "height" ), QStringLiteral( "400" ) ).toInt();
  bool floating = elem.attribute( QStringLiteral( "floating" ), QStringLiteral( "0" ) ).toInt();
  Qt::DockWidgetArea area = static_cast< Qt::DockWidgetArea >( elem.attribute( QStringLiteral( "area" ), QString::number( Qt::RightDockWidgetArea ) ).toInt() );

  setupDockWidget( dockWidget, floating, QRect( x, y, w, h ), area );
}


void QgisApp::readProject( const QDomDocument &doc )
{
  projectChanged( doc );

  // force update of canvas, without automatic changes to extent and OTF projections
  bool autoSetupOnFirstLayer = mLayerTreeCanvasBridge->autoSetupOnFirstLayer();
  mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( false );

  mLayerTreeCanvasBridge->setCanvasLayers();

  if ( autoSetupOnFirstLayer )
    mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer( true );

  QDomNodeList nodes = doc.elementsByTagName( QStringLiteral( "mapViewDocks" ) );
  QList< QgsMapCanvas * > views;
  if ( !nodes.isEmpty() )
  {
    QDomNode viewNode = nodes.at( 0 );
    nodes = viewNode.childNodes();
    for ( int i = 0; i < nodes.size(); ++i )
    {
      QDomElement elementNode = nodes.at( i ).toElement();
      QString mapName = elementNode.attribute( QStringLiteral( "name" ) );
      bool synced = elementNode.attribute( QStringLiteral( "synced" ), QStringLiteral( "0" ) ).toInt();
      bool showCursor = elementNode.attribute( QStringLiteral( "showCursor" ), QStringLiteral( "0" ) ).toInt();
      bool showExtent = elementNode.attribute( QStringLiteral( "showExtent" ), QStringLiteral( "0" ) ).toInt();
      bool scaleSynced = elementNode.attribute( QStringLiteral( "scaleSynced" ), QStringLiteral( "0" ) ).toInt();
      double scaleFactor = elementNode.attribute( QStringLiteral( "scaleFactor" ), QStringLiteral( "1" ) ).toDouble();
      bool showLabels = elementNode.attribute( QStringLiteral( "showLabels" ), QStringLiteral( "1" ) ).toInt();

      QgsMapCanvasDockWidget *mapCanvasDock = createNewMapCanvasDock( mapName );
      readDockWidgetSettings( mapCanvasDock, elementNode );
      QgsMapCanvas *mapCanvas = mapCanvasDock->mapCanvas();
      mapCanvasDock->setViewCenterSynchronized( synced );
      mapCanvasDock->setCursorMarkerVisible( showCursor );
      mapCanvasDock->setScaleFactor( scaleFactor );
      mapCanvasDock->setViewScaleSynchronized( scaleSynced );
      mapCanvasDock->setMainCanvasExtentVisible( showExtent );
      mapCanvasDock->setLabelsVisible( showLabels );
      mapCanvas->readProject( doc );
      views << mapCanvas;
    }
  }

#ifdef HAVE_3D
  QgsReadWriteContext readWriteContext;
  readWriteContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QDomElement elem3DMaps = doc.documentElement().firstChildElement( QStringLiteral( "mapViewDocks3D" ) );
  if ( !elem3DMaps.isNull() )
  {
    QDomElement elem3DMap = elem3DMaps.firstChildElement( QStringLiteral( "view" ) );
    while ( !elem3DMap.isNull() )
    {
      QString mapName = elem3DMap.attribute( QStringLiteral( "name" ) );

      Qgs3DMapCanvasDockWidget *mapCanvasDock3D = createNew3DMapCanvasDock( mapName );
      readDockWidgetSettings( mapCanvasDock3D, elem3DMap );

      QDomElement elem3D = elem3DMap.firstChildElement( QStringLiteral( "qgis3d" ) );
      Qgs3DMapSettings *map = new Qgs3DMapSettings;
      map->readXml( elem3D, readWriteContext );
      map->resolveReferences( *QgsProject::instance() );

      // these things are not saved in project
      map->setSelectionColor( mMapCanvas->selectionColor() );
      map->setBackgroundColor( mMapCanvas->canvasColor() );
      if ( map->terrainGenerator()->type() == QgsTerrainGenerator::Flat )
      {
        QgsFlatTerrainGenerator *flatTerrainGen = static_cast<QgsFlatTerrainGenerator *>( map->terrainGenerator() );
        flatTerrainGen->setExtent( mMapCanvas->fullExtent() );
      }

      mapCanvasDock3D->setMapSettings( map );

      QDomElement elemCamera = elem3DMap.firstChildElement( QStringLiteral( "camera" ) );
      if ( !elemCamera.isNull() )
      {
        mapCanvasDock3D->mapCanvas3D()->cameraController()->readXml( elemCamera );
      }

      elem3DMap = elem3DMap.nextSiblingElement( QStringLiteral( "view" ) );
    }
  }
#endif

  // unfreeze all new views at once. We don't do this as they are created since additional
  // views which may exist in project could rearrange the docks and cause the canvases to resize
  // resulting in multiple redraws
  Q_FOREACH ( QgsMapCanvas *c, views )
  {
    c->freeze( false );
  }
}

void QgisApp::showLayerProperties( QgsMapLayer *ml )
{
  /*
  TODO: Consider reusing the property dialogs again.
  Sometimes around mid 2005, the property dialogs were saved for later reuse;
  this resulted in a time savings when reopening the dialog. The code below
  cannot be used as is, however, simply by saving the dialog pointer here.
  Either the map layer needs to be passed as an argument to sync or else
  a separate copy of the dialog pointer needs to be stored with each layer.
  */

  if ( !ml )
    return;

  if ( !QgsProject::instance()->layerIsEmbedded( ml->id() ).isEmpty() )
  {
    return; //don't show properties of embedded layers
  }

  if ( ml->type() == QgsMapLayer::RasterLayer )
  {
#if 0 // See note above about reusing this
    QgsRasterLayerProperties *rlp = nullptr;
    if ( rlp )
    {
      rlp->sync();
    }
    else
    {
      rlp = new QgsRasterLayerProperties( ml, mMapCanvas, this );
      // handled by rendererChanged() connect( rlp, SIGNAL( refreshLegend( QString, bool ) ), mLayerTreeView, SLOT( refreshLayerSymbology( QString ) ) );
    }
#else
    QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties( ml, mMapCanvas, this );
#endif

    rlp->exec();
    delete rlp; // delete since dialog cannot be reused without updating code
  }
  else if ( ml->type() == QgsMapLayer::VectorLayer ) // VECTOR
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( ml );

#if 0 // See note above about reusing this
    QgsVectorLayerProperties *vlp = nullptr;
    if ( vlp )
    {
      vlp->syncToLayer();
    }
    else
    {
      vlp = new QgsVectorLayerProperties( vlayer, this );
      // handled by rendererChanged() connect( vlp, SIGNAL( refreshLegend( QString ) ), mLayerTreeView, SLOT( refreshLayerSymbology( QString ) ) );
    }
#else
    QgsVectorLayerProperties *vlp = new QgsVectorLayerProperties( vlayer, this );
#endif
    Q_FOREACH ( QgsMapLayerConfigWidgetFactory *factory, mMapLayerPanelFactories )
    {
      vlp->addPropertiesPageFactory( factory );
    }

    mMapStyleWidget->blockUpdates( true );
    if ( vlp->exec() )
    {
      activateDeactivateLayerRelatedActions( ml );
      mMapStyleWidget->updateCurrentWidgetLayer();
    }
    mMapStyleWidget->blockUpdates( false );

    delete vlp; // delete since dialog cannot be reused without updating code
  }
  else if ( ml->type() == QgsMapLayer::PluginLayer )
  {
    QgsPluginLayer *pl = qobject_cast<QgsPluginLayer *>( ml );
    if ( !pl )
      return;

    QgsPluginLayerType *plt = QgsApplication::pluginLayerRegistry()->pluginLayerType( pl->pluginLayerType() );
    if ( !plt )
      return;

    if ( !plt->showLayerProperties( pl ) )
    {
      messageBar()->pushMessage( tr( "Warning" ),
                                 tr( "This layer doesn't have a properties dialog." ),
                                 QgsMessageBar::INFO, messageTimeout() );
    }
  }
}

void QgisApp::namSetup()
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  connect( nam, &QNetworkAccessManager::authenticationRequired,
           this, &QgisApp::namAuthenticationRequired );

  connect( nam, &QNetworkAccessManager::proxyAuthenticationRequired,
           this, &QgisApp::namProxyAuthenticationRequired );

  connect( nam, &QgsNetworkAccessManager::requestTimedOut,
           this, &QgisApp::namRequestTimedOut );

#ifndef QT_NO_SSL
  connect( nam, &QNetworkAccessManager::sslErrors,
           this, &QgisApp::namSslErrors );
#endif
}

void QgisApp::namAuthenticationRequired( QNetworkReply *inReply, QAuthenticator *auth )
{
  QPointer<QNetworkReply> reply( inReply );
  Q_ASSERT( qApp->thread() == QThread::currentThread() );

  QString username = auth->user();
  QString password = auth->password();

  if ( username.isEmpty() && password.isEmpty() && reply->request().hasRawHeader( "Authorization" ) )
  {
    QByteArray header( reply->request().rawHeader( "Authorization" ) );
    if ( header.startsWith( "Basic " ) )
    {
      QByteArray auth( QByteArray::fromBase64( header.mid( 6 ) ) );
      int pos = auth.indexOf( ':' );
      if ( pos >= 0 )
      {
        username = auth.left( pos );
        password = auth.mid( pos + 1 );
      }
    }
  }

  for ( ;; )
  {
    bool ok;

    {
      QMutexLocker lock( QgsCredentials::instance()->mutex() );
      ok = QgsCredentials::instance()->get(
             QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
             username, password,
             tr( "Authentication required" ) );
    }
    if ( !ok )
      return;

    if ( reply.isNull() || reply->isFinished() )
      return;

    if ( auth->user() != username || ( password != auth->password() && !password.isNull() ) )
      break;

    // credentials didn't change - stored ones probably wrong? clear password and retry
    {
      QMutexLocker lock( QgsCredentials::instance()->mutex() );
      QgsCredentials::instance()->put(
        QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
        username, QString() );
    }
  }

  // save credentials
  {
    QMutexLocker lock( QgsCredentials::instance()->mutex() );
    QgsCredentials::instance()->put(
      QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
      username, password
    );
  }

  auth->setUser( username );
  auth->setPassword( password );
}

void QgisApp::namProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *auth )
{
  QgsSettings settings;
  if ( !settings.value( QStringLiteral( "proxy/proxyEnabled" ), false ).toBool() ||
       settings.value( QStringLiteral( "proxy/proxyType" ), "" ).toString() == QLatin1String( "DefaultProxy" ) )
  {
    auth->setUser( QLatin1String( "" ) );
    return;
  }

  QString username = auth->user();
  QString password = auth->password();

  for ( ;; )
  {
    bool ok;

    {
      QMutexLocker lock( QgsCredentials::instance()->mutex() );
      ok = QgsCredentials::instance()->get(
             QStringLiteral( "proxy %1:%2 [%3]" ).arg( proxy.hostName() ).arg( proxy.port() ).arg( auth->realm() ),
             username, password,
             tr( "Proxy authentication required" ) );
    }
    if ( !ok )
      return;

    if ( auth->user() != username || ( password != auth->password() && !password.isNull() ) )
      break;

    // credentials didn't change - stored ones probably wrong? clear password and retry
    {
      QMutexLocker lock( QgsCredentials::instance()->mutex() );
      QgsCredentials::instance()->put(
        QStringLiteral( "proxy %1:%2 [%3]" ).arg( proxy.hostName() ).arg( proxy.port() ).arg( auth->realm() ),
        username, QString() );
    }
  }

  {
    QMutexLocker lock( QgsCredentials::instance()->mutex() );
    QgsCredentials::instance()->put(
      QStringLiteral( "proxy %1:%2 [%3]" ).arg( proxy.hostName() ).arg( proxy.port() ).arg( auth->realm() ),
      username, password
    );
  }

  auth->setUser( username );
  auth->setPassword( password );
}

#ifndef QT_NO_SSL
void QgisApp::namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  // stop the timeout timer, or app crashes if the user (or slot) takes longer than
  // singleshot timeout and tries to update the closed QNetworkReply
  QTimer *timer = reply->findChild<QTimer *>( QStringLiteral( "timeoutTimer" ) );
  if ( timer )
  {
    QgsDebugMsg( "Stopping network reply timeout" );
    timer->stop();
  }

  QgsDebugMsg( QString( "SSL errors occurred accessing URL:\n%1" ).arg( reply->request().url().toString() ) );

  QString hostport( QStringLiteral( "%1:%2" )
                    .arg( reply->url().host() )
                    .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                    .trimmed() );
  QString digest( QgsAuthCertUtils::shaHexForCert( reply->sslConfiguration().peerCertificate() ) );
  QString dgsthostport( QStringLiteral( "%1:%2" ).arg( digest, hostport ) );

  const QHash<QString, QSet<QSslError::SslError> > &errscache( QgsApplication::authManager()->ignoredSslErrorCache() );

  if ( errscache.contains( dgsthostport ) )
  {
    QgsDebugMsg( QString( "Ignored SSL errors cahced item found, ignoring errors if they match for %1" ).arg( hostport ) );
    const QSet<QSslError::SslError> &errenums( errscache.value( dgsthostport ) );
    bool ignore = !errenums.isEmpty();
    int errmatched = 0;
    if ( ignore )
    {
      Q_FOREACH ( const QSslError &error, errors )
      {
        if ( error.error() == QSslError::NoError )
          continue;

        bool errmatch = errenums.contains( error.error() );
        ignore = ignore && errmatch;
        errmatched += errmatch ? 1 : 0;
      }
    }

    if ( ignore && errenums.size() == errmatched )
    {
      QgsDebugMsg( QString( "Errors matched cached item's, ignoring all for %1" ).arg( hostport ) );
      reply->ignoreSslErrors();
      return;
    }

    QgsDebugMsg( QString( "Errors %1 for cached item for %2" )
                 .arg( errenums.isEmpty() ? "not found" : "did not match",
                       hostport ) );
  }


  QgsAuthSslErrorsDialog *dlg = new QgsAuthSslErrorsDialog( reply, errors, this, digest, hostport );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 580, 512 );
  if ( dlg->exec() )
  {
    if ( reply )
    {
      QgsDebugMsg( QString( "All SSL errors ignored for %1" ).arg( hostport ) );
      reply->ignoreSslErrors();
    }
  }
  dlg->deleteLater();

  // restart network request timeout timer
  if ( reply )
  {
    QgsSettings s;
    QTimer *timer = reply->findChild<QTimer *>( QStringLiteral( "timeoutTimer" ) );
    if ( timer )
    {
      QgsDebugMsg( "Restarting network reply timeout" );
      timer->setSingleShot( true );
      timer->start( s.value( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), "60000" ).toInt() );
    }
  }
}
#endif

void QgisApp::namRequestTimedOut( QNetworkReply *reply )
{
  Q_UNUSED( reply );
  QLabel *msgLabel = new QLabel( tr( "A network request timed out, any data received is likely incomplete." ) +
                                 tr( " Please check the <a href=\"#messageLog\">message log</a> for further info." ), messageBar() );
  msgLabel->setWordWrap( true );
  connect( msgLabel, &QLabel::linkActivated, mLogDock, &QWidget::show );
  messageBar()->pushItem( new QgsMessageBarItem( msgLabel, QgsMessageBar::WARNING, messageTimeout() ) );
}

void QgisApp::namUpdate()
{
  QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
}

void QgisApp::masterPasswordSetup()
{
  connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
           this, &QgisApp::authMessageOut );
  connect( QgsApplication::authManager(), &QgsAuthManager::passwordHelperMessageOut,
           this, &QgisApp::authMessageOut );
  connect( QgsApplication::authManager(), &QgsAuthManager::authDatabaseEraseRequested,
           this, &QgisApp::eraseAuthenticationDatabase );
}

void QgisApp::eraseAuthenticationDatabase()
{
  // First check if now is a good time to interact with the user, e.g. project is done loading.
  // If not, ask QgsAuthManager to re-emit authDatabaseEraseRequested from the schedule timer.
  // No way to know if user interaction will interfere with plugins loading layers.

  if ( !QgsProject::instance()->fileName().isNull() ) // a non-blank project is loaded
  {
    // Apparently, as of QGIS 2.9, the only way to query that the project is in a
    // layer-loading state is via a custom property of the project's layer tree.
    QgsLayerTreeGroup *layertree( QgsProject::instance()->layerTreeRoot() );
    if ( layertree && layertree->customProperty( QStringLiteral( "loading" ) ).toBool() )
    {
      QgsDebugMsg( "Project loading, skipping auth db erase" );
      QgsApplication::authManager()->setScheduledAuthDatabaseEraseRequestEmitted( false );
      return;
    }
  }

  // TODO: Check is Browser panel is also still loading?
  //       It has auto-connections in parallel (if tree item is expanded), though
  //       such connections with possible master password requests *should* be ignored
  //       when there is an authentication db erase scheduled.

  // This function should tell QgsAuthManager to stop any erase db schedule timer,
  // *after* interacting with the user
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), messageTimeout(), this );
}

void QgisApp::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  // Use system notifications if the main window is not the active one,
  // push message to the message bar if the main window is active
  if ( qApp->activeWindow() != this )
  {
    showSystemNotification( tr( "QGIS Authentication" ), message );
  }
  else
  {
    int levelint = static_cast< int >( level );
    messageBar()->pushMessage( authtag, message, static_cast< QgsMessageBar::MessageLevel >( levelint ), 7 );
  }
}

void QgisApp::completeInitialization()
{
  emit initializationCompleted();
}

void QgisApp::toolButtonActionTriggered( QAction *action )
{
  QToolButton *bt = qobject_cast<QToolButton *>( sender() );
  if ( !bt )
    return;

  QgsSettings settings;
  if ( action == mActionSelectFeatures )
    settings.setValue( QStringLiteral( "UI/selectTool" ), 1 );
  else if ( action == mActionSelectRadius )
    settings.setValue( QStringLiteral( "UI/selectTool" ), 2 );
  else if ( action == mActionSelectPolygon )
    settings.setValue( QStringLiteral( "UI/selectTool" ), 3 );
  else if ( action == mActionSelectFreehand )
    settings.setValue( QStringLiteral( "UI/selectTool" ), 4 );
  else if ( action == mActionSelectByForm )
    settings.setValue( QStringLiteral( "UI/selectionTool" ), 0 );
  else if ( action == mActionSelectByExpression )
    settings.setValue( QStringLiteral( "UI/selectionTool" ), 1 );
  else if ( action == mActionSelectAll )
    settings.setValue( QStringLiteral( "UI/selectionTool" ), 2 );
  else if ( action == mActionInvertSelection )
    settings.setValue( QStringLiteral( "UI/selectionTool" ), 3 );
  else if ( action == mActionMeasure )
    settings.setValue( QStringLiteral( "UI/measureTool" ), 0 );
  else if ( action == mActionMeasureArea )
    settings.setValue( QStringLiteral( "UI/measureTool" ), 1 );
  else if ( action == mActionMeasureAngle )
    settings.setValue( QStringLiteral( "UI/measureTool" ), 2 );
  else if ( action == mActionTextAnnotation )
    settings.setValue( QStringLiteral( "UI/annotationTool" ), 0 );
  else if ( action == mActionFormAnnotation )
    settings.setValue( QStringLiteral( "UI/annotationTool" ), 1 );
  else if ( action == mActionHtmlAnnotation )
    settings.setValue( QStringLiteral( "UI/annotationTool" ), 2 );
  else if ( action == mActionSvgAnnotation )
    settings.setValue( QStringLiteral( "UI/annotationTool" ), 3 );
  else if ( action == mActionAnnotation )
    settings.setValue( QStringLiteral( "UI/annotationTool" ), 4 );
  else if ( action == mActionNewSpatiaLiteLayer )
    settings.setValue( QStringLiteral( "UI/defaultNewLayer" ), 0 );
  else if ( action == mActionNewVectorLayer )
    settings.setValue( QStringLiteral( "UI/defaultNewLayer" ), 1 );
  else if ( action == mActionNewMemoryLayer )
    settings.setValue( QStringLiteral( "UI/defaultNewLayer" ), 2 );
  else if ( action == mActionNewGeoPackageLayer )
    settings.setValue( QStringLiteral( "UI/defaultNewLayer" ), 3 );
  else if ( action == mActionRotatePointSymbols )
    settings.setValue( QStringLiteral( "UI/defaultPointSymbolAction" ), 0 );
  else if ( action == mActionOffsetPointSymbol )
    settings.setValue( QStringLiteral( "UI/defaultPointSymbolAction" ), 1 );
  else if ( mActionAddPgLayer && action == mActionAddPgLayer )
    settings.setValue( QStringLiteral( "UI/defaultAddDbLayerAction" ), 0 );
  else if ( mActionAddMssqlLayer && action == mActionAddMssqlLayer )
    settings.setValue( QStringLiteral( "UI/defaultAddDbLayerAction" ), 1 );
  else if ( mActionAddDb2Layer && action == mActionAddDb2Layer )
    settings.setValue( QStringLiteral( "UI/defaultAddDbLayerAction" ), 2 );
  else if ( mActionAddOracleLayer && action == mActionAddOracleLayer )
    settings.setValue( QStringLiteral( "UI/defaultAddDbLayerAction" ), 3 );
  else if ( action == mActionAddWfsLayer )
    settings.setValue( QStringLiteral( "UI/defaultFeatureService" ), 0 );
  else if ( action == mActionAddAfsLayer )
    settings.setValue( QStringLiteral( "UI/defaultFeatureService" ), 1 );
  else if ( action == mActionAddWmsLayer )
    settings.setValue( QStringLiteral( "UI/defaultMapService" ), 0 );
  else if ( action == mActionAddAmsLayer )
    settings.setValue( QStringLiteral( "UI/defaultMapService" ), 1 );
  else if ( action == mActionMoveFeature )
    settings.setValue( QStringLiteral( "UI/defaultMoveTool" ), 0 );
  else if ( action == mActionMoveFeatureCopy )
    settings.setValue( QStringLiteral( "UI/defaultMoveTool" ), 1 );

  bt->setDefaultAction( action );
}

QMenu *QgisApp::createPopupMenu()
{
  QMenu *menu = QMainWindow::createPopupMenu();
  QList< QAction * > al = menu->actions();
  QList< QAction * > panels, toolbars;

  if ( !al.isEmpty() )
  {
    bool found = false;
    for ( int i = 0; i < al.size(); ++i )
    {
      if ( al[ i ]->isSeparator() )
      {
        found = true;
        continue;
      }

      if ( !found )
      {
        panels.append( al[ i ] );
      }
      else
      {
        toolbars.append( al[ i ] );
      }
    }

    std::sort( panels.begin(), panels.end(), cmpByText_ );
    QWidgetAction *panelstitle = new QWidgetAction( menu );
    QLabel *plabel = new QLabel( QStringLiteral( "<b>%1</b>" ).arg( tr( "Panels" ) ) );
    plabel->setMargin( 3 );
    plabel->setAlignment( Qt::AlignHCenter );
    panelstitle->setDefaultWidget( plabel );
    menu->addAction( panelstitle );
    Q_FOREACH ( QAction *a, panels )
    {
      menu->addAction( a );
    }
    menu->addSeparator();
    QWidgetAction *toolbarstitle = new QWidgetAction( menu );
    QLabel *tlabel = new QLabel( QStringLiteral( "<b>%1</b>" ).arg( tr( "Toolbars" ) ) );
    tlabel->setMargin( 3 );
    tlabel->setAlignment( Qt::AlignHCenter );
    toolbarstitle->setDefaultWidget( tlabel );
    menu->addAction( toolbarstitle );
    std::sort( toolbars.begin(), toolbars.end(), cmpByText_ );
    Q_FOREACH ( QAction *a, toolbars )
    {
      menu->addAction( a );
    }
  }

  return menu;
}


void QgisApp::showSystemNotification( const QString &title, const QString &message )
{
  // Menubar icon is hidden by default. Show to enable notification bubbles
  mTray->show();
  mTray->showMessage( title, message );
  // Re-hide menubar icon
  mTray->hide();
}

void QgisApp::showStatisticsDockWidget()
{
  mStatisticalSummaryDockWidget->show();
  mStatisticalSummaryDockWidget->raise();
}

void QgisApp::onLayerError( const QString &msg )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );

  Q_ASSERT( layer );

  mInfoBar->pushCritical( tr( "Layer %1" ).arg( layer->name() ), msg );
}

bool QgisApp::gestureEvent( QGestureEvent *event )
{
#ifdef Q_OS_ANDROID
  if ( QGesture *tapAndHold = event->gesture( Qt::TapAndHoldGesture ) )
  {
    tapAndHoldTriggered( static_cast<QTapAndHoldGesture *>( tapAndHold ) );
  }
  return true;
#else
  Q_UNUSED( event );
  return false;
#endif
}

void QgisApp::tapAndHoldTriggered( QTapAndHoldGesture *gesture )
{
  if ( gesture->state() == Qt::GestureFinished )
  {
    QPoint pos = gesture->position().toPoint();
    QWidget *receiver = QApplication::widgetAt( pos );
    qDebug() << "tapAndHoldTriggered: LONG CLICK gesture happened at " << pos;
    qDebug() << "widget under point of click: " << receiver;

    QApplication::postEvent( receiver, new QMouseEvent( QEvent::MouseButtonPress, receiver->mapFromGlobal( pos ), Qt::RightButton, Qt::RightButton, Qt::NoModifier ) );
    QApplication::postEvent( receiver, new QMouseEvent( QEvent::MouseButtonRelease, receiver->mapFromGlobal( pos ), Qt::RightButton, Qt::RightButton, Qt::NoModifier ) );
  }
}

void QgisApp::transactionGroupCommitError( const QString &error )
{
  displayMessage( tr( "Transaction" ), error, QgsMessageBar::CRITICAL );
}

QgsFeature QgisApp::duplicateFeatures( QgsMapLayer *mlayer, const QgsFeature &feature )
{
  if ( mlayer->type() != QgsMapLayer::VectorLayer )
    return QgsFeature();

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mlayer );

  layer->startEditing();

  QgsFeatureList featureList;

  if ( feature.isValid() )
  {
    featureList.append( feature );
  }
  else
  {
    const auto selectedFeatures = layer->selectedFeatures();
    for ( const QgsFeature &f : selectedFeatures )
    {
      featureList.append( f );
    }
  }

  int featureCount = 0;

  QString childrenInfo;

  for ( const QgsFeature &f : featureList )
  {
    QgsVectorLayerUtils::QgsDuplicateFeatureContext duplicateFeatureContext;

    QgsVectorLayerUtils::duplicateFeature( layer, f, QgsProject::instance(), 0, duplicateFeatureContext );
    featureCount += 1;

    const auto duplicatedFeatureContextLayers = duplicateFeatureContext.layers();
    for ( QgsVectorLayer *chl : duplicatedFeatureContextLayers )
    {
      childrenInfo += ( tr( "%1 children on layer %2 duplicated" ).arg( QString::number( duplicateFeatureContext.duplicatedFeatures( chl ).size() ), chl->name() ) );
    }
  }

  messageBar()->pushMessage( tr( "%1 features on layer %2 duplicated\n%3" ).arg( QString::number( featureCount ), layer->name(), childrenInfo ), QgsMessageBar::SUCCESS, 5 );

  return QgsFeature();
}


QgsFeature QgisApp::duplicateFeatureDigitized( QgsMapLayer *mlayer, const QgsFeature &feature )
{
  if ( mlayer->type() != QgsMapLayer::VectorLayer )
    return QgsFeature();

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mlayer );

  layer->startEditing();

  QgsMapToolDigitizeFeature *digitizeFeature = new QgsMapToolDigitizeFeature( mMapCanvas, mlayer, QgsMapToolCapture::CaptureNone );

  mMapCanvas->setMapTool( digitizeFeature );
  mMapCanvas->window()->raise();
  mMapCanvas->activateWindow();
  mMapCanvas->setFocus();

  QString msg = tr( "Digitize the duplicate on layer %1" ).arg( layer->name() );
  messageBar()->pushMessage( msg, QgsMessageBar::INFO, 3 );

  connect( digitizeFeature, static_cast<void ( QgsMapToolDigitizeFeature::* )( const QgsFeature & )>( &QgsMapToolDigitizeFeature::digitizingCompleted ), this, [this, layer, feature, digitizeFeature]( const QgsFeature & digitizedFeature )
  {
    QString msg = tr( "Duplicate digitized" );
    messageBar()->pushMessage( msg, QgsMessageBar::INFO, 1 );

    QgsVectorLayerUtils::QgsDuplicateFeatureContext duplicateFeatureContext;

    QgsFeature newFeature = feature;
    newFeature.setGeometry( digitizedFeature.geometry() );
    QgsVectorLayerUtils::duplicateFeature( layer, newFeature, QgsProject::instance(), 0, duplicateFeatureContext );

    QString childrenInfo;
    const auto duplicateFeatureContextLayers = duplicateFeatureContext.layers();
    for ( QgsVectorLayer *chl : duplicateFeatureContextLayers )
    {
      childrenInfo += ( tr( "%1 children on layer %2 duplicated" ).arg( duplicateFeatureContext.duplicatedFeatures( chl ).size() ).arg( chl->name() ) );
    }

    messageBar()->pushMessage( tr( "Feature on layer %2 duplicated\n%3" ).arg( layer->name() ).arg( childrenInfo ), QgsMessageBar::SUCCESS, 5 );

    mMapCanvas->unsetMapTool( digitizeFeature );
  }
         );

  connect( digitizeFeature, static_cast<void ( QgsMapToolDigitizeFeature::* )()>( &QgsMapToolDigitizeFeature::digitizingFinished ), this, [this, digitizeFeature]()
  {
    digitizeFeature->deleteLater();
  }
         );

  return QgsFeature();
}

