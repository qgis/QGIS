#include "qgspreviewquad.h"


PreviewQuad::PreviewQuad( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters, Qt3DCore::QEntity *parent )
  : Qt3DCore::QEntity( parent )
{
  mMaterial = new PreviewQuadMaterial( texture, additionalShaderParameters );

  setObjectName( "Preview Quad" );
  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry;
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute;
  float d = 1.0f;
  QVector<float> vert =
  {
    -d, -d, 0.0f,
      d, -d, 0.0f,
      -d,  d, 0.0f,
      -d,  d, 0.0f,
      d, -d, 0.0f,
      d,  d, 0.0f
    };

  QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer;
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setCount( 6 );

  geom->addAttribute( positionAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  addComponent( renderer );
  addComponent( mMaterial );
}

PreviewQuadMaterial::PreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters, QNode *parent )
  : Qt3DRender::QMaterial( parent )
{
  mTextureParameter = new Qt3DRender::QParameter( "previewTexture", texture );
  addParameter( mTextureParameter );
  for ( Qt3DRender::QParameter *parameter : additionalShaderParameters ) addParameter( parameter );

  mEffect = new Qt3DRender::QEffect;

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;

  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram;
  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.vert" ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.frag" ) ) );
  renderPass->setShaderProgram( shader );

  technique->addRenderPass( renderPass );

  mEffect->addTechnique( technique );
  setEffect( mEffect );
}

