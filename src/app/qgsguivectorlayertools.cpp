/***************************************************************************
    qgsfeaturefactory.cpp
     --------------------------------------
    Date                 : 30.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QToolButton>

#include "qgsguivectorlayertools.h"
#include "moc_qgsguivectorlayertools.cpp"

#include "qgsavoidintersectionsoperation.h"
#include "qgisapp.h"
#include "qgsfeatureaction.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessageviewer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectorlayertoolscontext.h"

bool QgsGuiVectorLayerTools::addFeatureV2( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature, const QgsVectorLayerToolsContext &context ) const
{
  QgsFeature *f = feature;
  if ( !feature )
    f = new QgsFeature();

  f->setGeometry( defaultGeometry );
  QgsFeatureAction *a = new QgsFeatureAction( tr( "Add feature" ), *f, layer, QUuid(), -1, context.parentWidget() );
  a->setForceSuppressFormPopup( forceSuppressFormPopup() );
  connect( a, &QgsFeatureAction::addFeatureFinished, a, &QObject::deleteLater );
  const QgsFeatureAction::AddFeatureResult result = a->addFeature( defaultValues, context.showModal(), std::unique_ptr<QgsExpressionContextScope>( context.additionalExpressionContextScope() ? new QgsExpressionContextScope( *context.additionalExpressionContextScope() ) : nullptr ), context.hideParent() );
  if ( !feature )
    delete f;

  switch ( result )
  {
    case QgsFeatureAction::AddFeatureResult::Success:
    case QgsFeatureAction::AddFeatureResult::Pending:
      return true;

    case QgsFeatureAction::AddFeatureResult::LayerStateError:
    case QgsFeatureAction::AddFeatureResult::Canceled:
    case QgsFeatureAction::AddFeatureResult::FeatureError:
      return false;
  }
  BUILTIN_UNREACHABLE
}

bool QgsGuiVectorLayerTools::startEditing( QgsVectorLayer *layer ) const
{
  if ( !layer )
  {
    return false;
  }

  const bool res = true;

  if ( !layer->isEditable() && !layer->readOnly() )
  {
    if ( !layer->supportsEditing() )
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Start editing failed" ), tr( "Provider cannot be opened for editing" ), Qgis::MessageLevel::Info );
      return false;
    }

    layer->startEditing();
  }

  return res;
}

bool QgsGuiVectorLayerTools::saveEdits( QgsVectorLayer *layer ) const
{
  bool res = true;

  if ( layer->isModified() )
  {
    if ( !layer->commitChanges() )
    {
      commitError( layer );
      // Leave the in-memory editing state alone,
      // to give the user a chance to enter different values
      // and try the commit again later
      res = false;
    }
    layer->startEditing();
  }
  else //layer not modified
  {
    res = true;
  }
  return res;
}

bool QgsGuiVectorLayerTools::stopEditing( QgsVectorLayer *layer, bool allowCancel ) const
{
  bool res = true;

  if ( layer->isModified() )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if ( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch ( QMessageBox::question( nullptr, tr( "Stop Editing" ), tr( "Do you want to save the changes to layer %1?" ).arg( layer->name() ), buttons ) )
    {
      case QMessageBox::Cancel:
        res = false;
        break;

      case QMessageBox::Save:
        if ( !layer->commitChanges() )
        {
          commitError( layer );
          // Leave the in-memory editing state alone,
          // to give the user a chance to enter different values
          // and try the commit again later
          res = false;
        }

        layer->triggerRepaint();
        break;

      case QMessageBox::Discard:
        QgisApp::instance()->freezeCanvases();
        if ( !layer->rollBack() )
        {
          QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "Problems during roll back" ), Qgis::MessageLevel::Critical );
          res = false;
        }
        QgisApp::instance()->freezeCanvases( false );

        layer->triggerRepaint();
        break;

      default:
        break;
    }
  }
  else //layer not modified
  {
    QgisApp::instance()->freezeCanvases( true );
    layer->rollBack();
    QgisApp::instance()->freezeCanvases( false );
    res = true;
    layer->triggerRepaint();
  }

  return res;
}

bool QgsGuiVectorLayerTools::copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double dx, double dy, QString *errorMsg, const bool topologicalEditing, QgsVectorLayer *topologicalLayer, QString *childrenInfoMsg ) const
{
  bool res = QgsVectorLayerTools::copyMoveFeatures( layer, request, dx, dy, errorMsg, topologicalEditing, topologicalLayer, childrenInfoMsg );

  if ( res && layer->geometryType() == Qgis::GeometryType::Polygon )
  {
    res = avoidIntersection( layer, request, errorMsg );
  }

  return res;
}

bool QgsGuiVectorLayerTools::avoidIntersection( QgsVectorLayer *layer, QgsFeatureRequest &request, QString *errorMsg ) const
{
  QgsAvoidIntersectionsOperation avoidIntersections;

  QgsFeatureIterator fi = layer->getFeatures( request );
  QgsFeature f;

  const QHash<QgsVectorLayer *, QSet<QgsFeatureId>> ignoreFeatures { { layer, request.filterFids() } };

  while ( fi.nextFeature( f ) )
  {
    QgsGeometry geom = f.geometry();
    const QgsFeatureId id = f.id();

    const QgsAvoidIntersectionsOperation::Result res = avoidIntersections.apply( layer, id, geom, ignoreFeatures );

    if ( res.operationResult == Qgis::GeometryOperationResult::InvalidInputGeometryType || geom.isEmpty() )
    {
      if ( errorMsg )
      {
        *errorMsg = ( geom.isEmpty() ) ? tr( "The feature cannot be moved because 1 or more resulting geometries would be empty" ) : tr( "An error was reported during intersection removal" );
      }

      return false;
    }
    layer->changeGeometry( id, geom );
  }

  return true;
}

void QgsGuiVectorLayerTools::commitError( QgsVectorLayer *vlayer ) const
{
  QgsMessageViewer *mv = new QgsMessageViewer();
  mv->setWindowTitle( tr( "Commit Errors" ) );
  mv->setMessageAsPlainText( tr( "Could not commit changes to layer %1" ).arg( vlayer->name() ) + "\n\n" + tr( "Errors: %1\n" ).arg( vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );

  QToolButton *showMore = new QToolButton();
  // store pointer to vlayer in data of QAction
  QAction *act = new QAction( showMore );
  act->setData( QVariant::fromValue( vlayer ) );
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
    Qgis::MessageLevel::Warning,
    0,
    QgisApp::instance()->messageBar()
  );
  QgisApp::instance()->messageBar()->pushItem( errorMsg );
}
