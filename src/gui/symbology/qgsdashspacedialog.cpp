/***************************************************************************
    qgsdashspacedialog.cpp
    ----------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdashspacedialog.h"
#include "qgsdoublevalidator.h"
#include "qgsapplication.h"

#include <QDialogButtonBox>
#include <QFile>

QgsDashSpaceWidget::QgsDashSpaceWidget( const QVector<qreal> &vectorPattern, QWidget *parent ) : QgsPanelWidget( parent )
{
  setupUi( this );

  mAddButton->setIcon( QgsApplication::getThemeIcon( "symbologyAdd.svg" ) );
  mRemoveButton->setIcon( QgsApplication::getThemeIcon( "symbologyRemove.svg" ) );

  double total = 0;
  for ( int i = 0; i < ( vectorPattern.size() - 1 ); ++i )
  {
    const double dash = vectorPattern.at( i );
    ++i;
    const double space = vectorPattern.at( i );
    total += dash + space;
    QTreeWidgetItem *entry = new QTreeWidgetItem();
    entry->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    entry->setText( 0, QLocale().toString( dash ) );
    entry->setText( 1, QLocale().toString( space ) );
    mDashSpaceTreeWidget->addTopLevelItem( entry );
  }

  mPatternLengthLabel->setText( QLocale().toString( total, 'f', 6 ) );

  connect( mAddButton, &QPushButton::clicked, this, &QgsDashSpaceWidget::mAddButton_clicked );
  connect( mRemoveButton, &QPushButton::clicked, this, &QgsDashSpaceWidget::mRemoveButton_clicked );
  connect( mDashSpaceTreeWidget, &QTreeWidget::itemChanged, this, [ this ] { emit widgetChanged(); } );

  connect( this, &QgsPanelWidget::widgetChanged, this, [ = ]
  {
    const QVector<qreal> pattern = dashDotVector();
    double total = 0;
    for ( qreal part : pattern )
    {
      total += part;
    }
    mPatternLengthLabel->setText( QLocale().toString( total, 'f', 6 ) );
  } );
}

void QgsDashSpaceWidget::mAddButton_clicked()
{
  //add new (default) item
  QTreeWidgetItem *entry = new QTreeWidgetItem();
  entry->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
  entry->setText( 0, QStringLiteral( "5" ) );
  entry->setText( 1, QStringLiteral( "2" ) );
  mDashSpaceTreeWidget->addTopLevelItem( entry );
  emit widgetChanged();
}

void QgsDashSpaceWidget::mRemoveButton_clicked()
{
  //get active item
  QTreeWidgetItem *currentItem = mDashSpaceTreeWidget->currentItem();
  if ( currentItem )
  {
    mDashSpaceTreeWidget->takeTopLevelItem( mDashSpaceTreeWidget->indexOfTopLevelItem( currentItem ) );
  }
  emit widgetChanged();
}

QVector<qreal> QgsDashSpaceWidget::dashDotVector() const
{
  QVector<qreal> dashVector;
  const int nTopLevelItems = mDashSpaceTreeWidget->topLevelItemCount();
  dashVector.reserve( nTopLevelItems * 2 );
  for ( int i = 0; i < nTopLevelItems; ++i )
  {
    QTreeWidgetItem *currentItem = mDashSpaceTreeWidget->topLevelItem( i );
    if ( currentItem )
    {
      dashVector << QgsDoubleValidator::toDouble( currentItem->text( 0 ) ) << QgsDoubleValidator::toDouble( currentItem->text( 1 ) );
    }
  }
  return dashVector;
}

void QgsDashSpaceWidget::setUnit( QgsUnitTypes::RenderUnit unit )
{
  QTreeWidgetItem *headerItem = mDashSpaceTreeWidget->headerItem();
  headerItem->setText( 0, QStringLiteral( "%1 (%2)" ).arg( tr( "Dash" ), QgsUnitTypes::toAbbreviatedString( unit ) ) );
  headerItem->setText( 1, QStringLiteral( "%1 (%2)" ).arg( tr( "Space" ), QgsUnitTypes::toAbbreviatedString( unit ) ) );
}

QgsDashSpaceDialog::QgsDashSpaceDialog( const QVector<qreal> &v, QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsDashSpaceWidget( v );
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsDashSpaceDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsDashSpaceDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Custom Dash Pattern" ) );
}

QVector<qreal> QgsDashSpaceDialog::dashDotVector() const
{
  return mWidget->dashDotVector();
}

void QgsDashSpaceDialog::setUnit( QgsUnitTypes::RenderUnit unit )
{
  mWidget->setUnit( unit );
}
