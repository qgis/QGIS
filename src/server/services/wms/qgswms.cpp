/***************************************************************************
                              qgswms.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
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

#include "qgsmodule.h"
#include "qgswmsutils.h"
#include "qgsdxfwriter.h"
#include "qgswmsgetcapabilities.h"
#include "qgswmsgetmap.h"
#include "qgswmsgetstyles.h"
#include "qgswmsgetcontext.h"
#include "qgswmsgetschemaextension.h"
#include "qgswmsgetprint.h"
#include "qgswmsgetfeatureinfo.h"
#include "qgswmsdescribelayer.h"
#include "qgswmsgetlegendgraphics.h"

#define QSTR_COMPARE( str, lit )\
  (str.compare( QStringLiteral( lit ), Qt::CaseInsensitive ) == 0)

namespace QgsWms
{

  /**
   * \ingroup server
   * \class QgsWms::Service
   * \brief OGC web service specialized for WMS
   * \since QGIS 3.0
   */
  class Service: public QgsService
  {
    public:

      /**
       * Constructor for WMS service.
       * \param version Version of the WMS service.
       * \param serverIface Interface for plugins.
       */
      Service( const QString &version, QgsServerInterface *serverIface )
        : mVersion( version )
        , mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WMS" ); }
      QString version() const override { return mVersion; }

      bool allowMethod( QgsServerRequest::Method method ) const override
      {
        return method == QgsServerRequest::GetMethod;
      }

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response,
                           const QgsProject *project ) override
      {
        QgsServerRequest::Parameters params = request.parameters();
        QString versionString = params.value( "VERSION" );
        if ( versionString.isEmpty() )
        {
          //WMTVER needs to be supported by WMS 1.1.1 for backwards compatibility with WMS 1.0.0
          versionString = params.value( "WMTVER" );
        }

        // Set the default version
        const bool valid = versionString.compare( "1.1.1" ) == 0 || versionString.compare( "1.3.0" ) == 0;
        if ( versionString.isEmpty() || !valid )
        {
          versionString = mVersion;
        }

        // Get the request
        QString req = params.value( QStringLiteral( "REQUEST" ) );
        if ( req.isEmpty() )
        {
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Please check the value of the REQUEST parameter" ) );
        }

        if ( ( QSTR_COMPARE( mVersion, "1.1.1" ) && QSTR_COMPARE( req, "capabilities" ) )
             || QSTR_COMPARE( req, "GetCapabilities" ) )
        {
          writeGetCapabilities( mServerIface, project, versionString, request, response, false );
        }
        else if ( QSTR_COMPARE( req, "GetProjectSettings" ) )
        {
          //getProjectSettings extends WMS 1.3.0 capabilities
          versionString = QStringLiteral( "1.3.0" );
          writeGetCapabilities( mServerIface, project, versionString, request, response, true );
        }
        else if ( QSTR_COMPARE( req, "GetMap" ) )
        {
          QString format = params.value( QStringLiteral( "FORMAT" ) );
          if QSTR_COMPARE( format, "application/dxf" )
          {
            writeAsDxf( mServerIface, project, versionString, request, response );
          }
          else
          {
            writeGetMap( mServerIface, project, versionString, request, response );
          }
        }
        else if ( QSTR_COMPARE( req, "GetFeatureInfo" ) )
        {
          writeGetFeatureInfo( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetContext" ) )
        {
          writeGetContext( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetSchemaExtension" ) )
        {
          writeGetSchemaExtension( mServerIface, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetStyle" ) )
        {
          writeGetStyle( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetStyles" ) )
        {
          writeGetStyles( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "DescribeLayer" ) )
        {
          writeDescribeLayer( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetLegendGraphic" ) || QSTR_COMPARE( req, "GetLegendGraphics" ) )
        {
          writeGetLegendGraphics( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetPrint" ) )
        {
          writeGetPrint( mServerIface, project, versionString, request, response );
        }
        else
        {
          // Operation not supported
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QString( "Request %1 is not supported" ).arg( req ) );
        }
      }

    private:
      QString mVersion;
      QgsServerInterface *mServerIface = nullptr;
  };
} // namespace QgsWms

/**
 * \ingroup server
 * \class QgsWmsModule
 * \brief Service module specialized for WMS
 * \since QGIS 3.0
 */
class QgsWmsModule: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsDebugMsg( "WMSModule::registerSelf called" );
      registry.registerService( new  QgsWms::Service( "1.3.0", serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWmsModule sModule;
  return &sModule;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
