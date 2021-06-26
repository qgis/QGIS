/***************************************************************************
                         qgseptproviderguimetadata.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTPROVIDERGUIMETADATA_H
#define QGSEPTPROVIDERGUIMETADATA_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsEptProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsEptProviderGuiMetadata();

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
};

///@endcond

#endif // QGSEPTPROVIDERGUIMETADATA_H
