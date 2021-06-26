/***************************************************************************
    qgsattributeforminterface.cpp
     --------------------------------------
    Date                 : 12.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeforminterface.h"

#include "qgsattributeform.h"
#include "qgsfeature.h"

QgsAttributeFormInterface::QgsAttributeFormInterface( QgsAttributeForm *form )
  : mForm( form )
{
}

bool QgsAttributeFormInterface::acceptChanges( const QgsFeature &feature )
{
  Q_UNUSED( feature )
  return true;
}

void QgsAttributeFormInterface::initForm()
{
}

void QgsAttributeFormInterface::featureChanged()
{

}

QgsAttributeForm *QgsAttributeFormInterface::form()
{
  return mForm;
}

const QgsFeature &QgsAttributeFormInterface::feature()
{
  return mForm->feature();
}
