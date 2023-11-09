/***************************************************************************
    qgslayoutqptdrophandler.cpp
    ------------------------------
    begin                : December 2017
    copyright            : (C) 2017 by nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutqptdrophandler.h"
#include "qgslayoutdesignerinterface.h"
#include "qgslayout.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"
#include "qgslayoutview.h"
#include <QMessageBox>

QgsLayoutQptDropHandler::QgsLayoutQptDropHandler( QObject *parent )
  : QgsLayoutCustomDropHandler( parent )
{

}

bool QgsLayoutQptDropHandler::handleFileDrop( QgsLayoutDesignerInterface *iface, QPointF, const QString &file )
{
  const QFileInfo fi( file );
  if ( fi.suffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) != 0 )
    return false;

  QFile templateFile( file );
  if ( !templateFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( iface->view(), tr( "Load from Template" ), tr( "Could not read template file." ) );
    return true;
  }

  QDomDocument templateDoc;
  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( templateDoc.setContent( &templateFile ) )
  {
    bool ok = false;
    const QList< QgsLayoutItem * > items = iface->layout()->loadFromTemplate( templateDoc, context, false, &ok );
    if ( !ok )
    {
      QMessageBox::warning( iface->view(), tr( "Load from Template" ), tr( "Could not read template file." ) );
      return true;
    }
    else
    {
      whileBlocking( iface->layout() )->deselectAll();
      iface->selectItems( items );
    }
  }

  return true;
}
