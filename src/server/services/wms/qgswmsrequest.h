/***************************************************************************
                          qgswmsrequest.h

  Define request class for getting request contents for WMS service
  -------------------
  begin                : 2021-02-10
  copyright            : (C) 2021 by Paul Blottiere
  email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWMSREQUEST_H
#define QGSWMSREQUEST_H

#include "qgsserverrequest.h"
#include "qgswmsparameters.h"

namespace QgsWms
{

  /**
   * \ingroup server
   * \class QgsWmsRequest
   * \brief Class defining request interface passed to WMS service
   * \since QGIS 3.20
   */
  class QgsWmsRequest : public QgsServerRequest
  {
      Q_GADGET

    public:
      /**
       * Copy constructor
       */
      QgsWmsRequest( const QgsServerRequest &other );

      /**
       * Destructor.
       */
      ~QgsWmsRequest() override = default;

      /**
       * Returns the parameters interpreted for the WMS service.
       */
      const QgsWmsParameters &wmsParameters() const;

      void setParameter( const QString &key, const QString &value ) override;

      void removeParameter( const QString &key ) override;

      void setUrl( const QUrl &url ) override;

    private:
      void init();

      QgsWmsParameters mWmsParams;
  };
} // namespace QgsWms

#endif
