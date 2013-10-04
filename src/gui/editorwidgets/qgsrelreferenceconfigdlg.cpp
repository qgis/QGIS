/***************************************************************************
    qgsrelreferenceconfigdlg.cpp
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelreferenceconfigdlg.h"

#include "qgseditorwidgetfactory.h"
#include "qgsfield.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

QgsRelReferenceConfigDlg::QgsRelReferenceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  foreach ( const QgsRelation& relation, vl->referencingRelations( fieldIdx ) )
  {
    QgsField fld = relation.fieldPairs().first().second;
    mComboRelation->addItem( QString( "%1 (%2)" ).arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    if ( relation.referencedLayer() )
    {
      mTxtDisplayExpression->setText( relation.referencedLayer()->displayExpression() );
    }
  }
}

void QgsRelReferenceConfigDlg::setConfig( const QMap<QString, QVariant>& config )
{
  if ( config.contains( "AllowNULL" ) )
  {
    mCbxAllowNull->setChecked( config[ "AllowNULL" ].toBool() );
  }

  if ( config.contains( "ShowForm" ) )
  {
    mCbxShowForm->setChecked( config[ "ShowForm" ].toBool() );
  }

  if ( config.contains( "Relation" ) )
  {
    mComboRelation->setCurrentIndex( mComboRelation->findData( config[ "Relation" ].toString() ) );
  }
}

void QgsRelReferenceConfigDlg::on_mComboRelation_indexChanged( int idx )
{
  QString relName = mComboRelation->itemData( idx ).toString();
  QgsRelation rel = QgsProject::instance()->relationManager()->relation( relName );

  QgsVectorLayer* referencedLayer = rel.referencedLayer();
  if ( referencedLayer )
  {
    mTxtDisplayExpression->setText( referencedLayer->displayExpression() );
  }
}

QgsEditorWidgetConfig QgsRelReferenceConfigDlg::config()
{
  QgsEditorWidgetConfig myConfig;
  myConfig.insert( "AllowNULL", mCbxAllowNull->isChecked() );
  myConfig.insert( "ShowForm", mCbxShowForm->isChecked() );
  myConfig.insert( "Relation", mComboRelation->itemData( mComboRelation->currentIndex() ) );

  return myConfig;
}
