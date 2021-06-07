/***************************************************************************
  qgspdaldataitemguiprovider.cpp
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgspdaldataitemguiprovider.h"

#include "qgsmanageconnectionsdialog.h"
#include "qgspdaldataitems.h"
#include "qgspdalprovider.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsPdalDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  Q_UNUSED( item )
  Q_UNUSED( menu )
  Q_UNUSED( context )
}
