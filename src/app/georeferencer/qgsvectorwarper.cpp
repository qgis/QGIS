/***************************************************************************
     qgsvectorwarper.cpp
     --------------------------------------
    Date                 :
    Copyright            :
    Email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>
#include <cstdio>

#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal.h>
#include <ogr_spatialref.h>

#include <QFile>
#include <QProgressDialog>

#include "qgsvectorwarper.h"
#include "qgsrunprocess.h"




QgsVectorWarper::QgsVectorWarper()
{
}

bool QgsVectorWarper::executeGDALCommand( const QString &fused_command )
{
  QString command;
  QStringList arguments;
  command, arguments = QgsRunProcess::splitCommand( fused_command );
  if ( arguments.length() == 1 )
    arguments.replace( 0, arguments.first().replace( "ogr2ogr ","") );
  else if ( arguments.first() == QString( "ogr2ogr" ) )
    arguments.removeFirst();

  if ( command.isEmpty() )
    command = QString( "ogr2ogr" );

  QgsBlockingProcess *proc = new QgsBlockingProcess( command, arguments );
  //proc.setStdOutHandler(on_stdout);
  //proc.setStdErrHandler(on_stderr);
  proc->run();
  if ( proc->exitStatus() == QProcess::NormalExit )
    return true;
  else
    return false;
}
