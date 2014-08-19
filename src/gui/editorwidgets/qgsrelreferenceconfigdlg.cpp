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
#include "qgsexpressionbuilderdialog.h"

QgsRelReferenceConfigDlg::QgsRelReferenceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( mComboRelation, SIGNAL( currentIndexChanged( int ) ), this, SLOT( relationChanged( int ) ) );

  foreach ( const QgsRelation& relation, vl->referencingRelations( fieldIdx ) )
  {
    mComboRelation->addItem( QString( "%1 (%2)" ).arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    if ( relation.referencedLayer() )
    {
      mExpressionWidget->setField( relation.referencedLayer()->displayExpression() );
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

  if ( config.contains( "MapIdentification" ) )
  {
    mCbxMapIdentification->setChecked( config[ "MapIdentification"].toBool() );
  }

  if ( config.contains( "ReadOnly" ) )
  {
    mCbxReadOnly->setChecked( config[ "ReadOnly"].toBool() );
  }
}

void QgsRelReferenceConfigDlg::relationChanged( int idx )
{
  QString relName = mComboRelation->itemData( idx ).toString();
  QgsRelation rel = QgsProject::instance()->relationManager()->relation( relName );

  QgsVectorLayer* referencedLayer = rel.referencedLayer();
  mExpressionWidget->setLayer( referencedLayer ); // set even if 0
  if ( referencedLayer )
  {
    mExpressionWidget->setField( referencedLayer->displayExpression() );
    mCbxMapIdentification->setEnabled( referencedLayer->hasGeometryType() );
  }
}

QgsEditorWidgetConfig QgsRelReferenceConfigDlg::config()
{
  QgsEditorWidgetConfig myConfig;
  myConfig.insert( "AllowNULL", mCbxAllowNull->isChecked() );
  myConfig.insert( "ShowForm", mCbxShowForm->isChecked() );
  myConfig.insert( "MapIdentification", mCbxMapIdentification->isEnabled() && mCbxMapIdentification->isChecked() );
  myConfig.insert( "ReadOnly", mCbxReadOnly->isChecked() );
  myConfig.insert( "Relation", mComboRelation->itemData( mComboRelation->currentIndex() ) );

  QString relName = mComboRelation->itemData( mComboRelation->currentIndex() ).toString();
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( relName );

  if ( relation.isValid() )
  {
    relation.referencedLayer()->setDisplayExpression( mExpressionWidget->currentField() );
  }

  return myConfig;
}

