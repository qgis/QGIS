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

namespace QgsWms
{

  void writeGetSchemaExtension( QgsServerInterface *serverIface, const QString &version,
                                const QgsServerRequest &request, QgsServerResponse &response )
  {
    QDomDocument doc = getSchemaExtension( serverIface, version, request );
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  QDomDocument getSchemaExtension( QgsServerInterface *serverIface, const QString &version,
                                   const QgsServerRequest &request )
  {
    Q_UNUSED( version );
    Q_UNUSED( serverIface );

    QgsServerRequest::Parameters parameters = request.parameters();

    QDomDocument xsdDoc;

    QDir resourcesDir = QFileInfo( QgsApplication::serverResourcesPath() ).absoluteDir();
    QFileInfo xsdFileInfo( resourcesDir, QStringLiteral( "schemaExtension.xsd" ) );

    if ( !xsdFileInfo.exists() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' does not exist" ),
                                 QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    QString xsdFilePath = xsdFileInfo.absoluteFilePath();
    QFile xsdFile( xsdFilePath );
    if ( !xsdFile.exists() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' does not exist" ),
                                 QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    if ( !xsdFile.open( QIODevice::ReadOnly ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, cannot open xsd file 'schemaExtension.xsd' does not exist" ),
                                 QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    QString errorMsg;
    int line, column;
    if ( !xsdDoc.setContent( &xsdFile, true, &errorMsg, &line, &column ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing file 'schemaExtension.xsd" ) +
                                 QStringLiteral( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ),
                                 QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
    }

    return xsdDoc;
  }

} // namespace QgsWms
