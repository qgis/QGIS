#ifndef QGSSTOREBADLAYERINFO_H
#define QGSSTOREBADLAYERINFO_H

#include "qgsprojectbadlayerhandler.h"
#include <QStringList>

/**
 * \ingroup server
 * Stores layer ids of bad layers
 */
class QgsStoreBadLayerInfo: public QgsProjectBadLayerHandler
{
  public:
    /*Default constructor
     */
    QgsStoreBadLayerInfo();

    /*Destructor
     */
    ~QgsStoreBadLayerInfo();

    /**
     * @brief handleBadLayers
     * @param layers layer nodes
     */
    void handleBadLayers( const QList<QDomNode> &layers );

    /**
     * @brief badLayers
     * @return ids of bad layers
     */
    QStringList badLayers() const { return mBadLayerIds; }

  private:
    QStringList mBadLayerIds;
};

#endif // QGSSTOREBADLAYERINFO_H
