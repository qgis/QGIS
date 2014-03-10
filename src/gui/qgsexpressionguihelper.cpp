
#include "qgsexpressionguihelper.h"

#include "qgsexpressionbuilderdialog.h"

QgsExpressionGuiHelper::QgsExpressionGuiHelper( QgsVectorLayer* layer, QComboBox* fieldCombo, QAbstractButton* expressionButton, QString title, QWidget *parent )
    : QObject( parent )
    , mLayer( 0 )
    , mFieldcombo( fieldCombo )
    , mExpressionButton( expressionButton )
    , mTitle( title )
    , mParent( parent )
{
  connect( mExpressionButton, SIGNAL( clicked() ), this, SLOT( showExpressionDialog() ) );

  changeLayer( layer );
}

void QgsExpressionGuiHelper::setExpression( const QString str )
{
  if ( str.isEmpty() )
  {
    return;
  }
  int idx = mFieldcombo->findText( str );
  if ( idx == -1 )
  {
    mFieldcombo->addItem( str );
    mFieldcombo->setCurrentIndex( mFieldcombo->count() - 1 );
  }
  else
  {
    mFieldcombo->setCurrentIndex( idx );
  }
}

void QgsExpressionGuiHelper::changeLayer( QgsVectorLayer* layer )
{
  // disconnect previous layer (0 at init)
  if ( mLayer )
  {
    disconnect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( addAttribute( int ) ) );
    disconnect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( removeAttribute( int ) ) );
    disconnect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( changeLayer() ) );
  }
  mLayer = layer;
  mExpressions.empty();
  if ( !layer )
    return;

  connect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  connect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( changeLayer() ) );

  mFieldcombo->clear();
  populateFieldCombo();
}

void QgsExpressionGuiHelper::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = da;
}

void QgsExpressionGuiHelper::showExpressionDialog()
{
  if ( !mLayer )
  {
    return;
  }

  QgsExpressionBuilderDialog dlg( mLayer, mFieldcombo->currentText() , mParent );
  dlg.setWindowTitle( mTitle );
  dlg.setGeomCalculator( mDa );


  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression =  dlg.expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      mFieldcombo->addItem( expression );
      mFieldcombo->setCurrentIndex( mFieldcombo->count() - 1 );
      mExpressions << expression;
    }
  }
  if ( mParent )
  {
    mParent->activateWindow();
  }
}

void QgsExpressionGuiHelper::populateFieldCombo()
{
  if ( !mLayer )
  {
    return;
  }
  const QgsFields& fields = mLayer->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    mFieldcombo->addItem( fields[idx].name() );
  }
  foreach ( const QString expr, mExpressions )
  {
    mFieldcombo->addItem( expr );
  }
}

void QgsExpressionGuiHelper::attributeAdded( int idx )
{
  const QgsFields& fields = mLayer->pendingFields();
  mFieldcombo->insertItem( idx, fields[idx].name() );
}

void QgsExpressionGuiHelper::attributeDeleted( int idx )
{
  mFieldcombo->removeItem( idx );
}

