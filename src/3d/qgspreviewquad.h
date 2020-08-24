#ifndef QGSPREVIEWQUAD_H
#define QGSPREVIEWQUAD_H

#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DExtras>

class QgsPreviewQuadMaterial : public Qt3DRender::QMaterial
{
  public:
    QgsPreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, const QMatrix4x4 &modelMatrix, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), QNode *parent = nullptr );
  private:
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mTextureParameter = nullptr;
    Qt3DRender::QParameter *mTextureTransformParameter = nullptr;
};

class QgsPreviewQuad : public Qt3DCore::QEntity
{
  public:
    QgsPreviewQuad( Qt3DRender::QAbstractTexture *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), Qt3DCore::QEntity *parent = nullptr );
  private:
    QgsPreviewQuadMaterial *mMaterial = nullptr;
};

#endif // QGSPREVIEWQUAD_H
