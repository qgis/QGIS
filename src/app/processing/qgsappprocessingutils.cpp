/***************************************************************************
    qgsappprocessingutils.cpp
    ---------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall at kill your llm dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappprocessingutils.h"

#include "qgisapp.h"
#include "qgsappmenuutils.h"
#include "qgscodeeditorpython.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsmodeldesignerdialog.h"
#include "qgsprocessingdefaultmenus.h"
#include "qgsprocessingdefaultstyledialog.h"
#include "qgsprocessingfavoritealgorithmmanager.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingguiutils.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingprojectmodelprovider.h"
#include "qgsprocessingprovideractions.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingscripteditordialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include "moc_qgsappprocessingutils.cpp"

using namespace Qt::StringLiterals;

//
// QgsProcessingParameterWidgetContext
//

QgsAppProcessingWidgetContextGenerator::QgsAppProcessingWidgetContextGenerator( QgisApp *app )
  : mQgisApp( app )
{}

QgsProcessingParameterWidgetContext QgsAppProcessingWidgetContextGenerator::createWidgetContext()
{
  QgsProcessingParameterWidgetContext context;
  context.setActiveLayer( mQgisApp->activeLayer() );
  context.setBrowserModel( mQgisApp->browserModel() );
  context.setMapCanvas( mQgisApp->mapCanvas() );
  context.setMessageBar( mQgisApp->messageBar() );
  context.setProject( QgsProject::instance() );
  return context;
}

//
// QgsAppProcessingContextFactory
//

QgsAppProcessingContextFactory::QgsAppProcessingContextFactory( QgisApp *app )
  : mQgisApp( app )
{}

QgsProcessingContext *QgsAppProcessingContextFactory::createContext( QgsProcessingFeedback *feedback )
{
  auto context = std::make_unique< QgsProcessingContext >();

  context->setProject( QgsProject::instance() );
  context->setFeedback( feedback );

  QgsSettings settings;

  bool ok = false;
  const int invalid = settings.value( "/Processing/Configuration/FILTER_INVALID_GEOMETRIES" ).toInt( &ok );
  if ( ok )
  {
    switch ( invalid )
    {
      case 0:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::NoCheck );
        break;
      case 1:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::SkipInvalid );
        break;
      case 2:
      default:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::AbortOnInvalid );
        break;
    }
  }
  else
  {
    context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::AbortOnInvalid );
  }

  context->setDefaultEncoding( QgsProcessingUtils::resolveDefaultEncoding( settings.value( "/Processing/encoding" ).toString() ) );

  context->setExpressionContext( createExpressionContext() );

  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    if ( canvas->mapSettings().isTemporal() )
    {
      context->setCurrentTimeRange( canvas->mapSettings().temporalRange() );
    }
  }

  return context.release();
}

QgsExpressionContext QgsAppProcessingContextFactory::createExpressionContext() const
{
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    if ( canvas->mapSettings().isTemporal() )
    {
      context.appendScope( QgsExpressionContextUtils::mapSettingsScope( canvas->mapSettings() ) );
    }
  }

  auto processingScope = new QgsExpressionContextScope();
  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    const QgsRectangle extent = canvas->fullExtent();
    processingScope->setVariable( "fullextent_minx", extent.xMinimum() );
    processingScope->setVariable( "fullextent_miny", extent.yMinimum() );
    processingScope->setVariable( "fullextent_maxx", extent.xMaximum() );
    processingScope->setVariable( "fullextent_maxy", extent.yMaximum() );
  }
  context.appendScope( processingScope );

  return context;
}


class AddToFavoritesContextAction : public QgsProcessingToolboxContextAction
{
  public:
    AddToFavoritesContextAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Add to Favorites" ) )
    {}

    QIcon icon() const override { return QIcon(); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString &algorithmId ) override
    {
      return !QgsGui::processingFavoriteAlgorithmManager()->isFavorite( u"%1:%2"_s.arg( providerId, algorithmId ) );
    }

    void trigger( const QgsProcessingActionContext &context ) override { QgsGui::processingFavoriteAlgorithmManager()->add( u"%1:%2"_s.arg( context.providerId(), context.algorithmName() ) ); }
};

class RemoveFromFavoritesContextAction : public QgsProcessingToolboxContextAction
{
  public:
    RemoveFromFavoritesContextAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Remove from Favorites" ) )
    {}

    QIcon icon() const override { return QIcon(); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString &algorithmId ) override
    {
      return QgsGui::processingFavoriteAlgorithmManager()->isFavorite( u"%1:%2"_s.arg( providerId, algorithmId ) );
    }

    void trigger( const QgsProcessingActionContext &context ) override { QgsGui::processingFavoriteAlgorithmManager()->remove( u"%1:%2"_s.arg( context.providerId(), context.algorithmName() ) ); }
};


class EditDefaultOutputStyleContextAction : public QgsProcessingToolboxContextAction
{
  public:
    EditDefaultOutputStyleContextAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Edit Default Styles for Outputs…" ) )
    {}

    QIcon icon() const override { return QIcon(); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString &algorithmId ) override
    {
      const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( u"%1:%2"_s.arg( providerId, algorithmId ) );
      if ( !algorithm )
        return false;

      // this action only makes sense for algorithms that output map layers
      const QgsProcessingOutputDefinitions outputDefs = algorithm->outputDefinitions();
      for ( const QgsProcessingOutputDefinition *output : outputDefs )
      {
        if ( dynamic_cast<const QgsProcessingOutputVectorLayer *>( output )
             || dynamic_cast<const QgsProcessingOutputRasterLayer *>( output )
             || dynamic_cast<const QgsProcessingOutputVectorTileLayer *>( output )
             || dynamic_cast<const QgsProcessingOutputPointCloudLayer *>( output ) )
        {
          return true;
        }
      }
      return false;
    }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      QgsProcessingProvider *provider = QgsApplication::processingRegistry()->providerById( context.providerId() );
      const QgsProcessingAlgorithm *algorithm = provider->algorithm( context.algorithmName() );
      if ( !algorithm )
        return;

      QgsProcessingDefaultStyleDialog dialog( algorithm, context.parentWidget() );
      dialog.exec();
    }
};


class CreateNewModelAction : public QgsProcessingToolboxAction
{
  public:
    CreateNewModelAction()
      : QgsProcessingToolboxAction( QObject::tr( "Create New Model…" ), QObject::tr( "Tools" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/processingModel.svg"_s ); }

    void trigger( const QgsProcessingActionContext & ) override
    {
      auto dlg = new QgsModelDesignerDialog();
      QObject::connect( dlg, &QgsModelDesignerDialog::modelUpdated, dlg, [] { QgsApplication::processingRegistry()->providerById( QgsProcessing::MODEL_PROVIDER_ID )->refreshAlgorithms(); } );
      dlg->show();
    }
};


class OpenModelFromFileAction : public QgsProcessingToolboxAction
{
  public:
    OpenModelFromFileAction()
      : QgsProcessingToolboxAction( QObject::tr( "Open Existing Model…" ), QObject::tr( "Tools" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/processingModel.svg"_s ); }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      QgsSettings settings;
      const QString lastDir = settings.value( u"Processing/lastModelsDir"_s, QDir::homePath() ).toString();
      const QString fileName = QFileDialog::getOpenFileName( context.parentWidget(), QObject::tr( "Open Model" ), lastDir, QObject::tr( "Processing models (*.model3 *.MODEL3)" ) );
      if ( fileName.isEmpty() )
        return;

      settings.setValue( u"Processing/lastModelsDir"_s, QFileInfo( fileName ).absoluteDir().absolutePath() );

      auto dlg = new QgsModelDesignerDialog();
      dlg->loadModel( fileName );
      dlg->show();
      dlg->activate();
    }
};


class AddModelFromFileAction : public QgsProcessingToolboxAction
{
  public:
    AddModelFromFileAction()
      : QgsProcessingToolboxAction( QObject::tr( "Add Model to Toolbox…" ), QObject::tr( "Tools" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/processingModel.svg"_s ); }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      QgsSettings settings;
      const QString lastDir = settings.value( u"Processing/lastModelsDir"_s, QDir::homePath() ).toString();
      const QString fileName = QFileDialog::getOpenFileName( context.parentWidget(), QObject::tr( "Open Model" ), lastDir, QObject::tr( "Processing models (*.model3 *.MODEL3)" ) );
      if ( fileName.isEmpty() )
        return;

      settings.setValue( u"Processing/lastModelsDir"_s, QFileInfo( fileName ).absoluteDir().absolutePath() );

      auto model = std::make_unique< QgsProcessingModelAlgorithm >();
      if ( !model->fromFile( fileName ) )
      {
        QMessageBox::warning( context.parentWidget(), QObject::tr( "Open Model" ), QObject::tr( "The selected file does not contain a valid model." ) );
        return;
      }

      if ( QgsApplication::instance()->processingRegistry()->algorithmById( u"model:%1"_s.arg( model->id() ) ) )
      {
        QMessageBox::warning( context.parentWidget(), QObject::tr( "Open Model" ), QObject::tr( "A model with the same name already exists." ) );
        return;
      }

      const QString destFolder = QgsProcessingUtils::modelFolders().at( 0 );
      const QString destFilename = QDir( destFolder ).filePath( QFileInfo( fileName ).fileName() );
      if ( QFile::exists( destFilename ) )
      {
        if ( QMessageBox::
               question( context.parentWidget(), QObject::tr( "Open Model" ), QObject::tr( "There is already a model file with the same file name. Overwrite?" ), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No )
             == QMessageBox::StandardButton::No )
        {
          return;
        }
      }

      if ( !QFile::copy( fileName, destFilename ) )
      {
        QMessageBox::warning( context.parentWidget(), QObject::tr( "Open Model" ), QObject::tr( "Could not write to model file %1!" ).arg( QDir::toNativeSeparators( destFilename ) ) );
        return;
      }

      QgsApplication::processingRegistry()->providerById( QgsProcessing::MODEL_PROVIDER_ID )->refreshAlgorithms();
    };
};


class EditModelContextAction : public QgsProcessingToolboxContextAction
{
  public:
    EditModelContextAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Edit Model…" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/processingModel.svg"_s ); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString & ) override { return providerId == QgsProcessing::MODEL_PROVIDER_ID || providerId == QgsProcessing::PROJECT_PROVIDER_ID; }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      QgsProcessingProvider *provider = QgsApplication::processingRegistry()->providerById( context.providerId() );
      const QgsProcessingAlgorithm *algorithm = provider->algorithm( context.algorithmName() );
      if ( !algorithm )
        return;

      const QgsProcessingModelAlgorithm *modelAlgorithm = dynamic_cast< const QgsProcessingModelAlgorithm * >( algorithm );
      if ( !modelAlgorithm )
        return;

      std::unique_ptr< QgsProcessingModelAlgorithm > newModel( qgis::down_cast< QgsProcessingModelAlgorithm * >( modelAlgorithm->create() ) );
      newModel->setSourceFilePath( modelAlgorithm->sourceFilePath() );

      auto dlg = new QgsModelDesignerDialog();
      dlg->setModel( newModel.release() );

      QObject::connect( dlg, &QgsModelDesignerDialog::modelUpdated, dlg, [] { QgsApplication::processingRegistry()->providerById( QgsProcessing::MODEL_PROVIDER_ID )->refreshAlgorithms(); } );
      dlg->show();
      dlg->activate();
    }
};


class DeleteModelContextAction : public QgsProcessingToolboxContextAction
{
  public:
    DeleteModelContextAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Delete Model…" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/processingModel.svg"_s ); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString & ) override { return providerId == QgsProcessing::MODEL_PROVIDER_ID || providerId == QgsProcessing::PROJECT_PROVIDER_ID; }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      const QString algorithmName = context.algorithmName();

      const bool isProjectProvider = context.providerId() == QgsProcessing::PROJECT_PROVIDER_ID;
      QgsProcessingProvider *provider = QgsApplication::processingRegistry()->providerById( context.providerId() );

      const QgsProcessingAlgorithm *algorithm = provider->algorithm( context.algorithmName() );
      if ( !algorithm )
        return;

      const QgsProcessingModelAlgorithm *modelAlgorithm = dynamic_cast< const QgsProcessingModelAlgorithm * >( algorithm );
      if ( !modelAlgorithm )
        return;

      QString msg;
      if ( isProjectProvider )
      {
        msg = QObject::tr( "Are you sure you want to delete this model from the current project?" );
      }
      else
      {
        msg = QObject::tr( "Are you sure you want to delete this model?" );
      }

      if ( QMessageBox::question( context.parentWidget(), QObject::tr( "Delete Model" ), msg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No )
           == QMessageBox::StandardButton::No )
        return;

      if ( isProjectProvider )
      {
        qgis::down_cast< QgsProcessingProjectModelProvider * >( provider )->removeModel( modelAlgorithm );
        QgsProject::instance()->setDirty( true );
      }
      else
      {
        QFile::remove( modelAlgorithm->sourceFilePath() );
        provider->refreshAlgorithms();
      }
    }
};


class ExportModelAsPythonScriptAction : public QgsProcessingToolboxContextAction
{
  public:
    ExportModelAsPythonScriptAction()
      : QgsProcessingToolboxContextAction( QObject::tr( "Export Model as Python Algorithm…" ) )
    {}

    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionSaveAsPython.svg"_s ); }

    bool isCompatibleWithAlgorithm( const QString &providerId, const QString & ) override { return providerId == QgsProcessing::MODEL_PROVIDER_ID || providerId == QgsProcessing::PROJECT_PROVIDER_ID; }

    void trigger( const QgsProcessingActionContext &context ) override
    {
      QgsProcessingProvider *provider = QgsApplication::processingRegistry()->providerById( context.providerId() );
      const QgsProcessingAlgorithm *algorithm = provider->algorithm( context.algorithmName() );
      if ( !algorithm )
        return;

      const QgsProcessingModelAlgorithm *modelAlgorithm = dynamic_cast< const QgsProcessingModelAlgorithm * >( algorithm );
      if ( !modelAlgorithm )
        return;

      QgsProcessingDialogFactory *dialogFactory = QgsGui::processingGuiRegistry()->dialogFactory();
      if ( !dialogFactory )
        return; // should never happen

      QgsProcessingScriptEditorDialog *dialog = dialogFactory->createScriptEditorDialog();
      if ( !dialog )
        return; // should never happen

      const QStringList codeLines = modelAlgorithm->asPythonCode( QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass, 4 );

      dialog->codeEditor()->setText( codeLines.join( '\n' ) );
      dialog->show();
    }
};


//
// QgsAppProcessingUtils
//

QgsAppProcessingUtils::QgsAppProcessingUtils( QgisApp *app )
  : mQgisApp( app )
{}

void QgsAppProcessingUtils::registerActions()
{
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new CreateNewModelAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new OpenModelFromFileAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new AddModelFromFileAction() );

  // toolbox context actions
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QString(), new EditDefaultOutputStyleContextAction() );
  auto separatorAction = new QgsProcessingToolboxContextAction( QString() );
  separatorAction->setIsSeparator( true );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QString(), separatorAction );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QString(), new AddToFavoritesContextAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QString(), new RemoveFromFavoritesContextAction() );

  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new EditModelContextAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new DeleteModelContextAction() );
  separatorAction = new QgsProcessingToolboxContextAction( QString() );
  separatorAction->setIsSeparator( true );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, separatorAction );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new ExportModelAsPythonScriptAction() );
}

void QgsAppProcessingUtils::openModelDesigner()
{
  // QgsModelDesignerDialog has delete on close set:
  auto dlg = new QgsModelDesignerDialog();
  connect( dlg, &QgsModelDesignerDialog::modelUpdated, this, &QgsAppProcessingUtils::updateModels );
  dlg->show();
}

void QgsAppProcessingUtils::initProjectModelProvider()
{
  auto projectModelProvider = std::make_unique< QgsProcessingProjectModelProvider >( QgsProject::instance() );
  QgsApplication::processingRegistry()->addProvider( projectModelProvider.release() );
}

QToolBar *QgsAppProcessingUtils::algorithmsToolBar()
{
  if ( mAlgorithmsToolbar )
    return mAlgorithmsToolbar;

  mAlgorithmsToolbar = mQgisApp->addToolBar( tr( "Processing Algorithms" ) );
  mAlgorithmsToolbar->setObjectName( "ProcessingAlgorithms" );
  mAlgorithmsToolbar->setToolTip( tr( "Processing Algorithms Toolbar" ) );
  return mAlgorithmsToolbar;
}

void QgsAppProcessingUtils::validateDefaultAlgorithmActions()
{
  const QMap<Qgis::ProcessingMenu, QStringList> entries = QgsProcessingDefaultMenus::defaultProcessingMenuEntries();
  for ( auto it = entries.constBegin(); it != entries.constEnd(); ++it )
  {
    const QStringList algorithms = it.value();
    for ( const QString &algorithmId : algorithms )
    {
      const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( algorithmId );
      if ( !algorithm || algorithm->id() != algorithmId )
      {
        QgsMessageLog::logMessage( u"Invalid algorithm ID for menu: %1"_s.arg( algorithmId ), tr( "Processing" ) );
      }
    }
  }
}

QString QgsAppProcessingUtils::menuTitle( Qgis::ProcessingMenu menu )
{
  switch ( menu )
  {
    case Qgis::ProcessingMenu::VectorAnalysis:
      return QCoreApplication::translate( "ProcessingPlugin", "&Analysis Tools" );
    case Qgis::ProcessingMenu::VectorResearch:
      return QCoreApplication::translate( "ProcessingPlugin", "&Research Tools" );
    case Qgis::ProcessingMenu::VectorGeoprocessing:
      return QCoreApplication::translate( "ProcessingPlugin", "&Geoprocessing Tools" );
    case Qgis::ProcessingMenu::VectorGeometry:
      return QCoreApplication::translate( "ProcessingPlugin", "G&eometry Tools" );
    case Qgis::ProcessingMenu::VectorDataManagement:
      return QCoreApplication::translate( "ProcessingPlugin", "&Data Management Tools" );

    case Qgis::ProcessingMenu::VectorGeneral:
      return mQgisApp->vectorMenu()->title();

    case Qgis::ProcessingMenu::RasterProjections:
      return QCoreApplication::translate( "ProcessingPlugin", "&Projection Tools" );
    case Qgis::ProcessingMenu::RasterConversion:
      return QCoreApplication::translate( "ProcessingPlugin", "&Conversion Tools" );
    case Qgis::ProcessingMenu::RasterExtraction:
      return QCoreApplication::translate( "ProcessingPlugin", "&Extraction Tools" );
    case Qgis::ProcessingMenu::RasterAnalysis:
      return QCoreApplication::translate( "ProcessingPlugin", "&Analysis Tools" );
    case Qgis::ProcessingMenu::RasterMiscellaneous:
      return QCoreApplication::translate( "ProcessingPlugin", "&Miscellaneous" );

    case Qgis::ProcessingMenu::RasterGeneral:
      return mQgisApp->rasterMenu()->title();
  }
  BUILTIN_UNREACHABLE
}

QString QgsAppProcessingUtils::legacyMenuTitle( Qgis::ProcessingMenu menu )
{
  // NOTE -- do not change these, see doxygen description for this method!
  switch ( menu )
  {
    case Qgis::ProcessingMenu::VectorAnalysis:
      return QCoreApplication::translate( "ProcessingPlugin", "&Analysis Tools" );
    case Qgis::ProcessingMenu::VectorResearch:
      return QCoreApplication::translate( "ProcessingPlugin", "&Research Tools" );
    case Qgis::ProcessingMenu::VectorGeoprocessing:
      return QCoreApplication::translate( "ProcessingPlugin", "&Geoprocessing Tools" );
    case Qgis::ProcessingMenu::VectorGeometry:
      return QCoreApplication::translate( "ProcessingPlugin", "G&eometry Tools" );
    case Qgis::ProcessingMenu::VectorDataManagement:
      return QCoreApplication::translate( "ProcessingPlugin", "&Data Management Tools" );

    case Qgis::ProcessingMenu::RasterProjections:
      return QCoreApplication::translate( "ProcessingPlugin", "Projections" );
    case Qgis::ProcessingMenu::RasterConversion:
      return QCoreApplication::translate( "ProcessingPlugin", "Conversion" );
    case Qgis::ProcessingMenu::RasterExtraction:
      return QCoreApplication::translate( "ProcessingPlugin", "Extraction" );
    case Qgis::ProcessingMenu::RasterAnalysis:
      return QCoreApplication::translate( "ProcessingPlugin", "Analysis" );
    case Qgis::ProcessingMenu::RasterMiscellaneous:
      return QCoreApplication::translate( "ProcessingPlugin", "Miscellaneous" );

    case Qgis::ProcessingMenu::VectorGeneral:
      return mQgisApp->vectorMenu()->title();

    case Qgis::ProcessingMenu::RasterGeneral:
      return mQgisApp->rasterMenu()->title();
  }
  BUILTIN_UNREACHABLE
}

QMenu *QgsAppProcessingUtils::parentMenu( Qgis::ProcessingMenu menu )
{
  switch ( menu )
  {
    case Qgis::ProcessingMenu::VectorAnalysis:
    case Qgis::ProcessingMenu::VectorResearch:
    case Qgis::ProcessingMenu::VectorGeoprocessing:
    case Qgis::ProcessingMenu::VectorGeometry:
    case Qgis::ProcessingMenu::VectorDataManagement:
    case Qgis::ProcessingMenu::VectorGeneral:
      return mQgisApp->vectorMenu();
    case Qgis::ProcessingMenu::RasterProjections:
    case Qgis::ProcessingMenu::RasterConversion:
    case Qgis::ProcessingMenu::RasterExtraction:
    case Qgis::ProcessingMenu::RasterAnalysis:
    case Qgis::ProcessingMenu::RasterMiscellaneous:
    case Qgis::ProcessingMenu::RasterGeneral:
      return mQgisApp->rasterMenu();
  }
  BUILTIN_UNREACHABLE
}

QMenu *QgsAppProcessingUtils::processingToolMenu( Qgis::ProcessingMenu menu )
{
  QMenu *parentMenu = QgsAppProcessingUtils::parentMenu( menu );
  switch ( menu )
  {
    case Qgis::ProcessingMenu::RasterGeneral:
    case Qgis::ProcessingMenu::VectorGeneral:
      // these are top-level menus, so we return them directly
      return parentMenu;

    case Qgis::ProcessingMenu::VectorAnalysis:
    case Qgis::ProcessingMenu::VectorResearch:
    case Qgis::ProcessingMenu::VectorGeoprocessing:
    case Qgis::ProcessingMenu::VectorGeometry:
    case Qgis::ProcessingMenu::VectorDataManagement:
    case Qgis::ProcessingMenu::RasterProjections:
    case Qgis::ProcessingMenu::RasterConversion:
    case Qgis::ProcessingMenu::RasterExtraction:
    case Qgis::ProcessingMenu::RasterAnalysis:
    case Qgis::ProcessingMenu::RasterMiscellaneous:
      break;
  }

  const QString menuName = menuTitle( menu );
  // identify menus by a stable, static, non-translated string:
  const QString menuId = qgsEnumValueToKey( menu );
  const QList< QAction * > menuActions = parentMenu->actions();
  for ( QAction *action : menuActions )
  {
    if ( QMenu *subMenu = action->menu() )
    {
      if ( subMenu->objectName() == menuId )
      {
        return subMenu;
      }
    }
  }

  auto subMenu = new QMenu( menuName, parentMenu );
  subMenu->setObjectName( menuId );
  QgsAppMenuUtils::insertSubmenuAlphabeticallyToMenu( parentMenu, subMenu );
  return subMenu;
}

QString QgsAppProcessingUtils::algorithmActionText( const QgsProcessingAlgorithm *algorithm )
{
  QString algTitle;
  if ( QgsGui::higFlags().testFlag( QgsGui::HigFlag::HigMenuTextIsTitleCase ) && !( algorithm->flags().testFlag( Qgis::ProcessingAlgorithmFlag::DisplayNameIsLiteral ) ) )
  {
    algTitle = QgsStringUtils::capitalize( algorithm->displayName(), Qgis::Capitalization::TitleCase );
  }
  else
  {
    algTitle = algorithm->displayName();
  }
  return algTitle + QCoreApplication::translate( "Processing", "…" );
}

QList<QAction *> QgsAppProcessingUtils::createAlgorithmActions()
{
  // remove any existing actions first, and then recreate all
  qDeleteAll( mAlgorithmActions );
  mAlgorithmActions.clear();

  QgsSettings settings;
  const QList< const QgsProcessingAlgorithm * > allAlgorithms = QgsApplication::processingRegistry()->algorithms();
  const QMap< Qgis::ProcessingMenu, QStringList > defaultProcessingMenuEntries = QgsProcessingDefaultMenus::defaultProcessingMenuEntries();

  QList< QAction * > actions;
  for ( const QgsProcessingAlgorithm *algorithm : allAlgorithms )
  {
    const QString id = algorithm->id();
    const QString menuSetting = u"Processing/Configuration/MENU_%1"_s.arg( id );
    const bool hasSetting = settings.contains( menuSetting );
    QString algMenu;
    QMenu *menu = nullptr;
    if ( !hasSetting )
    {
      // when no setting exists, we use the default menu configuration for this algorithm
      for ( auto it = defaultProcessingMenuEntries.constBegin(); it != defaultProcessingMenuEntries.constEnd(); ++it )
      {
        if ( it.value().contains( id ) )
        {
          menu = processingToolMenu( it.key() );
          break;
        }
      }
    }
    else
    {
      algMenu = settings.value( menuSetting ).toString();
      // TODO -- respect user settings
    }

    const bool addToToolbar = settings.value( u"Processing/Configuration/BUTTON_%1"_s.arg( id ) ).toBool();
    const QString iconPathSetting = settings.value( u"Processing/Configuration/ICON_%1"_s.arg( id ) ).toString();

    if ( menu || addToToolbar )
    {
      QAction *algorithmAction = createActionForAlgorithm( algorithm );
      if ( menu )
      {
        QgsAppMenuUtils::insertActionAlphabeticallyToMenu( menu, algorithmAction );
      }
      if ( addToToolbar )
      {
        algorithmsToolBar()->addAction( algorithmAction );
      }
      mAlgorithmActions.append( algorithmAction );
    }
  }
  return mAlgorithmActions;
}

void QgsAppProcessingUtils::addAlgorithmsToDefaultToolbars()
{
  QToolBar *selectionToolBar = mQgisApp->selectionToolBar();
  auto toolbutton = new QToolButton( selectionToolBar );
  toolbutton->setPopupMode( QToolButton::ToolButtonPopupMode::MenuButtonPopup );
  QAction *toolButtonAction = selectionToolBar->addWidget( toolbutton );
  toolButtonAction->setObjectName( "selectByToolButton" );

  auto addToolBarButton = [this]( QToolButton *toolButton, int index, const QString &algorithmId ) {
    const QgsProcessingAlgorithm *algorithm = QgsApplication::processingRegistry()->algorithmById( algorithmId );
    if ( !algorithm )
      return;

    QAction *action = createActionForAlgorithm( algorithm );
    toolButton->addAction( action );
    if ( index == 0 )
    {
      toolButton->setDefaultAction( action );
    }
  };

  int index = 0;
  for ( const QString &algorithmId : { u"native:selectbylocation"_s, u"native:selectwithindistance"_s } )
  {
    addToolBarButton( toolbutton, index, algorithmId );
    index++;
  }
}

void QgsAppProcessingUtils::updateModels()
{
  if ( QgsProcessingProvider *modelProvider = QgsApplication::processingRegistry()->providerById( QgsProcessing::MODEL_PROVIDER_ID ) )
  {
    modelProvider->refreshAlgorithms();
  }
}

QAction *QgsAppProcessingUtils::createActionForAlgorithm( const QgsProcessingAlgorithm *algorithm, const QString &iconPath )
{
  const QString id = algorithm->id();
  auto algorithmAction = new QAction( algorithmActionText( algorithm ), this );
  QIcon icon;
  if ( !iconPath.isEmpty() )
  {
    icon = QIcon( iconPath );
  }
  algorithmAction->setToolTip( algorithmActionText( algorithm ) );
  algorithmAction->setIcon( !icon.isNull() ? icon : algorithm->icon() );
  algorithmAction->setData( id );
  algorithmAction->setObjectName( u"mProcessingUserMenu_%1"_s.arg( id ) );

  connect( algorithmAction, &QAction::triggered, this, [id] { QgsGui::instance()->emitExecuteAlgorithm( id ); } );

  return algorithmAction;
}
