/***************************************************************************
                         qgspdaldataitems.h
                         --------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALDATAITEMS_H
#define QGSPDALDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsprovidermetadata.h"

class QgsPdalLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsPdalLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
    QString layerName() const override;
};

//! Provider for PDAL data items
class QgsPdalDataItemProvider : public QgsDataItemProvider
{
  public:
    QgsPdalDataItemProvider();

    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
  private:
    QString mFileFilter;
};

#endif // QGSPDALDATAITEMS_H



