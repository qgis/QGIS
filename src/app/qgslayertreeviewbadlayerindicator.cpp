/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.cpp - QgsLayerTreeViewBadLayerIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewbadlayerindicator.h"
#include "moc_qgslayertreeviewbadlayerindicator.cpp"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"
#include "qgsbrowserproxymodel.h"
#include "qgsmessageviewer.h"

#include <functional>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

QgsLayerTreeViewBadLayerIndicatorProvider::QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewBadLayerIndicatorProvider::reportLayerError( const QString &error, QgsMapLayer *layer )
{
  for ( const Error &e : std::as_const( mErrors ) )
  {
    // don't report identical errors
    if ( e.layer == layer && error == e.error )
      return;
  }
  mErrors.append( Error( error, layer ) );
  updateLayerIndicator( layer );
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();

  if ( !layer )
    return;

  if ( !layer->isValid() )
    emit requestChangeDataSource( layer );
  else
  {
    QStringList thisLayerErrors;
    QList<Error> newErrors;
    for ( const Error &error : std::as_const( mErrors ) )
    {
      if ( error.layer != layer )
        newErrors.append( error );
      else
        thisLayerErrors.append( error.error );
    }
    mErrors = newErrors;
    updateLayerIndicator( layer );

    if ( !thisLayerErrors.empty() )
    {
      // show error in a dialog (delete on close is set automatically for QgsMessageViewer!)
      QgsMessageViewer *m = new QgsMessageViewer( QgisApp::instance() );
      m->setWindowTitle( tr( "Layer Error" ) );
      if ( thisLayerErrors.count() == 1 )
        m->setMessageAsPlainText( thisLayerErrors.at( 0 ) );
      else
      {
        QString message = QStringLiteral( "<ul>" );
        for ( const QString &e : thisLayerErrors )
          message += QStringLiteral( "<li>%1</li>" ).arg( e );
        message += QLatin1String( "</ul>" );
        m->setMessageAsHtml( message );
      }
      m->exec();
    }
  }
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::iconName( QgsMapLayer *layer )
{
  if ( !layer->isValid() )
    return QStringLiteral( "/mIndicatorBadLayer.svg" );
  else
    return QStringLiteral( "/mIndicatorLayerError.svg" );
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  if ( !layer->isValid() )
    return tr( "<b>Unavailable layer!</b><br>Layer data source could not be found. Click to set a new data source" );
  else
  {
    for ( const Error &error : std::as_const( mErrors ) )
    {
      if ( error.layer == layer )
        return error.error;
    }
  }
  return QString();
}

bool QgsLayerTreeViewBadLayerIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer->isValid() )
    return true;

  for ( const Error &error : std::as_const( mErrors ) )
  {
    if ( error.layer == layer )
      return true;
  }
  return false;
}
