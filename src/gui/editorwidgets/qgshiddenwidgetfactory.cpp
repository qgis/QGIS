/***************************************************************************
    qgshiddenwidgetfactory.cpp
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

#include "qgshiddenwidgetfactory.h"

#include "qgshiddenwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsHiddenWidgetFactory::QgsHiddenWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsHiddenWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsHiddenWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsHiddenWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "A hidden field will be invisible - the user is not able to see its contents." ) );
}
