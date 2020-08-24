#include "qgspreviewquad.h"


QgsPreviewQuad::QgsPreviewQuad( Qt3DRender::QAbstractTexture *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters, Qt3DCore::QEntity *parent )
  : Qt3DCore::QEntity( parent )
{
  setObjectName( "Preview Quad" );
  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry;
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute;
  QVector<float> vert =
  {
    -1.0f, -1.0f, 1.0f, // bottom left
      1.0f, -1.0f, 1.0f, // top left
      -1.0f,  1.0f, 1.0f, // bottom right
      -1.0f,  1.0f, 1.0f, // bottom right
      1.0f, -1.0f, 1.0f, // top right
      1.0f,  1.0f, 1.0f  // bottom right
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

  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();
  modelMatrix.translate( centerNDC.x(), centerNDC.y() );
  modelMatrix.scale( size.width(), size.height() );
  mMaterial = new QgsPreviewQuadMaterial( texture, modelMatrix, additionalShaderParameters );

  addComponent( mMaterial );
}

QgsPreviewQuadMaterial::QgsPreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, const QMatrix4x4 &modelMatrix, QVector<Qt3DRender::QParameter *> additionalShaderParameters, QNode *parent )
  : Qt3DRender::QMaterial( parent )
{
  mTextureParameter = new Qt3DRender::QParameter( "previewTexture", texture );
  mTextureTransformParameter = new Qt3DRender::QParameter( "modelMatrix", QVariant::fromValue( modelMatrix ) );
  addParameter( mTextureParameter );
  addParameter( mTextureTransformParameter );
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

