/***************************************************************************
    qgsattributeforminterface.cpp
     --------------------------------------
    Date                 : 12.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeforminterface.h"

#include "qgsattributeform.h"
#include "qgsfeature.h"

QgsAttributeFormInterface::QgsAttributeFormInterface( QgsAttributeForm* form )
    : mForm( form )
{
}

bool QgsAttributeFormInterface::acceptChanges( const QgsFeature& feature )
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

QgsAttributeForm* QgsAttributeFormInterface::form()
{
  return mForm;
}

const QgsFeature& QgsAttributeFormInterface::feature()
{
  return mForm->feature();
}
