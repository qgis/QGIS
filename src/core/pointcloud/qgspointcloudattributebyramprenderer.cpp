/***************************************************************************
                         qgspointcloudattributebyramprenderer.h
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

#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudblock.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgssymbollayerutils.h"
#include "qgslayertreemodellegendnode.h"

QgsPointCloudAttributeByRampRenderer::QgsPointCloudAttributeByRampRenderer()
{
}

QString QgsPointCloudAttributeByRampRenderer::type() const
{
  return QStringLiteral( "ramp" );
}

QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > res = qgis::make_unique< QgsPointCloudAttributeByRampRenderer >();
  res->mAttribute = mAttribute;
  res->mColorRampShader = mColorRampShader;
  res->mMin = mMin;
  res->mMax = mMax;

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudAttributeByRampRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsRectangle visibleExtent = context.renderContext().extent();

  const char *ptr = block->data();
  int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int attributeOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mAttribute, attributeOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType attributeType = attribute->type();

  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite();

  const bool applyZOffset = attribute->name() == QLatin1String( "Z" );
  const bool applyXOffset = attribute->name() == QLatin1String( "X" );
  const bool applyYOffset = attribute->name() == QLatin1String( "Y" );

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();

  int red = 0;
  int green = 0;
  int blue = 0;
  int alpha = 0;
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

      double attributeValue = 0;
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, attributeValue );

      if ( applyXOffset )
        attributeValue = context.offset().x() + context.scale().x() * attributeValue;
      if ( applyYOffset )
        attributeValue = context.offset().y() + context.scale().y() * attributeValue;
      if ( applyZOffset )
        attributeValue = context.offset().z() + context.scale().z() * attributeValue;

      mColorRampShader.shade( attributeValue, &red, &green, &blue, &alpha );
      drawPoint( x, y, QColor( red, green, blue, alpha ), context );

      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}

void QgsPointCloudAttributeByRampRenderer::renderDisplaz(DrawCount mdrawlist, std::shared_ptr<Geometry> m_geom, QgsPointCloudRenderContext &context)
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

	const QgsDoubleRange zRange = context.renderContext().zRange();
	const bool considerZ = !zRange.isInfinite();

	int rendered = 0;
	double x = 0;
	double y = 0;
	double z = 0;
	const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
	const bool reproject = ct.isValid();
	V3f* m_P;
	const uint16_t * intensity = nullptr;
	bool is_Z = false;
	const std::vector<PointCloudGeomField>* m_pointarrayfields = m_geom->GetFiled();
	

	for (size_t i = 0; i < m_pointarrayfields->size(); ++i)
	{
		const PointCloudGeomField& field = (*m_pointarrayfields)[i];
		if (field.name == "position")
		{
			m_P = (V3f*)field.as<float>();
			//offset = m_geom->offset();
		}
		if (QString::fromStdString(field.name).toLower().toStdString() == mAttribute.toLower().toStdString())
		{
			//uint16_t* bufferData = field.data.get();
			intensity = field.as<uint16_t>();
		}
		if (mAttribute == QStringLiteral("Z"))
		{
			is_Z = true;
		}
	}
	if (is_Z ==false)
	{
		if (intensity == nullptr)
		{
			return;
		}
	}

	V3d Vertex;
	double attributeValue;
	int red;
	int green;
	int blue;
	int  alpha;

	while (mdrawlist.index.size() > decimal_step && mdrawlist.numVertices>1)
	{
		std::list<size_t>::iterator it = mdrawlist.index.begin();
		try
		{
      if ( !m_geom->getPointByIndex(*it, Vertex))
      {
        return;
      }
			if (is_Z)
			{
				attributeValue = Vertex.z;
			}
			else
			{
				attributeValue = intensity[(*it)];
			}
			mColorRampShader.shade(attributeValue, &red, &green, &blue, &alpha);
			drawPoint(Vertex.x, Vertex.y, QColor(red, green, blue, alpha), context);
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
	intensity = nullptr;
	context.incrementPointsRendered(rendered);
}

QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > r = qgis::make_unique< QgsPointCloudAttributeByRampRenderer >();

  r->setAttribute( element.attribute( QStringLiteral( "attribute" ), QStringLiteral( "Intensity" ) ) );

  QDomElement elemShader = element.firstChildElement( QStringLiteral( "colorrampshader" ) );
  r->mColorRampShader.readXml( elemShader );

  r->setMinimum( element.attribute( QStringLiteral( "min" ), QStringLiteral( "0" ) ).toDouble() );
  r->setMaximum( element.attribute( QStringLiteral( "max" ), QStringLiteral( "100" ) ).toDouble() );

  r->restoreCommonProperties( element, context );

  return r.release();
}

QDomElement QgsPointCloudAttributeByRampRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ramp" ) );
  rendererElem.setAttribute( QStringLiteral( "min" ), mMin );
  rendererElem.setAttribute( QStringLiteral( "max" ), mMax );

  rendererElem.setAttribute( QStringLiteral( "attribute" ), mAttribute );

  QDomElement elemShader = mColorRampShader.writeXml( doc );
  rendererElem.appendChild( elemShader );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

QSet<QString> QgsPointCloudAttributeByRampRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mAttribute;
  return res;
}

QList<QgsLayerTreeModelLegendNode *> QgsPointCloudAttributeByRampRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  QList< QPair< QString, QColor > > items;
  mColorRampShader.legendSymbologyItems( items );
  for ( const QPair< QString, QColor > &item : qgis::as_const( items ) )
  {
    nodes << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
  }

  return nodes;
}

QString QgsPointCloudAttributeByRampRenderer::attribute() const
{
  return mAttribute;
}

void QgsPointCloudAttributeByRampRenderer::setAttribute( const QString &attribute )
{
  mAttribute = attribute;
}

QgsColorRampShader QgsPointCloudAttributeByRampRenderer::colorRampShader() const
{
  return mColorRampShader;
}

void QgsPointCloudAttributeByRampRenderer::setColorRampShader( const QgsColorRampShader &shader )
{
  mColorRampShader = shader;
  if (mColorRampShader.isEmpty())
  {
    mColorRampShader.classifyColorRamp(10,0);
  }
}

double QgsPointCloudAttributeByRampRenderer::minimum() const
{
  return mMin;
}

void QgsPointCloudAttributeByRampRenderer::setMinimum( double minimum )
{
  mMin = minimum;
}

double QgsPointCloudAttributeByRampRenderer::maximum() const
{
  return mMax;
}

void QgsPointCloudAttributeByRampRenderer::setMaximum( double value )
{
  mMax = value;
}

