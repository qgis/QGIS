/***************************************************************************
    qgsrelationreferencewidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsrelationreferencewidget.h"

QgsRelationReferenceWidgetWrapper::QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent )
  : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
  , mCanvas( canvas )
  , mMessageBar( messageBar )
  , mIndeterminateState( false )
{
}

QWidget *QgsRelationReferenceWidgetWrapper::createWidget( QWidget *parent )
{
  QgsRelationReferenceWidget *w = new QgsRelationReferenceWidget( parent );
  return w;
}

void QgsRelationReferenceWidgetWrapper::initWidget( QWidget *editor )
{
  QgsRelationReferenceWidget *w = dynamic_cast<QgsRelationReferenceWidget *>( editor );
  if ( !w )
  {
    w = new QgsRelationReferenceWidget( editor );
  }

  mWidget = w;

  mWidget->setEditorContext( context(), mCanvas, mMessageBar );

  bool showForm = config( QStringLiteral( "ShowForm" ), false ).toBool();
  bool mapIdent = config( QStringLiteral( "MapIdentification" ), false ).toBool();
  bool readOnlyWidget = config( QStringLiteral( "ReadOnly" ), false ).toBool();
  bool orderByValue = config( QStringLiteral( "OrderByValue" ), false ).toBool();
  bool showOpenFormButton = config( QStringLiteral( "ShowOpenFormButton" ), true ).toBool();

  mWidget->setEmbedForm( showForm );
  mWidget->setReadOnlySelector( readOnlyWidget );
  mWidget->setAllowMapIdentification( mapIdent );
  mWidget->setOrderByValue( orderByValue );
  mWidget->setOpenFormButtonVisible( showOpenFormButton );
  if ( config( QStringLiteral( "FilterFields" ), QVariant() ).isValid() )
  {
    mWidget->setFilterFields( config( QStringLiteral( "FilterFields" ) ).toStringList() );
    mWidget->setChainFilters( config( QStringLiteral( "ChainFilters" ) ).toBool() );
  }
  mWidget->setAllowAddFeatures( config( QStringLiteral( "AllowAddFeatures" ), false ).toBool() );

  const QVariant relationName = config( QStringLiteral( "Relation" ) );
  QgsRelation relation = relationName.isValid() ?
                         QgsProject::instance()->relationManager()->relation( relationName.toString() ) :
                         layer()->referencingRelations( fieldIdx() )[0];

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext *ctx = &context();
  do
  {
    if ( ctx->relation().name() == relation.name() )
    {
      mWidget->setEmbedForm( false );
      mWidget->setReadOnlySelector( false );
      mWidget->setAllowMapIdentification( false );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  mWidget->setRelation( relation, config( QStringLiteral( "AllowNULL" ) ).toBool() );

  connect( mWidget, &QgsRelationReferenceWidget::foreignKeyChanged, this, &QgsRelationReferenceWidgetWrapper::foreignKeyChanged );
}

QVariant QgsRelationReferenceWidgetWrapper::value() const
{
  if ( !mWidget )
    return QVariant( field().type() );

  QVariant v = mWidget->foreignKey();

  if ( v.isNull() )
  {
    return QVariant( field().type() );
  }
  else
  {
    return v;
  }
}

bool QgsRelationReferenceWidgetWrapper::valid() const
{
  return mWidget;
}

void QgsRelationReferenceWidgetWrapper::showIndeterminateState()
{
  if ( mWidget )
  {
    mWidget->showIndeterminateState();
  }
  mIndeterminateState = true;
}

void QgsRelationReferenceWidgetWrapper::setValue( const QVariant &val )
{
  if ( !mWidget || ( !mIndeterminateState && val == value() && val.isNull() == value().isNull() ) )
    return;

  mIndeterminateState = false;
  mWidget->setForeignKey( val );
}

void QgsRelationReferenceWidgetWrapper::setEnabled( bool enabled )
{
  if ( !mWidget )
    return;

  mWidget->setRelationEditable( enabled );
}

void QgsRelationReferenceWidgetWrapper::foreignKeyChanged( QVariant value )
{
  if ( !value.isValid() || value.isNull() )
  {
    value = QVariant( field().type() );
  }
  emit valueChanged( value );
}

void QgsRelationReferenceWidgetWrapper::updateConstraintWidgetStatus( ConstraintResult status )
{
  if ( mWidget )
  {
    switch ( status )
    {
      case ConstraintResultPass:
        mWidget->setStyleSheet( QString() );
        break;

      case ConstraintResultFailHard:
        mWidget->setStyleSheet( QStringLiteral( ".QComboBox { background-color: #dd7777; }" ) );
        break;

      case ConstraintResultFailSoft:
        mWidget->setStyleSheet( QStringLiteral( ".QComboBox { background-color: #ffd85d; }" ) );
        break;
    }
  }
}
