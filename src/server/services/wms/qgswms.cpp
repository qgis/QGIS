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
#include "qgswmsgetstyle.h"
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

  class Service: public QgsService
  {
    public:
      // Constructor
      Service( const QString& version, QgsServerInterface* serverIface )
          : mVersion( version )
          , mServerIface( serverIface )
      {}

      QString name()    const { return QStringLiteral( "WMS" ); }
      QString version() const { return mVersion; }

      bool allowMethod( QgsServerRequest::Method method ) const
      {
        return method == QgsServerRequest::GetMethod;
      }

      void executeRequest( const QgsServerRequest& request, QgsServerResponse& response,
                           QgsProject* project )
      {
        Q_UNUSED( project );

        // Get the request
        QgsServerRequest::Parameters params = request.parameters();
        QString req = params.value( QStringLiteral( "REQUEST" ) );
        if ( req.isEmpty() )
        {
          writeError( response, QStringLiteral( "OperationNotSupported" ),
                      QStringLiteral( "Please check the value of the REQUEST parameter" ) );
          return;
        }

        if (( QSTR_COMPARE( mVersion, "1.1.1" ) && QSTR_COMPARE( req, "capabilities" ) )
            || QSTR_COMPARE( req, "GetCapabilites" ) )
        {
          //TODO GetCapabilities
          writeGetCapabilities( mServerIface, mVersion, request, response, false );
        }
        else if QSTR_COMPARE( req, "GetProjectSettings" )
        {
          //Ensure that we are supporting 1.3.0
          if ( mVersion.compare( "1.3.0" ) != 0 )
          {
            writeError( response, QStringLiteral( "OperationNotSupported" ),
                        QStringLiteral( "GetProjectSettings is not supported" ) );
            return;
          }
          writeGetCapabilities( mServerIface, mVersion, request, response, true );
          // TODO GetProjectSettings
        }
        else if QSTR_COMPARE( req, "GetMap" )
        {
          QString format = params.value( QStringLiteral( "FORMAT" ) );
          if QSTR_COMPARE( format, "application/dxf" )
          {
            writeAsDxf( mServerIface, mVersion, request, response );
          }
          else
          {
            writeGetMap( mServerIface, mVersion, request, response );
          }
        }
        else if QSTR_COMPARE( req, "GetFeatureInfo" )
        {
          writeGetFeatureInfo( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "GetContext" )
        {
          writeGetContext( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "GetSchemaExtension" )
        {
          writeGetSchemaExtension( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "GetStyle" )
        {
          writeGetStyle( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "GetStyles" )
        {
          writeGetStyles( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "DescribeLayer" )
        {
          writeDescribeLayer( mServerIface, mVersion, request, response );
        }
        else if ( QSTR_COMPARE( req, "GetLegendGraphic" ) || QSTR_COMPARE( req, "GetLegendGraphics" ) )
        {
          writeGetLegendGraphics( mServerIface, mVersion, request, response );
        }
        else if QSTR_COMPARE( req, "GetPrint" )
        {
          writeDescribeLayer( mServerIface, mVersion, request, response );
        }
        else
        {
          // Operation not supported
          writeError( response, QStringLiteral( "OperationNotSupported" ),
                      QString( "Request %1 is not supported" ).arg( req ) );
          return;
        }
      }

    private:
      QString mVersion;
      QgsServerInterface* mServerIface;
  };


} // namespace QgsWms


// Module
class QgsWmsModule: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry& registry, QgsServerInterface* serverIface )
    {
      QgsDebugMsg( "WMSModule::registerSelf called" );
      registry.registerService( new  QgsWms::Service( "1.3.0", serverIface ) );
      registry.registerService( new  QgsWms::Service( "1.1.1", serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule* QGS_ServiceModule_Init()
{
  static QgsWmsModule module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule* )
{
  // Nothing to do
}





