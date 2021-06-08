/***************************************************************************
                         qgspointcloudrgbrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudblock.h"
#include "qgscontrastenhancement.h"

QgsPointCloudRgbRenderer::QgsPointCloudRgbRenderer()
{

}

QString QgsPointCloudRgbRenderer::type() const
{
  return QStringLiteral( "rgb" );
}

QgsPointCloudRenderer *QgsPointCloudRgbRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudRgbRenderer > res = qgis::make_unique< QgsPointCloudRgbRenderer >();
  res->mRedAttribute = mRedAttribute;
  res->mGreenAttribute = mGreenAttribute;
  res->mBlueAttribute = mBlueAttribute;

  if ( mRedContrastEnhancement )
  {
    res->setRedContrastEnhancement( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  }
  if ( mGreenContrastEnhancement )
  {
    res->setGreenContrastEnhancement( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  }
  if ( mBlueContrastEnhancement )
  {
    res->setBlueContrastEnhancement( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );
  }

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudRgbRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsRectangle visibleExtent = context.renderContext().extent();

  const char *ptr = block->data();
  int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int redOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mRedAttribute, redOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType redType = attribute->type();

  int greenOffset = 0;
  attribute = request.find( mGreenAttribute, greenOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType greenType = attribute->type();

  int blueOffset = 0;
  attribute = request.find( mBlueAttribute, blueOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType blueType = attribute->type();

  const bool useRedContrastEnhancement = mRedContrastEnhancement && mRedContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useBlueContrastEnhancement = mBlueContrastEnhancement && mBlueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useGreenContrastEnhancement = mGreenContrastEnhancement && mGreenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;

  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite();

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();
  for ( int i = 0; i < count; ++i )
  {
    if ( considerZ )
    {
      // z value filtering is cheapest, if we're doing it...
      z = pointZ( context, ptr, i );
      if ( !zRange.contains( z ) )
        continue;
    }

    pointXY( context, ptr, i, x, y );
    if ( visibleExtent.contains( QgsPointXY( x, y ) ) )
    {
      if ( reproject )
      {
        try
        {
          ct.transformInPlace( x, y, z );
        }
        catch ( QgsCsException & )
        {
          continue;
        }
      }

      int red = 0;
      context.getAttribute( ptr, i * recordSize + redOffset, redType, red );
      int green = 0;
      context.getAttribute( ptr, i * recordSize + greenOffset, greenType, green );
      int blue = 0;
      context.getAttribute( ptr, i * recordSize + blueOffset, blueType, blue );

      //skip if red, green or blue not in displayable range
      if ( ( useRedContrastEnhancement && !mRedContrastEnhancement->isValueInDisplayableRange( red ) )
           || ( useGreenContrastEnhancement && !mGreenContrastEnhancement->isValueInDisplayableRange( green ) )
           || ( useBlueContrastEnhancement && !mBlueContrastEnhancement->isValueInDisplayableRange( blue ) ) )
      {
        continue;
      }

      //stretch color values
      if ( useRedContrastEnhancement )
      {
        red = mRedContrastEnhancement->enhanceContrast( red );
      }
      if ( useGreenContrastEnhancement )
      {
        green = mGreenContrastEnhancement->enhanceContrast( green );
      }
      if ( useBlueContrastEnhancement )
      {
        blue = mBlueContrastEnhancement->enhanceContrast( blue );
      }

      drawPoint( x, y, QColor( red, green, blue ), context );
      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}

void QgsPointCloudRgbRenderer::renderDisplaz(DrawCount mdrawlist, std::shared_ptr<Geometry> m_geom, QgsPointCloudRenderContext &context)
{
	const QgsRectangle visibleExtent = context.renderContext().extent();
	//attributes(), request.attributes()
	int decimal_step = 1;
	if (mdrawlist.numVertices>1000000)
	{
		decimal_step = std::floorl(mdrawlist.numVertices / 1000000);
	}
	else
	{
		decimal_step = 1;
	}
   int count = mdrawlist.numVertices / decimal_step;
	
	const bool useRedContrastEnhancement = mRedContrastEnhancement && mRedContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
	const bool useBlueContrastEnhancement = mBlueContrastEnhancement && mBlueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
	const bool useGreenContrastEnhancement = mGreenContrastEnhancement && mGreenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;

	const QgsDoubleRange zRange = context.renderContext().zRange();
	const bool considerZ = !zRange.isInfinite();

	int rendered = 0;
	double x = 0;
	double y = 0;
	double z = 0;
	const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
	const bool reproject = ct.isValid();
	V3f* m_P;
	const uint16_t * color;
	const std::vector<PointCloudGeomField>* m_pointarrayfields = m_geom->GetFiled();
	for (size_t i = 0; i < m_pointarrayfields->size(); ++i)
	{
		const PointCloudGeomField& field = (*m_pointarrayfields)[i];
		if (field.name == "position")
		{
			m_P = (V3f*)field.as<float>();
			//offset = m_geom->offset();
		}
		if (field.name == "color")
		{
			//uint16_t* bufferData = field.data.get();
			color = field.as<uint16_t>();
		}
	}
	V3d Vertex;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	while (mdrawlist.index.size() > decimal_step && mdrawlist.numVertices>1)
	{
		std::list<size_t>::iterator it = mdrawlist.index.begin();
		try
		{
      if (!m_geom->getPointByIndex(*it, Vertex))
      {
        return;
      } 
			//V3f Vertex = V3f(m_P[*it]) + m_geom->offset();+
			 red = color[(*it) * 3] / 100;
			 green = color[(*it) * 3 + 1] / 100;
			blue = color[(*it) * 3 + 2] / 100;
			drawPoint(Vertex.x, Vertex.y, QColor(red, green, blue), context);
		}
		catch (const std::exception& e)
		{
			e.what();
			return;
		}
		rendered++;
		for (size_t i = 0; i < decimal_step; i++)
		{
			mdrawlist.index.erase(it);
			it = mdrawlist.index.begin();
			mdrawlist.numVertices--;
		}
	} 
	m_P = nullptr;
	color = nullptr;
	context.incrementPointsRendered(rendered);
}
QgsPointCloudRenderer *QgsPointCloudRgbRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudRgbRenderer > r = qgis::make_unique< QgsPointCloudRgbRenderer >();

  r->setRedAttribute( element.attribute( QStringLiteral( "red" ), QStringLiteral( "Red" ) ) );
  r->setGreenAttribute( element.attribute( QStringLiteral( "green" ), QStringLiteral( "Green" ) ) );
  r->setBlueAttribute( element.attribute( QStringLiteral( "blue" ), QStringLiteral( "Blue" ) ) );

  r->restoreCommonProperties( element, context );

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  QDomElement redContrastElem = element.firstChildElement( QStringLiteral( "redContrastEnhancement" ) );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
    redContrastEnhancement->readXml( redContrastElem );
    r->setRedContrastEnhancement( redContrastEnhancement );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  QDomElement greenContrastElem = element.firstChildElement( QStringLiteral( "greenContrastEnhancement" ) );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
    greenContrastEnhancement->readXml( greenContrastElem );
    r->setGreenContrastEnhancement( greenContrastEnhancement );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  QDomElement blueContrastElem = element.firstChildElement( QStringLiteral( "blueContrastEnhancement" ) );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( Qgis::UnknownDataType );
    blueContrastEnhancement->readXml( blueContrastElem );
    r->setBlueContrastEnhancement( blueContrastEnhancement );
  }

  return r.release();
}

QDomElement QgsPointCloudRgbRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "rgb" ) );

  rendererElem.setAttribute( QStringLiteral( "red" ), mRedAttribute );
  rendererElem.setAttribute( QStringLiteral( "green" ), mGreenAttribute );
  rendererElem.setAttribute( QStringLiteral( "blue" ), mBlueAttribute );

  saveCommonProperties( rendererElem, context );

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( QStringLiteral( "redContrastEnhancement" ) );
    mRedContrastEnhancement->writeXml( doc, redContrastElem );
    rendererElem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( QStringLiteral( "greenContrastEnhancement" ) );
    mGreenContrastEnhancement->writeXml( doc, greenContrastElem );
    rendererElem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( QStringLiteral( "blueContrastEnhancement" ) );
    mBlueContrastEnhancement->writeXml( doc, blueContrastElem );
    rendererElem.appendChild( blueContrastElem );
  }

  return rendererElem;
}

QSet<QString> QgsPointCloudRgbRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mRedAttribute << mGreenAttribute << mBlueAttribute;
  return res;
}

QString QgsPointCloudRgbRenderer::redAttribute() const
{
  return mRedAttribute;
}

void QgsPointCloudRgbRenderer::setRedAttribute( const QString &redAttribute )
{
  mRedAttribute = redAttribute;
}

QString QgsPointCloudRgbRenderer::greenAttribute() const
{
  return mGreenAttribute;
}

void QgsPointCloudRgbRenderer::setGreenAttribute( const QString &greenAttribute )
{
  mGreenAttribute = greenAttribute;
}

QString QgsPointCloudRgbRenderer::blueAttribute() const
{
  return mBlueAttribute;
}

void QgsPointCloudRgbRenderer::setBlueAttribute( const QString &blueAttribute )
{
  mBlueAttribute = blueAttribute;
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::redContrastEnhancement() const
{
  return mRedContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setRedContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mRedContrastEnhancement.reset( enhancement );
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::greenContrastEnhancement() const
{
  return mGreenContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setGreenContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mGreenContrastEnhancement.reset( enhancement );
}

const QgsContrastEnhancement *QgsPointCloudRgbRenderer::blueContrastEnhancement() const
{
  return mBlueContrastEnhancement.get();
}

void QgsPointCloudRgbRenderer::setBlueContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mBlueContrastEnhancement.reset( enhancement );
}
