/***************************************************************************
    qgsrelationcontext.cpp
     --------------------------------------
    Date                 : 20.12.2019
    Copyright            : (C) 2019 David Marteau
    Email                : dmarteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrelationcontext.h"

#include "qgsmessagelog.h"
#include "qgsproject.h"

QgsRelationContext::QgsRelationContext()
{
  QgsMessageLog::logMessage( "QgsRelationContext constructed without specified project. This will be removed in QGIS 5.0.", "QgsRelationContext", Qgis::Warning );
}

QgsRelationContext::QgsRelationContext( QgsProject *project )
  : mProject( project )
{}

QgsRelationContext::~QgsRelationContext() = default;


const QgsProject *QgsRelationContext::project() const
{
  if ( mProject )
  {
    return mProject;
  }

  // TODO QGIS 5.0 -- Remove the fallback to the QgsProject instance
  // Fallback to qgis instance
  return QgsProject::instance(); // skip-keyword-check
}
