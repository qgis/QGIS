/***************************************************************************
  qgsconfigurationmap.sip - QgsConfigurationMap

 ---------------------
 begin                : 11.11.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
class QgsConfigurationMap
{
%TypeHeaderCode
#include "qgsconfigurationmap.h"
%End

  public:
    QgsConfigurationMap();

    QgsConfigurationMap( QVariantMap value );

    QVariantMap get() const;

    void set( const QVariantMap& value );

    void toXml( QDomElement& parentElement ) const;

    void fromXml( const QDomElement& element );
};
