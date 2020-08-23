#ifndef QGSPREVIEWQUAD_H
#define QGSPREVIEWQUAD_H

#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DExtras>

class PreviewQuadMaterial : public Qt3DRender::QMaterial
{
  public:
    PreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), QNode *parent = nullptr );
  private:
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mTextureParameter = nullptr;
};

class PreviewQuad : public Qt3DCore::QEntity
{
  public:
    PreviewQuad( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>(), Qt3DCore::QEntity *parent = nullptr );
  private:
    PreviewQuadMaterial *mMaterial = nullptr;
};

#endif // QGSPREVIEWQUAD_H
