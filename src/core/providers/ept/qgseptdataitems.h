/***************************************************************************
                         qgseptdataitems.h
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

#ifndef QGSEPTDATAITEMS_H
#define QGSEPTDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

class CORE_EXPORT QgsEptLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsEptLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
    QString layerName() const override;

};

//! Provider for EPT data items
class QgsEptDataItemProvider : public QgsDataItemProvider
{
  public:

    QgsEptDataItemProvider();

    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;

  private:

    QString mFileFilter;
};

///@endcond
#endif // QGSEPTDATAITEMS_H



