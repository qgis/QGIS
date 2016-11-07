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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeatureaction.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessageviewer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


QgsGuiVectorLayerTools::QgsGuiVectorLayerTools()
    : QObject( nullptr )
{}

bool QgsGuiVectorLayerTools::addFeature( QgsVectorLayer* layer, const QgsAttributeMap& defaultValues, const QgsGeometry& defaultGeometry, QgsFeature* feat ) const
{
  QgsFeature* f = feat;
  if ( !feat )
    f = new QgsFeature();

  f->setGeometry( defaultGeometry );
  QgsFeatureAction a( tr( "Add feature" ), *f, layer );
  bool added = a.addFeature( defaultValues );
  if ( !feat )
    delete f;

  return added;
}

bool QgsGuiVectorLayerTools::startEditing( QgsVectorLayer* layer ) const
{
  if ( !layer )
  {
    return false;
  }

  bool res = true;

  if ( !layer->isEditable() && !layer->readOnly() )
  {
    if ( !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Start editing failed" ),
          tr( "Provider cannot be opened for editing" ),
          QgsMessageBar::INFO, QgisApp::instance()->messageTimeout() );
      return false;
    }

    layer->startEditing();
  }

  return res;
}

bool QgsGuiVectorLayerTools::saveEdits( QgsVectorLayer* layer ) const
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

bool QgsGuiVectorLayerTools::copyMoveFeatures( QgsVectorLayer* layer, QgsFeatureRequest& request, double dx, double dy ) const
{
  bool res = false;
  if ( !layer || !layer->isEditable() )
  {
    return false;
  }

  QgsFeatureIterator fi = layer->getFeatures( request );
  QgsFeature f;
  QgsAttributeList pkAttrList = layer->pkAttributeList();

  int browsedFeatureCount = 0;
  int couldNotWriteCount = 0;
  int noGeometryCount = 0;

  QgsFeatureIds fidList;

  while ( fi.nextFeature( f ) )
  {
    browsedFeatureCount++;
    // remove pkey values
    Q_FOREACH ( auto idx, pkAttrList )
    {
      f.setAttribute( idx, QVariant() );
    }
    // translate
    if ( f.hasGeometry() )
    {
      QgsGeometry geom = f.geometry();
      geom.translate( dx, dy );
      f.setGeometry( geom );
#ifdef QGISDEBUG
      const QgsFeatureId  fid = f.id();
#endif
      // paste feature
      if ( !layer->addFeature( f, false ) )
      {
        couldNotWriteCount++;
        QgsDebugMsg( QString( "Could not add new feature. Original copied feature id: %1" ).arg( fid ) );
      }
      else
      {
        fidList.insert( f.id() );
      }
    }
    else
    {
      noGeometryCount++;
    }
  }

  request = QgsFeatureRequest();
  request.setFilterFids( fidList );

  if ( !couldNotWriteCount && !noGeometryCount )
  {
    res = true;
  }
  else
  {
    QString msg = QString( tr( "Only %1 out of %2 features were copied." ) ).arg( browsedFeatureCount - couldNotWriteCount - noGeometryCount, browsedFeatureCount );
    if ( noGeometryCount )
    {
      msg.append( " " );
      msg.append( tr( "Some features have no geometry." ) );
    }
    if ( couldNotWriteCount )
    {
      msg.append( " " );
      msg.append( tr( "Some could not be created on the layer." ) );
    }
    QgisApp::instance()->messageBar()->pushMessage( tr( "Copy and move" ), msg, QgsMessageBar::CRITICAL );
  }
  return res;
}


bool QgsGuiVectorLayerTools::stopEditing( QgsVectorLayer* layer, bool allowCancel ) const
{
  bool res = true;

  if ( layer->isModified() )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if ( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch ( QMessageBox::information( nullptr,
                                       tr( "Stop editing" ),
                                       tr( "Do you want to save the changes to layer %1?" ).arg( layer->name() ),
                                       buttons ) )
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
        QgisApp::instance()->mapCanvas()->freeze( true );
        if ( !layer->rollBack() )
        {
          QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ),
              tr( "Problems during roll back" ),
              QgsMessageBar::CRITICAL );
          res = false;
        }
        QgisApp::instance()->mapCanvas()->freeze( false );

        layer->triggerRepaint();
        break;

      default:
        break;
    }
  }
  else //layer not modified
  {
    QgisApp::instance()->mapCanvas()->freeze( true );
    layer->rollBack();
    QgisApp::instance()->mapCanvas()->freeze( false );
    res = true;
    layer->triggerRepaint();
  }

  return res;
}

void QgsGuiVectorLayerTools::commitError( QgsVectorLayer* vlayer ) const
{
  QgsMessageViewer *mv = new QgsMessageViewer();
  mv->setWindowTitle( tr( "Commit errors" ) );
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
  connect( showMore, SIGNAL( triggered( QAction* ) ), mv, SLOT( exec() ) );
  connect( showMore, SIGNAL( triggered( QAction* ) ), showMore, SLOT( deleteLater() ) );

  // no timeout set, since notice needs attention and is only shown first time layer is labeled
  QgsMessageBarItem *errorMsg = new QgsMessageBarItem(
    tr( "Commit errors" ),
    tr( "Could not commit changes to layer %1" ).arg( vlayer->name() ),
    showMore,
    QgsMessageBar::WARNING,
    0,
    QgisApp::instance()->messageBar() );
  QgisApp::instance()->messageBar()->pushItem( errorMsg );

}
