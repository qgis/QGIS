#ifndef QGSPOINTCLOUD3DSYMBOL_P_H
#define QGSPOINTCLOUD3DSYMBOL_P_H

#define NO_SIP

///@cond PRIVATE

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

#include "qgspointcloudlayerchunkloader_p.h"

class QgsPointCloud3DSymbol;

#ifndef SIP_RUN

class QgsPointCloud3DGeometry: public Qt3DRender::QGeometry
{
  public:
//    QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol );
    QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, QgsPointCloud3DSymbol *symbol );

  protected:
    virtual void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) = 0;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mParameterAttribute = nullptr;
    Qt3DRender::QAttribute *mColorAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    int mVertexCount = 0;

    unsigned int mByteStride = 16;
};

class QgsSingleColorPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsColorRampPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsColorRampPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsRGBPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsRGBPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol );
  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

#endif

/// @endcond

#endif // QGSPOINTCLOUD3DSYMBOL_P_H
