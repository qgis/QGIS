#include "qgsdatadefinedsymboldialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QCheckBox>


QgsDataDefinedSymbolDialog::QgsDataDefinedSymbolDialog( const QList< DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mVectorLayer( vl )
{
  setupUi( this );

  QgsFields attributeFields;
  if ( mVectorLayer )
  {
    attributeFields = mVectorLayer->pendingFields();
  }

  int i = 0;
  QList< DataDefinedSymbolEntry >::const_iterator entryIt = entries.constBegin();
  for ( ; entryIt != entries.constEnd(); ++entryIt )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem( mTreeWidget );

    //check box
    QCheckBox* cb = new QCheckBox( entryIt->title, this );
    cb->setChecked( !entryIt->initialValue.isEmpty() );
    item->setData( 0, Qt::UserRole, entryIt->property );
    mTreeWidget->setItemWidget( item, 0, cb );

    // expression
    QgsFieldExpressionWidget* few = new QgsFieldExpressionWidget( this );
    few->setMaximumWidth( 350 );
    few->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
    few->setLayer( const_cast<QgsVectorLayer*>( vl ) );
    few->setField( entryIt->initialValue );
    mTreeWidget->setItemWidget( item, 1, few );

    //help text
    item->setText( 2, entryIt->helpText );

    mTreeWidget->addTopLevelItem( item );
    ++i;
  }

  for ( int c = 0; c != mTreeWidget->columnCount() - 1; c++ )
    mTreeWidget->resizeColumnToContents( c );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QgsDataDefinedSymbolDialog/geometry" ).toByteArray() );
}

QgsDataDefinedSymbolDialog::~QgsDataDefinedSymbolDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/QgsDataDefinedSymbolDialog/geometry", saveGeometry() );
}

QMap< QString, QString > QgsDataDefinedSymbolDialog::dataDefinedProperties() const
{
  QMap< QString, QString > propertyMap;
  int rowCount = mTreeWidget->topLevelItemCount();
  for ( int i = 0; i < rowCount; ++i )
  {
    QTreeWidgetItem* item = mTreeWidget->topLevelItem( i );
    //property
    QString propertyKey = item->data( 0, Qt::UserRole ).toString();
    //checked?
    bool checked = false;
    QCheckBox* cb = qobject_cast<QCheckBox*>( mTreeWidget->itemWidget( item, 0 ) );
    if ( cb )
    {
      checked = cb->isChecked();
    }
    QString expressionString;
    if ( checked )
    {
      QgsFieldExpressionWidget* few = qobject_cast<QgsFieldExpressionWidget*>( mTreeWidget->itemWidget( item, 1 ) );
      expressionString = few->currentField();
    }
    propertyMap.insert( propertyKey, expressionString );
  }
  return propertyMap;
}

QString QgsDataDefinedSymbolDialog::doubleHelpText()
{
  return tr( "double" );
}

QString QgsDataDefinedSymbolDialog::colorHelpText()
{
  return tr( "'<red>,<green>,<blue>,<alpha>'" );
}

QString QgsDataDefinedSymbolDialog::offsetHelpText()
{
  return "'<x>,<y>'";
}

QString QgsDataDefinedSymbolDialog::fileNameHelpText()
{
  return tr( "'<filename>'" );
}

QString QgsDataDefinedSymbolDialog::horizontalAnchorHelpText()
{
  // Don't translate, localized keywords are not supported.
  return "'left'|'center'|'right'";
}

QString QgsDataDefinedSymbolDialog::verticalAnchorHelpText()
{
  // Don't translate, localized keywords are not supported.
  return "'top'|'center'|'bottom'";
}

QString QgsDataDefinedSymbolDialog::gradientTypeHelpText()
{
  return tr( "'linear'|'radial'|'conical'" );
}

QString QgsDataDefinedSymbolDialog::gradientCoordModeHelpText()
{
  return tr( "'feature'|'viewport'" );
}

QString QgsDataDefinedSymbolDialog::gradientSpreadHelpText()
{
  return tr( "'pad'|'repeat'|'reflect'" );
}

QString QgsDataDefinedSymbolDialog::boolHelpText()
{
  return tr( "0 (false)|1 (true)" );
}


