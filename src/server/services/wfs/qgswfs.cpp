/***************************************************************************
                              qgswfs.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts from qgswmshandler)
                         (C) 2012 by René-Luc D'Hont    ( parts from qgswmshandler)
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
#include "qgswfsutils.h"
#include "qgswfsgetcapabilities.h"
#include "qgswfsgetcapabilities_1_0_0.h"
#include "qgswfsgetfeature.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfstransaction.h"
#include "qgswfstransaction_1_0_0.h"

#define QSTR_COMPARE( str, lit )\
  (str.compare( QLatin1String( lit ), Qt::CaseInsensitive ) == 0)

namespace QgsWfs
{

  /**
   * \ingroup server
   * \class QgsWfs::Service
   * \brief OGC web service specialized for WFS
   * \since QGIS 3.0
   */
  class Service: public QgsService
  {
    public:

      /**
       * Constructor for WFS service.
       * \param serverIface Interface for plugins.
       */
      Service( QgsServerInterface *serverIface )
        : mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WFS" ); }
      QString version() const override { return implementationVersion(); }

      bool allowMethod( QgsServerRequest::Method method ) const override
      {
        return method == QgsServerRequest::GetMethod || method == QgsServerRequest::PostMethod;
      }

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response,
                           const QgsProject *project ) override
      {
        const QgsWfsParameters params( QUrlQuery( request.url() ) );

        // Set the default version
        QString versionString = params.version();
        if ( versionString.isEmpty() )
        {
          versionString = version(); // defined in qgswfsutils.h
        }

        // Get the request
        const QString req = params.request();
        if ( req.isEmpty() )
        {
          throw QgsServiceException( QStringLiteral( "OperationNotSupported" ),
                                     QStringLiteral( "Please check the value of the REQUEST parameter" ), 501 );
        }

        if ( QSTR_COMPARE( req, "GetCapabilities" ) )
        {
          // Supports WFS 1.0.0
          if ( QSTR_COMPARE( versionString, "1.0.0" ) )
          {
            v1_0_0::writeGetCapabilities( mServerIface, project, versionString, request, response );
          }
          else
          {
            writeGetCapabilities( mServerIface, project, versionString, request, response );
          }
        }
        else if ( QSTR_COMPARE( req, "GetFeature" ) )
        {
          writeGetFeature( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "DescribeFeatureType" ) )
        {
          writeDescribeFeatureType( mServerIface, project, versionString, request, response );
        }
        else if ( QSTR_COMPARE( req, "Transaction" ) )
        {
          // Supports WFS 1.0.0
          if ( QSTR_COMPARE( versionString, "1.0.0" ) )
          {
            v1_0_0::writeTransaction( mServerIface, project, versionString, request, response );
          }
          else
          {
            writeTransaction( mServerIface, project, versionString, request, response );
          }
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


} // namespace QgsWfs

/**
 * \ingroup server
 * \class QgsWfsModule
 * \brief Module specialized for WFS service
 * \since QGIS 3.0
 */
class QgsWfsModule: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsDebugMsg( QStringLiteral( "WFSModule::registerSelf called" ) );
      registry.registerService( new  QgsWfs::Service( serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWfsModule module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
