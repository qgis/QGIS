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
       * \param version Version of the WFS service. (since QGIS 3.22.12)
       * \param serverIface Interface for plugins.
       */
      Service( const QString &version, QgsServerInterface *serverIface )
        : mVersion( version )
        , mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WFS" ); }
      QString version() const override { return mVersion; }

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
                                     QStringLiteral( "Please add or check the value of the REQUEST parameter" ), 501 );
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
      QString mVersion;
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
      registry.registerService( new  QgsWfs::Service( QgsWfs::implementationVersion(), serverIface ) ); // 1.1.0 default version
      registry.registerService( new  QgsWfs::Service( QStringLiteral( "1.0.0" ), serverIface ) ); // second version
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
