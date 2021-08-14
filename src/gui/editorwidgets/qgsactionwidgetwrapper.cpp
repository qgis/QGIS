/***************************************************************************
  qgsactionwidgetwrapper.cpp - QgsActionWidgetWrapper

 ---------------------
 begin                : 14.8.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsactionwidgetwrapper.h"
#include "qgsactionmanager.h"
#include "qgsexpressioncontextutils.h"

QgsActionWidgetWrapper::QgsActionWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{

}

void QgsActionWidgetWrapper::setAction( const QgsAction &action )
{
  mAction = action;
}

void QgsActionWidgetWrapper::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

bool QgsActionWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsActionWidgetWrapper::createWidget( QWidget *parent )
{
  return new QPushButton( parent );
}

void QgsActionWidgetWrapper::initWidget( QWidget *editor )
{

  mActionButton = qobject_cast<QPushButton *>( editor );

  if ( !mActionButton )
    return;

  if ( mAction.isValid()
       && layer() )
  {
    if ( mAction.isValid() && mAction.runable() )
    {
      mActionButton->setText( mAction.shortTitle() );
      connect( mActionButton, &QPushButton::clicked, this, [ & ]
      {
        const QgsAttributeEditorContext attributecontext = context();
        QgsExpressionContext expressionContext = layer()->createExpressionContext();
        expressionContext << QgsExpressionContextUtils::formScope( mFeature, attributecontext.attributeFormModeString() );
        expressionContext.setFeature( mFeature );
        mAction.run( layer(), mFeature, expressionContext );
      } );
    }
  }

}


