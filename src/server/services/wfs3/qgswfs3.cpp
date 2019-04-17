/***************************************************************************
                              qgswfs3.cpp
                              -------------------------
  begin                : April 15, 2019
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
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
#include "qgsproject.h"
#include "qgsserverexception.h"

#include "nlohmann/json.hpp"
#include "inja/inja.hpp"
using json = nlohmann::json;

namespace QgsWfs3
{

  /**
   * \ingroup server
   * \class QgsWfs3::Api
   * \brief OGC web service specialized for WFS3
   * \since QGIS 3.10
   */
  class Api: public QgsServerApi
  {
    public:

      /**
       * Constructor for WFS service.
       * \param serverIface Interface for plugins.
       */
      Api( QgsServerInterface *serverIface )
        : mServerIface( serverIface )
      {}

      QString name()    const override { return QStringLiteral( "WFS3" ); }
      QString version() const override { return QStringLiteral( "1.0.0" ); }
      QRegularExpression rootPath() const override { return mRootPath; }

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) override
      {
        QJsonObject data;
        if ( QRegularExpression( "raw(^/?collections$)raw" ).match( request.url().path() ).hasMatch() )
        {
          if ( ! project )
          {
            throw QgsServerApiException( QStringLiteral( "project_error" ), QStringLiteral( "Project not found" ) );
          }
          if ( request.method() != QgsServerRequest::Method::GetMethod )
          {
            throw QgsServerApiException( QStringLiteral( "method_error" ), QStringLiteral( "Unsuppored method" ) );
          }
          QJsonArray collections;
          QJsonArray crs {{
              QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" )
            }};
          // TODO: exposed CRSs
          for ( const auto &l : project->mapLayers( ) )
          {
            // TODO: use layer id?
            const auto extent { l->extent().toString( ) };
            collections.append( QJsonObject { {
                { QStringLiteral( "name" ), l->shortName().isEmpty() ? l->name() : l->shortName() },
                { QStringLiteral( "title" ), l->name() },
                { QStringLiteral( "description" ), l->abstract() },
                { QStringLiteral( "extent" ), extent },
                { QStringLiteral( "crs" ), crs },
              }
            } );
          }
          data =
          {
            { QStringLiteral( "links" ), QStringLiteral( "links" )},
            { QStringLiteral( "collections" ), collections },
          };
        }
        else if ( QRegularExpression( "raw(^/?jsontest)raw" ).match( request.url().path() ).hasMatch() )
        {
          json j;
          j[ "a" ] = "b";
          j[ "b" ] = { "b", 1.2, 3 };
          data = QJsonDocument::fromJson( j.dump( ).data() ).object();
        }
        else  // Default root
        {
          if ( request.method() != QgsServerRequest::Method::GetMethod )
          {
            throw QgsServerApiException( QStringLiteral( "method_error" ), QStringLiteral( "Unsuppored method" ) );
          }
          json j;
          j["conformance"] = QStringLiteral( "%1conformance" ).arg( request.url().toString( ) ).toStdString();
          j["path"] = request.url().path().toStdString();
          data = QJsonDocument::fromJson( j.dump( ).data() ).object();
        }
#ifdef QGISDEBUG
        response.write( QJsonDocument( data ).toJson( QJsonDocument::JsonFormat::Indented ) );
#else
        response.write( QJsonDocument( data ).toJson( QJsonDocument::JsonFormat::Compact ) );
#endif
      }

    private:

      QgsServerInterface *mServerIface = nullptr;
      //! Catch all, must be the last!
      QRegularExpression mRootPath { QStringLiteral( R"raw(.*)raw" ) };

  };


} // namespace QgsWfs3

/**
 * \ingroup server
 * \class QgsWfsModule
 * \brief Module specialized for WFS3 service
 * \since QGIS 3.10
 */
class QgsWfs3Module: public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsDebugMsg( QStringLiteral( "QgsWfs3Module::registerSelf called" ) );
      registry.registerApi( new  QgsWfs3::Api( serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWfs3Module module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
