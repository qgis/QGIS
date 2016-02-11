/***************************************************************************
                         qgsattributeeditor.cpp  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgseditorwidgetwrapper.h"

#include "qgsattributeeditor.h"
#include "qgsattributeeditorcontext.h"
#include "qgsvectorlayer.h"


QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  QgsAttributeEditorContext context;

  return createAttributeEditor( parent, editor, vl, idx, value, context );
}

QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value, QMap<int, QWidget*> &proxyWidgets )
{
  Q_UNUSED( proxyWidgets )

  QgsAttributeEditorContext context;

  return createAttributeEditor( parent, editor, vl, idx, value, context );
}

QWidget* QgsAttributeEditor::createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value, QgsAttributeEditorContext& context )
{
  QString widgetType = vl->editFormConfig()->widgetType( idx );
  QgsEditorWidgetConfig cfg = vl->editFormConfig()->widgetConfig( idx );

  QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, vl, idx, cfg, editor, parent, context );

  if ( eww )
  {
    eww->setValue( value );
    return eww->widget();
  }
  else
  {
    return nullptr;
  }
}

bool QgsAttributeEditor::retrieveValue( QWidget *editor, QgsVectorLayer *vl, int idx, QVariant &value )
{
  Q_UNUSED( vl )
  Q_UNUSED( idx )

  if ( !editor )
    return false;

  QgsEditorWidgetWrapper* wrapper = QgsEditorWidgetWrapper::fromWidget( editor );

  if ( wrapper )
  {
    value = wrapper->value();
    return true;
  }
  return false;
}

bool QgsAttributeEditor::setValue( QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  Q_UNUSED( vl )
  Q_UNUSED( idx )

  if ( !editor )
    return false;

  QgsEditorWidgetWrapper* wrapper = QgsEditorWidgetWrapper::fromWidget( editor );

  if ( wrapper )
  {
    wrapper->setValue( value );
    return true;
  }
  return false;
}
