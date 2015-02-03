/***************************************************************************
                              qgsmapserviceexception.h
                              ------------------------
  begin                : June 13, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSERVICEEXCEPTION
#define QGSMAPSERVICEEXCEPTION

#include <QString>

/**
 * \class  QgsMapServiceException
 * \brief Exception class for WMS service exceptions.
 *
 * The most important codes are:
 *  * "InvalidFormat"
 *  * "Invalid CRS"
 *  * "LayerNotDefined" / "StyleNotDefined"
 *  * "OperationNotSupported"
 */

class SERVER_EXPORT QgsMapServiceException
{
  public:
    QgsMapServiceException( const QString& code, const QString& message );
    QString code() const {return mCode;}
    QString message() const {return mMessage;}
  private:
    QString mCode;
    QString mMessage;
};

#endif
