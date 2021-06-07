/***************************************************************************
    qgsuniquevaluewidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
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

#include "qgsuniquevaluewidgetfactory.h"

#include "qgsuniquevaluewidgetwrapper.h"
#include "qgsuniquevaluesconfigdlg.h"

QgsUniqueValueWidgetFactory::QgsUniqueValueWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper *QgsUniqueValueWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsUniqueValuesWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsUniqueValueWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsUniqueValuesConfigDlg( vl, fieldIdx, parent );
}
