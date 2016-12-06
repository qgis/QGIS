/***************************************************************************
  qgsfieldformatterregistry.sip - QgsFieldFormatterRegistry

 ---------------------
 begin                : 2.12.2016
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
class QgsFieldFormatterRegistry : QObject
{
%TypeHeaderCode
#include "qgsfieldformatterregistry.h"
%End
  public:

    QgsFieldFormatterRegistry();
    ~QgsFieldFormatterRegistry();

    void addFieldFormatter( QgsFieldFormatter* kit /Transfer/ );

    void removeFieldFormatter( QgsFieldFormatter* kit );

    QgsFieldFormatter* fieldKit( const QString& id ) const;

  signals:
    void fieldKitAdded( QgsFieldFormatter* kit );

    void fieldKitRemoved( QgsFieldFormatter* kit );
};
