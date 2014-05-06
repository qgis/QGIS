/***************************************************************************
                              qgstransaction.h
                              ----------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRANSACTION_H
#define QGSTRANSACTION_H

#include <QUuid>
#include <QSet>
#include <QString>

class CORE_EXPORT QgsTransaction
{
    public:
        QgsTransaction( const QString& connString, const QString& providerKey );
        ~QgsTransaction();

        bool addLayer( const QString& layerId );

        bool begin( QString& errorMsg );
        bool commit( QString& errorMsg );
        bool rollback( QString& errorMsg );
        bool executeSql( QString& errorMsg ); //error message?

        //readXML
        //writeXML

    private:
        QUuid mId;
        QSet<QString> mLayers;
        QString mConnString;
        QString mProviderKey;

        bool disableLayerEditModes();
};

#endif // QGSTRANSACTION_H
