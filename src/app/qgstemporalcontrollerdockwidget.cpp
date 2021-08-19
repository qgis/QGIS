/***************************************************************************
                         qgstemporalcontrollerdockwidget.cpp
                         ------------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalcontrollerdockwidget.h"
#include "qgstemporalcontrollerwidget.h"
#include "qgspanelwidgetstack.h"
#include "qgsanimationexportdialog.h"
#include "qgsdecorationitem.h"
#include "qgsmapcanvas.h"
#include "qgsmapdecoration.h"

#include "qgstemporalutils.h"
#include "qgstaskmanager.h"
#include "qgsproxyprogresstask.h"
#include "qgsmessagebar.h"

#include <QProgressDialog>
#include <QMessageBox>
#include <QUrl>

QgsTemporalControllerDockWidget::QgsTemporalControllerDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setWindowTitle( name );
  mControllerWidget = new QgsTemporalControllerWidget();
  mControllerWidget->setDockMode( true );

  QgsPanelWidgetStack *stack = new QgsPanelWidgetStack();
  stack->setMainPanel( mControllerWidget );
  setWidget( stack );

  connect( mControllerWidget, &QgsTemporalControllerWidget::exportAnimation, this, &QgsTemporalControllerDockWidget::exportAnimation );
}

QgsTemporalController *QgsTemporalControllerDockWidget::temporalController()
{
  return mControllerWidget->temporalController();
}

void QgsTemporalControllerDockWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( canvas && canvas->viewport() )
    canvas->viewport()->installEventFilter( this );
}

bool QgsTemporalControllerDockWidget::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::Wheel )
  {
    QWheelEvent *wheelEvent = dynamic_cast< QWheelEvent * >( event );
    // handle horizontal wheel events by scrubbing timeline
    if ( wheelEvent->angleDelta().x() != 0 )
    {
      const int step = -wheelEvent->angleDelta().x() / 120.0;
      mControllerWidget->temporalController()->setCurrentFrameNumber( mControllerWidget->temporalController()->currentFrameNumber() + step );
      return true;
    }
  }
  return QgsDockWidget::eventFilter( object, event );
}

void QgsTemporalControllerDockWidget::exportAnimation()
{
  QgsAnimationExportDialog *dlg = new QgsAnimationExportDialog( this, QgisApp::instance()->mapCanvas(), QgisApp::instance()->activeDecorations() );
  connect( dlg, &QgsAnimationExportDialog::startExport, this, [ = ]
  {
    QgsMapSettings s = QgisApp::instance()->mapCanvas()->mapSettings();
    dlg->applyMapSettings( s );

    const QgsDateTimeRange animationRange = dlg->animationRange();
    const QgsInterval frameDuration = dlg->frameInterval();
    const QString outputDir = dlg->outputDirectory();
    const QString fileNameExpression = dlg->fileNameExpression();

    dlg->hide();

    QgsFeedback progressFeedback;
    QgsScopedProxyProgressTask task( tr( "Exporting animation" ) );

    QProgressDialog progressDialog( tr( "Exporting animation…" ), tr( "Abort" ), 0, 100, this );
    progressDialog.setWindowTitle( tr( "Exporting Animation" ) );
    progressDialog.setWindowModality( Qt::WindowModal );
    QString error;

    connect( &progressFeedback, &QgsFeedback::progressChanged, this,
             [&progressDialog, &progressFeedback, &task]
    {
      progressDialog.setValue( static_cast<int>( progressFeedback.progress() ) );
      task.setProgress( progressFeedback.progress() );
      QCoreApplication::processEvents();
    } );

    connect( &progressDialog, &QProgressDialog::canceled, &progressFeedback, &QgsFeedback::cancel );

    QList<QgsMapDecoration *> decorations;
    if ( dlg->drawDecorations() )
      decorations = QgisApp::instance()->activeDecorations();

    QgsTemporalUtils::AnimationExportSettings animationSettings;
    animationSettings.frameDuration = frameDuration;
    animationSettings.animationRange = animationRange;
    animationSettings.outputDirectory = outputDir;
    animationSettings.fileNameTemplate = fileNameExpression;
    animationSettings.decorations = decorations;

    const bool success = QgsTemporalUtils::exportAnimation( s, animationSettings, error, &progressFeedback );

    progressDialog.hide();
    if ( !success )
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Export Animation" ), error, Qgis::MessageLevel::Critical );
    }
    else
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Export Animation" ),
          tr( "Successfully exported animation to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( outputDir ).toString(), QDir::toNativeSeparators( outputDir ) ),
          Qgis::MessageLevel::Success, 0 );
    }
  } );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}
