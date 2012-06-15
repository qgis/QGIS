#include "qgsrasterrendererwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"


QString QgsRasterRendererWidget::displayBandName( int band ) const
{
  QString name;
  if ( !mRasterLayer )
  {
    return name;
  }

  const QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return name;
  }

  name = provider->colorInterpretationName( band );
  if ( name == "Undefined" )
  {
    name = provider->generateBandName( band );
  }
  return name;
}
