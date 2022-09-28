/***************************************************************************
                         qgssymbolsavedialog.cpp
                         ---------------------------------------
    begin                : November 2016
    copyright            : (C) 2016 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylesavedialog.h"

#include "qgis.h"
#include "qgsstyle.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsprojectstylesettings.h"

#include <QLineEdit>
#include <QCheckBox>

QgsStyleSaveDialog::QgsStyleSaveDialog( QWidget *parent, QgsStyle::StyleEntity type )
  : QDialog( parent )
  , mType( type )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  QStringList defaultTags = QgsStyle::defaultStyle()->tags();
  defaultTags.sort( Qt::CaseInsensitive );
  mTags->addItems( defaultTags );

  QList< QgsStyle::StyleEntity > possibleEntities;
  switch ( type )
  {
    case QgsStyle::SymbolEntity:
      this->setWindowTitle( tr( "Save New Symbol" ) );
      possibleEntities << QgsStyle::SymbolEntity;
      break;

    case QgsStyle::ColorrampEntity:
      this->setWindowTitle( tr( "Save New Color Ramp" ) );
      possibleEntities << QgsStyle::ColorrampEntity;
      break;

    case QgsStyle::TextFormatEntity:
      this->setWindowTitle( tr( "Save New Text Format" ) );
      possibleEntities << QgsStyle::TextFormatEntity;
      break;

    case QgsStyle::LabelSettingsEntity:
      this->setWindowTitle( tr( "Save New Label Settings" ) );
      possibleEntities << QgsStyle::LabelSettingsEntity << QgsStyle::TextFormatEntity;
      break;

    case QgsStyle::LegendPatchShapeEntity:
      this->setWindowTitle( tr( "Save New Legend Patch Shape" ) );
      possibleEntities << QgsStyle::LegendPatchShapeEntity;
      break;

    case QgsStyle::Symbol3DEntity:
      this->setWindowTitle( tr( "Save New 3D Symbol" ) );
      possibleEntities << QgsStyle::Symbol3DEntity;
      break;

    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      break;
  }

  QgsProjectStyleDatabaseModel *projectStyleModel = new QgsProjectStyleDatabaseModel( QgsProject::instance()->styleSettings(), this );
  QgsProjectStyleDatabaseProxyModel *styleProxyModel = new QgsProjectStyleDatabaseProxyModel( projectStyleModel, this );
  styleProxyModel->setFilters( QgsProjectStyleDatabaseProxyModel::Filter::FilterHideReadOnly );

  if ( styleProxyModel->rowCount( QModelIndex() ) == 0 )
  {
    mLabelDestination->hide();
    mComboBoxDestination->hide();
  }
  else
  {
    projectStyleModel->setShowDefaultStyle( true );
    mComboBoxDestination->setModel( styleProxyModel );
  }

  if ( possibleEntities.size() < 2 )
  {
    mLabelSaveAs->hide();
    mComboSaveAs->hide();
  }
  else
  {
    for ( QgsStyle::StyleEntity e : std::as_const( possibleEntities ) )
    {
      switch ( e )
      {
        case QgsStyle::SymbolEntity:
          mComboSaveAs->addItem( tr( "Symbol" ), e );
          break;

        case QgsStyle::ColorrampEntity:
          mComboSaveAs->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/color.svg" ) ), tr( "Color Ramp" ), e );
          break;

        case QgsStyle::TextFormatEntity:
          mComboSaveAs->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconFieldText.svg" ) ), tr( "Text Format" ), e );
          break;

        case QgsStyle::LabelSettingsEntity:
          mComboSaveAs->addItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), tr( "Label Settings" ), e );
          break;

        case QgsStyle::LegendPatchShapeEntity:
          mComboSaveAs->addItem( QgsApplication::getThemeIcon( QStringLiteral( "legend.svg" ) ), tr( "Legend Patch Shape" ), e );
          break;

        case QgsStyle::Symbol3DEntity:
          mComboSaveAs->addItem( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), tr( "3D Symbol" ), e );
          break;

        case QgsStyle::TagEntity:
        case QgsStyle::SmartgroupEntity:
          break;
      }
    }
    mComboSaveAs->setCurrentIndex( 0 );
  }
}

QString QgsStyleSaveDialog::name() const
{
  return mName->text();
}

void QgsStyleSaveDialog::setDefaultTags( const QString &tags )
{
  mTags->setCurrentText( tags );
}

QString QgsStyleSaveDialog::tags() const
{
  return mTags->currentText();
}

bool QgsStyleSaveDialog::isFavorite() const
{
  return mFavorite->isChecked();
}

QgsStyle::StyleEntity QgsStyleSaveDialog::selectedType() const
{
  if ( mComboSaveAs->count() > 0 )
    return static_cast< QgsStyle::StyleEntity >( mComboSaveAs->currentData().toInt() );
  else
    return mType;
}

QgsStyle *QgsStyleSaveDialog::destinationStyle()
{
  if ( QgsStyle *style = qobject_cast< QgsStyle * >( mComboBoxDestination->model()->data( mComboBoxDestination->model()->index( mComboBoxDestination->currentIndex(), 0, QModelIndex() ), QgsProjectStyleDatabaseModel::StyleRole ).value< QObject * >() ) )
  {
    return style;
  }
  else
  {
    return QgsStyle::defaultStyle();
  }
}
