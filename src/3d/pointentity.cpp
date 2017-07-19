#include "pointentity.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include <Qt3DExtras/QCylinderGeometry>
#include <Qt3DExtras/QConeGeometry>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DExtras/QTorusGeometry>
#if QT_VERSION >= 0x050900
#include <Qt3DExtras/QExtrudedTextGeometry>
#endif

#include <QUrl>
#include <QVector3D>

#include "map3d.h"
#include "terraingenerator.h"

#include "qgsvectorlayer.h"
#include "qgspoint.h"



PointEntity::PointEntity( const Map3D &map, const PointRenderer &settings, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  //
  // load features
  //

  QList<QVector3D> positions;
  QgsFeature f;
  QgsFeatureRequest request;
  request.setDestinationCrs( map.crs );
  QgsFeatureIterator fi = settings.layer()->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    QgsAbstractGeometry *g = f.geometry().geometry();
    if ( QgsWkbTypes::flatType( g->wkbType() ) == QgsWkbTypes::Point )
    {
      QgsPoint *pt = static_cast<QgsPoint *>( g );
      // TODO: use Z coordinates if the point is 3D
      float h = map.terrainGenerator->heightAt( pt->x(), pt->y(), map ) * map.zExaggeration;
      positions.append( QVector3D( pt->x() - map.originX, h + settings.height, -( pt->y() - map.originY ) ) );
      //qDebug() << positions.last();
    }
    else
      qDebug() << "not a point";
  }

  int count = positions.count();

  QByteArray ba;
  ba.resize( count * sizeof( QVector3D ) );
  QVector3D *posData = reinterpret_cast<QVector3D *>( ba.data() );
  for ( int j = 0; j < count; ++j )
  {
    *posData = positions[j];
    ++posData;
  }

  //
  // geometry renderer
  //

  Qt3DRender::QBuffer *instanceBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer );
  instanceBuffer->setData( ba );

  Qt3DRender::QAttribute *instanceDataAttribute = new Qt3DRender::QAttribute;
  instanceDataAttribute->setName( "pos" );
  instanceDataAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  instanceDataAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  instanceDataAttribute->setVertexSize( 3 );
  instanceDataAttribute->setDivisor( 1 );
  instanceDataAttribute->setBuffer( instanceBuffer );

  Qt3DRender::QGeometry *geometry = nullptr;
  QString shape = settings.shapeProperties["shape"].toString();
  if ( shape == "sphere" )
  {
    float radius = settings.shapeProperties["radius"].toFloat();
    Qt3DExtras::QSphereGeometry *g = new Qt3DExtras::QSphereGeometry;
    g->setRadius( radius ? radius : 10 );
    geometry = g;
  }
  else if ( shape == "cone" )
  {
    float length = settings.shapeProperties["length"].toFloat();
    float bottomRadius = settings.shapeProperties["bottomRadius"].toFloat();
    float topRadius = settings.shapeProperties["topRadius"].toFloat();
    Qt3DExtras::QConeGeometry *g = new Qt3DExtras::QConeGeometry;
    g->setLength( length ? length : 10 );
    g->setBottomRadius( bottomRadius );
    g->setTopRadius( topRadius );
    //g->setHasBottomEndcap(hasBottomEndcap);
    //g->setHasTopEndcap(hasTopEndcap);
    geometry = g;
  }
  else if ( shape == "cube" )
  {
    float size = settings.shapeProperties["size"].toFloat();
    Qt3DExtras::QCuboidGeometry *g = new Qt3DExtras::QCuboidGeometry;
    g->setXExtent( size ? size : 10 );
    g->setYExtent( size ? size : 10 );
    g->setZExtent( size ? size : 10 );
    geometry = g;
  }
  else if ( shape == "torus" )
  {
    float radius = settings.shapeProperties["radius"].toFloat();
    float minorRadius = settings.shapeProperties["minorRadius"].toFloat();
    Qt3DExtras::QTorusGeometry *g = new Qt3DExtras::QTorusGeometry;
    g->setRadius( radius ? radius : 10 );
    g->setMinorRadius( minorRadius ? minorRadius : 5 );
    geometry = g;
  }
  else if ( shape == "plane" )
  {
    float size = settings.shapeProperties["size"].toFloat();
    Qt3DExtras::QPlaneGeometry *g = new Qt3DExtras::QPlaneGeometry;
    g->setWidth( size ? size : 10 );
    g->setHeight( size ? size : 10 );
    geometry = g;
  }
#if QT_VERSION >= 0x050900
  else if ( shape == "extrudedText" )
  {
    float depth = settings.shapeProperties["depth"].toFloat();
    QString text = settings.shapeProperties["text"].toString();
    Qt3DExtras::QExtrudedTextGeometry *g = new Qt3DExtras::QExtrudedTextGeometry;
    g->setDepth( depth ? depth : 1 );
    g->setText( text );
    geometry = g;
  }
#endif
  else  // shape == "cylinder" or anything else
  {
    float radius = settings.shapeProperties["radius"].toFloat();
    float length = settings.shapeProperties["length"].toFloat();
    Qt3DExtras::QCylinderGeometry *g = new Qt3DExtras::QCylinderGeometry;
    //g->setRings(2);  // how many vertices vertically
    //g->setSlices(8); // how many vertices on circumference
    g->setRadius( radius ? radius : 10 );
    g->setLength( length ? length : 10 );
    geometry = g;
  }

  geometry->addAttribute( instanceDataAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  renderer->setInstanceCount( count );
  addComponent( renderer );

  //
  // material
  //

  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( "renderingStyle" );
  filterKey->setValue( "forward" );

  // the fragment shader implements a simplified version of phong shading that uses hardcoded light
  // (instead of whatever light we have defined in the scene)
  // TODO: use phong shading that respects lights from the scene
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/instanced.vert" ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/instanced.frag" ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 2 );

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( QStringLiteral( "kd" ), QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f );

  diffuseParameter->setValue( settings.material.diffuse() );
  ambientParameter->setValue( settings.material.ambient() );
  specularParameter->setValue( settings.material.specular() );
  shininessParameter->setValue( settings.material.shininess() );

  QMatrix4x4 transformMatrix = settings.transform;
  QMatrix3x3 normalMatrix = transformMatrix.normalMatrix();  // transponed inverse of 3x3 sub-matrix

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  QMatrix4x4 normalMatrix4(
    n[0], n[3], n[6], 0,
    n[1], n[4], n[7], 0,
    n[2], n[5], n[8], 0,
    0, 0, 0, 0 );

  Qt3DRender::QParameter *paramInst = new Qt3DRender::QParameter;
  paramInst->setName( "inst" );
  paramInst->setValue( transformMatrix );

  Qt3DRender::QParameter *paramInstNormal = new Qt3DRender::QParameter;
  paramInstNormal->setName( "instNormal" );
  paramInstNormal->setValue( normalMatrix4 );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  effect->addTechnique( technique );
  effect->addParameter( paramInst );
  effect->addParameter( paramInstNormal );

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;
  material->setEffect( effect );
  addComponent( material );
}
