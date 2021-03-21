#ifndef QGSPOINTCLOUDBLOCKHANDLE_H
#define QGSPOINTCLOUDBLOCKHANDLE_H

#include <QObject>

#include "qgspointcloudattribute.h"

class QgsTileDownloadManagerReply;
class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

class QgsPointCloudBlockHandle : public QObject
{
    Q_OBJECT
  public:
    QgsPointCloudBlockHandle( const QString &dataType, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, QgsTileDownloadManagerReply *tileDownloadManagerReply );
  signals:
    void blockLoadingSucceeded( QgsPointCloudBlock *block );
    void blockLoadingFailed( const QString &errorStr );
  private:
    QString mDataType;
    QgsPointCloudAttributeCollection mAttributes;
    QgsPointCloudAttributeCollection mRequestedAttributes;
    QgsTileDownloadManagerReply *mTileDownloadManagetReply = nullptr;
  private slots:
    void blockFinishedLoading();
};

#endif // QGSPOINTCLOUDBLOCKHANDLE_H
