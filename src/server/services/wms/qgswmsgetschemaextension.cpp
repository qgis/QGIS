/***************************************************************************
                              qgswmsgetschemaextension.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmsutils.h"
#include "qgswmsgetschemaextension.h"
#include "qgsapplication.h"

#include <QDir>
#include <QFileInfo>
#include <QTextStream>

namespace QgsWms
{

  void writeGetSchemaExtension( QgsServerResponse &response )
  {
    const QDir resourcesDir = QFileInfo( QgsApplication::serverResourcesPath() ).absoluteDir();
    const QFileInfo xsdFileInfo( resourcesDir, QStringLiteral( "schemaExtension.xsd" ) );
    QString schema_str = QStringLiteral( "<?xml version='1.0'?>" );

    if ( !xsdFileInfo.exists() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' does not exist" ),
                                 QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    }
    else
    {
      QFile file( xsdFileInfo.absoluteFilePath() );
      if ( file.open( QFile::ReadOnly | QFile::Text ) )
      {
        QTextStream in( &file );
        schema_str = in.readAll();
        file.close();
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' not readable" ),
                                   QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
      }
    }
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( schema_str );
  }

} // namespace QgsWms
