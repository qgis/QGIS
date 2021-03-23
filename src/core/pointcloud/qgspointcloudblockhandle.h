#ifndef QGSPOINTCLOUDBLOCKHANDLE_H
#define QGSPOINTCLOUDBLOCKHANDLE_H

#include <QObject>

#include "qgspointcloudattribute.h"
#include "qgstiledownloadmanager.h"

#define SIP_NO_FILE

class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Base class for handling loading QgsPointCloudBlock asynchronously
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsPointCloudBlockHandle : public QObject
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
