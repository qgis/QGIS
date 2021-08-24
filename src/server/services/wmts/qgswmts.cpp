/***************************************************************************
                              qgswmts.cpp
                              -------------------------
  begin                : July 23 , 2018
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgswmtsutils.h"
#include "qgswmtsgetcapabilities.h"
#include "qgswmtsgettile.h"
#include "qgswmtsgetfeatureinfo.h"

#define QSTR_COMPARE( str, lit )\
  (str.compare( QLatin1String( lit ), Qt::CaseInsensitive ) == 0)

namespace QgsWmts
{

  /**
   * \ingroup server
   * \class QgsWmts::Service
   * \brief OGC web service specialized for WMTS
   * \since QGIS 3.4
   */
  class Service: public QgsService
  {
    public:

      /**
       * Constructor for WMTS service.
       * \param serverIface Interface for plugins.
       */
      Service( QgsServerInterface *serverIface )
        : mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WMTS" ); }
      QString version() const override { return implementationVersion(); }

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response,
                           const QgsProject *project ) override
      {
        Q_UNUSED( project )

        const QgsWmtsParameters params( QUrlQuery( request.url() ) );

        // Set the default version
        QString versionString = params.version();
        if ( versionString.isEmpty() )
        {
          versionString = version(); // defined in qgswfsutils.h
        }

        // Get the request
        const QString req = params.value( QgsServerParameter::name( QgsServerParameter::REQUEST ) );
        if ( req.isEmpty() )
        {
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Please add or check the value of the REQUEST parameter" ), 501 );
        }

        if ( QSTR_COMPARE( req, "GetCapabilities" ) )
        {
          writeGetCapabilities( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetTile" ) )
        {
          writeGetTile( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetFeatureInfo" ) )
        {
          writeGetFeatureInfo( mServerIface, project, versionString, request, response );
        }
        else
        {
          // Operation not supported
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Request %1 is not supported" ).arg( req ), 501 );
        }
      }

    private:
      QgsServerInterface *mServerIface = nullptr;
  };


} // namespace QgsWmts

/**
 * \ingroup server
 * \class QgsWmtsModule
 * \brief Service module specialized for WMTS
 * \since QGIS 3.4
 */
class QgsWmtsModule: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsDebugMsg( QStringLiteral( "WMTSModule::registerSelf called" ) );
      registry.registerService( new  QgsWmts::Service( serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWmtsModule module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
