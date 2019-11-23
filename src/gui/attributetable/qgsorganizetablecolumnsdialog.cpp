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

#include <QMessageBox>

#include "qgsorganizetablecolumnsdialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"
#include "qgsdockwidget.h"

#include "qgsapplication.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"

#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsmessagebar.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsrubberband.h"
#include "qgsfields.h"
#include "qgseditorwidgetregistry.h"

#include "qgsgui.h"


QgsOrganizeTableColumnsDialog::QgsOrganizeTableColumnsDialog( const QgsVectorLayer *vl, const QgsAttributeTableConfig &config, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mShowAllButton, &QAbstractButton::clicked, this, &QgsOrganizeTableColumnsDialog::showAll );
  connect( mHideAllButton, &QAbstractButton::clicked, this, &QgsOrganizeTableColumnsDialog::hideAll );

  if ( vl )
  {
    mConfig = config;
    mConfig.update( vl->fields() );

    mFieldsList->clear();

    const auto constColumns = mConfig.columns();
    for ( const QgsAttributeTableConfig::ColumnConfig &columnConfig : constColumns )
    {
      QListWidgetItem *item = nullptr;
      if ( columnConfig.type == QgsAttributeTableConfig::Action )
      {
        item = new QListWidgetItem( tr( "[Action Widget]" ), mFieldsList );
        item->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/action.svg" ) ) );
      }
      else
      {
        int idx = vl->fields().lookupField( columnConfig.name );
        item = new QListWidgetItem( vl->attributeDisplayName( idx ), mFieldsList );

        switch ( vl->fields().fieldOrigin( idx ) )
        {
          case QgsFields::OriginExpression:
            item->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
            break;

          case QgsFields::OriginJoin:
            item->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/join.svg" ) ) );
            break;

          default:
            item->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/attributes.svg" ) ) );
            break;
        }
      }

      item->setCheckState( columnConfig.hidden ? Qt::Unchecked : Qt::Checked );
      item->setData( Qt::UserRole, QVariant::fromValue( columnConfig ) );
    }
  }

  if ( !vl || mConfig.columns().count() < 7 )
  {
    mShowAllButton->hide();
    mHideAllButton->hide();
  }
}

///@cond PRIVATE
QgsOrganizeTableColumnsDialog::QgsOrganizeTableColumnsDialog( const QgsVectorLayer *vl, QWidget *parent, Qt::WindowFlags flags )
  : QgsOrganizeTableColumnsDialog( vl, vl->attributeTableConfig(), parent, flags )
{
}
///@endcond

QgsAttributeTableConfig QgsOrganizeTableColumnsDialog::config() const
{
  QVector<QgsAttributeTableConfig::ColumnConfig> columns;
  columns.reserve( mFieldsList->count() );

  for ( int i = 0; i < mFieldsList->count() ; i++ )
  {
    const QListWidgetItem *item = mFieldsList->item( i );
    QgsAttributeTableConfig::ColumnConfig columnConfig = item->data( Qt::UserRole ).value<QgsAttributeTableConfig::ColumnConfig>();

    columnConfig.hidden = item->checkState() == Qt::Unchecked;

    columns.append( columnConfig );
  }

  QgsAttributeTableConfig config = mConfig;
  config.setColumns( columns );
  return config;
}

void QgsOrganizeTableColumnsDialog::showAll()
{
  for ( int i = 0; i < mFieldsList->count() ; i++ )
  {
    mFieldsList->item( i )->setCheckState( Qt::Checked );
  }
}

void QgsOrganizeTableColumnsDialog::hideAll()
{
  for ( int i = 0; i < mFieldsList->count() ; i++ )
  {
    mFieldsList->item( i )->setCheckState( Qt::Unchecked );
  }
}
