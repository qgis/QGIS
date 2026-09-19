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
#include "qgscodeeditorpython.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsmodeldesignerdialog.h"
#include "qgsprocessingguiregistry.h"
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

void QgsAppProcessingUtils::registerActions()
{
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new CreateNewModelAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new OpenModelFromFileAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxAction( QgsProcessing::MODEL_PROVIDER_ID, new AddModelFromFileAction() );

  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new EditModelContextAction() );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new DeleteModelContextAction() );
  auto separatorAction = new QgsProcessingToolboxContextAction( QString() );
  separatorAction->setIsSeparator( true );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, separatorAction );
  QgsGui::processingGuiRegistry()->registerProviderToolboxContextAction( QgsProcessing::MODEL_PROVIDER_ID, new ExportModelAsPythonScriptAction() );
}

void QgsAppProcessingUtils::initProjectModelProvider()
{
  auto projectModelProvider = std::make_unique< QgsProcessingProjectModelProvider >( QgsProject::instance() );
  QgsApplication::processingRegistry()->addProvider( projectModelProvider.release() );
}
