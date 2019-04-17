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
#include "qgsproject.h"

namespace QgsWfs3
{

  /**
   * \ingroup server
   * \class QgsWfs::Service
   * \brief OGC web service specialized for WFS
   * \since QGIS 3.0
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
        Q_UNUSED( request );
        QgsMessageLog::logMessage( QStringLiteral( "Running WFS3" ), QStringLiteral( "Server" ), Qgis::Info );
        response.write( QStringLiteral( "Hello! from project %1" ).arg( project ? project->fileName() : QStringLiteral( "unknown" ) ) );
        response.finish();
      }

    private:
      QgsServerInterface *mServerIface = nullptr;
      QRegularExpression mRootPath { QStringLiteral( R"raw(^$)raw" ) };
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
