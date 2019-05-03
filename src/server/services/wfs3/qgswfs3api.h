/***************************************************************************
                              qgswfs3api.h
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

#ifndef QGSWFS3API_H
#define QGSWFS3API_H

#include "qgsmodule.h"
#include "qgsproject.h"
#include "qgsserverexception.h"

#include "nlohmann/json.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;


namespace QgsWfs3
{

  class Api;

  Q_NAMESPACE

  //! Rel link types
  enum rel
  {
    alternate,
    conformance,
    data,
    describedBy,
    item,
    self,
    service
  };
  Q_ENUM_NS( rel )

  //! For content negotiation based on requested file extention
  enum contentType
  {
    JSON,
    HTML
  };
  Q_ENUM_NS( contentType )

  //! Generic URL handler
  struct Handler
  {

    //Q_GADGET

    virtual ~Handler() = default;
    virtual void handleRequest( const Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const;
    std::string href( const Api *api, const QgsServerRequest &request ) const;

    QString path;
    std::string operationId;
    std::string summary;
    std::string description;
    std::string linkTitle;
    rel linkType;
    std::string mimeType;
  };

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
      Api( QgsServerInterface *serverIface );

      QString name()    const override { return QStringLiteral( "WFS3" ); }
      QString version() const override { return QStringLiteral( "1.0.0" ); }
      QString rootPath() const override { return mRootPath; }

      // Utilities

      template<class T>
      void registerHandler();

      /**
       * Returns a copy of the \a url with the path without an ending slash
       */
      QUrl normalizedUrl( QUrl url ) const;

      /**
       * Returns the \a url path extension to upper case (if any)
       */
      static std::string extension( QUrl url );

      const std::vector<std::unique_ptr<Handler>> &handlers() const;

      /**
       * Utility that writes JSON output to the response stream (indented if debug is active)
       * \param data json data
       * \param response
       */
      static void jsonDump( const json &data, QgsServerResponse &response, const QString &contentType = QStringLiteral( "application/json" ) );

      /**
       * Utility that writes JSON output to the response stream (indented if debug is active)
       * \param data json data
       * \param response
       */
      static void htmlDump( const json &data, QgsServerResponse &response, const std::string &templatePath );

      static std::string relToString( const QgsWfs3::rel &rel )
      {
        static QMetaEnum metaEnum = QMetaEnum::fromType<QgsWfs3::rel>();
        return metaEnum.valueToKey( rel );
      }

      static std::string contentTypeToString( const contentType &ct )
      {
        static QMetaEnum metaEnum = QMetaEnum::fromType<contentType>();
        return metaEnum.valueToKey( ct );
      }

    private:

      QgsServerInterface *mServerIface = nullptr;
      //! Catch all, must be the last!
      QString mRootPath { "" };

      std::vector<std::unique_ptr<Handler>> mHandlers;


      // QgsServerApi interface
    public:

      void executeRequest( const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const override;
  };


} // namespace QgsWfs3

#endif // QGSWFS3API_H
