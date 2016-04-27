/***************************************************************************
  qgsorganizetablecolumnsdialog.cpp
  -------------------
         date                 : Feb 2016
         copyright            : Stéphane Brunner
         email                : stephane.brunner@gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDockWidget>
#include <QMessageBox>

#include "qgsorganizetablecolumnsdialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"

#include <qgsapplication.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsexpression.h>

#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsattributeaction.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsmessagebar.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsrubberband.h"
#include "qgsfield.h"
#include "qgseditorwidgetregistry.h"


QgsOrganizeTableColumnsDialog::QgsOrganizeTableColumnsDialog( const QgsVectorLayer* vl, const QStringList visble, QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
{
  setupUi( this );
  if ( vl )
  {
    mFieldsList->clear();
    const QgsFields& layerAttributes = vl->fields();
    for ( int idx = 0; idx < layerAttributes.count(); ++idx )
    {
      QListWidgetItem* item = new QListWidgetItem( layerAttributes[idx].name(), mFieldsList );
      item->setCheckState( visble.contains( layerAttributes[idx].name() ) ? Qt::Checked : Qt::Unchecked );
      switch ( vl->fields().fieldOrigin( idx ) )
      {
        case QgsFields::OriginExpression:
          item->setIcon( QgsApplication::getThemeIcon( "/mIconExpression.svg" ) );
          break;

        case QgsFields::OriginJoin:
          item->setIcon( QgsApplication::getThemeIcon( "/propertyicons/join.png" ) );
          break;

        default:
          item->setIcon( QgsApplication::getThemeIcon( "/propertyicons/attributes.png" ) );
          break;
      }

      item->setData( Qt::UserRole, idx );
    }
  }

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QgsFilterTableFieldsDialog/geometry" ).toByteArray() );
}

QgsOrganizeTableColumnsDialog::~QgsOrganizeTableColumnsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/QgsFilterTableFieldsDialog/geometry", saveGeometry() );
}

QStringList QgsOrganizeTableColumnsDialog::selectedFields() const
{
  QStringList selectionList;
  for ( int i = 0 ; i < mFieldsList->count() ; i++ )
  {
    const QListWidgetItem* item = mFieldsList->item( i );
    if ( item->checkState() == Qt::Checked )
    {
      selectionList.push_back( item->text() );
    }
  }
  return selectionList;
}
