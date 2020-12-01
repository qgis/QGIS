#ifndef QGSPOINTCLOUD3DSYMBOL_P_H
#define QGSPOINTCLOUD3DSYMBOL_P_H

///@cond PRIVATE

#include "qgspointcloud3dsymbol.h"

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"
#include "qgschunkedentity_p.h"

#include <QFutureWatcher>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QMaterial>
#include <QVector3D>

#define SIP_NO_FILE

class IndexedPointCloudNode;

class QgsPointCloud3DSymbolHandler // : public QgsFeature3DHandler
{
  public:
    QgsPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol );

    virtual bool prepare( const Qgs3DRenderContext &context ) = 0;// override;
    virtual void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context ) = 0; // override;
    virtual void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) = 0;// override;

    float zMinimum() const { return mZMin; }
    float zMaximum() const { return mZMax; }

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
      QVector<float> parameter;
      QVector<QVector3D> colors;
    };

  protected:
    float mZMin = std::numeric_limits<float>::max();
    float mZMax = std::numeric_limits<float>::lowest();

//  private:

    //static void addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent );
    //static void addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected );
    //static Qt3DCore::QTransform *transform( QVector3D position, const QgsPoint3DSymbol *symbol );

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );
    virtual Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data ) = 0;
    // input specific for this class
    //std::unique_ptr< QgsPoint3DSymbol > mSymbol;
    // inputs - generic
    //QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    // PointData outSelected;  //!< Features that are selected

    std::unique_ptr<QgsPointCloud3DSymbol> mSymbol;
};

class QgsSingleColorPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsSingleColorPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol );

    bool prepare( const Qgs3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;
  private:
//    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsColorRampPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsColorRampPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol );

    bool prepare( const Qgs3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;
  private:
//    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsRGBPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsRGBPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol );

    bool prepare( const Qgs3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;
  private:
//    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

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


/// @endcond

#endif // QGSPOINTCLOUD3DSYMBOL_P_H
