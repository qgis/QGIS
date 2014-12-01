/***************************************************************************
                       qgsmapserviceexception.sip

  QGIS Server exception
  -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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

class QgsMapServiceException
{
%TypeHeaderCode
#include <qgsmapserviceexception.h>
%End
  public:
    QgsMapServiceException( const QString& code, const QString& message );
    QString code() const;
    QString message() const;

};
