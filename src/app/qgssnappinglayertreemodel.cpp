/***************************************************************************
  qgssnappinglayertreemodel.cpp - QgsSnappingLayerTreeView

 ---------------------
 begin                : 31.8.2016
 copyright            : (C) 2016 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QComboBox>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QAction>
#include "qgssnappinglayertreemodel.h"

#include "qgslayertree.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssnappingconfig.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsscalewidget.h"

class SnapTypeMenu: public QMenu
{
  public:
    SnapTypeMenu( const QString &title, QWidget *parent = nullptr )
      : QMenu( title, parent ) {}

    void mouseReleaseEvent( QMouseEvent *e )
    {
      QAction *action = activeAction();
      if ( action )
        action->trigger();
      else
        QMenu::mouseReleaseEvent( e );
    }

    // set focus to parent so that mTypeButton is not displayed
    void hideEvent( QHideEvent *e )
    {
      qobject_cast<QWidget *>( parent() )->setFocus();
      QMenu::hideEvent( e );
    }
};


QgsSnappingLayerDelegate::QgsSnappingLayerDelegate( QgsMapCanvas *canvas, QObject *parent )
  : QItemDelegate( parent )
  , mCanvas( canvas )
{
}

QWidget *QgsSnappingLayerDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  Q_UNUSED( index )

  if ( index.column() == QgsSnappingLayerTreeModel::TypeColumn )
  {
    // type button
    QToolButton *mTypeButton = new QToolButton( parent );
    mTypeButton->setToolTip( tr( "Snapping Type" ) );
    mTypeButton->setPopupMode( QToolButton::InstantPopup );
    SnapTypeMenu *typeMenu = new SnapTypeMenu( tr( "Set Snapping Mode" ), parent );

    for ( Qgis::SnappingType type : qgsEnumList<Qgis::SnappingType>() )
    {
      if ( type == Qgis::SnappingType::NoSnap )
        continue;
      QAction *action = new QAction( QgsSnappingConfig::snappingTypeToIcon( type ), QgsSnappingConfig::snappingTypeToString( type ), typeMenu );
      action->setData( QVariant::fromValue( type ) );
      action->setCheckable( true );
      typeMenu->addAction( action );
    }
    mTypeButton->setMenu( typeMenu );
    mTypeButton->setObjectName( QStringLiteral( "SnappingTypeButton" ) );
    mTypeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    return mTypeButton;
  }

  if ( index.column() == QgsSnappingLayerTreeModel::ToleranceColumn )
  {
    QDoubleSpinBox *w = new QDoubleSpinBox( parent );
    w->setMaximum( 99999999.990000 );
    const QVariant val = index.model()->data( index.model()->sibling( index.row(), QgsSnappingLayerTreeModel::UnitsColumn, index ), Qt::UserRole );
    if ( val.isValid() )
    {
      const QgsTolerance::UnitType units = static_cast<QgsTolerance::UnitType>( val.toInt() );
      if ( units == QgsTolerance::Pixels )
      {
        w->setDecimals( 0 );
      }
      else
      {
        const QgsUnitTypes::DistanceUnitType type = QgsUnitTypes::unitType( mCanvas->mapUnits() );
        w->setDecimals( type == QgsUnitTypes::Standard ? 2 : 5 );
      }
    }
    else
    {
      w->setDecimals( 5 );
    }
    return w;
  }

  if ( index.column() == QgsSnappingLayerTreeModel::UnitsColumn )
  {
    QComboBox *w = new QComboBox( parent );
    w->addItem( tr( "px" ), QgsTolerance::Pixels );
    w->addItem( QgsUnitTypes::toString( mCanvas->mapSettings().mapUnits() ), QgsTolerance::ProjectUnits );
    return w;
  }

  if ( index.column() == QgsSnappingLayerTreeModel::MinScaleColumn )
  {
    QgsScaleWidget *minLimitSp = new QgsScaleWidget( parent );
    minLimitSp->setToolTip( tr( "Minimum scale from which snapping is enabled (i.e. most \"zoomed out\" scale)" ) );
    connect( minLimitSp, &QgsScaleWidget::scaleChanged, this, &QgsSnappingLayerDelegate::onScaleChanged );
    return minLimitSp;
  }

  if ( index.column() == QgsSnappingLayerTreeModel::MaxScaleColumn )
  {
    QgsScaleWidget *maxLimitSp = new QgsScaleWidget( parent );
    maxLimitSp->setToolTip( tr( "Maximum scale up to which snapping is enabled (i.e. most \"zoomed in\" scale)" ) );
    connect( maxLimitSp, &QgsScaleWidget::scaleChanged, this, &QgsSnappingLayerDelegate::onScaleChanged );
    return maxLimitSp;
  }

  return nullptr;
}

void QgsSnappingLayerDelegate::onScaleChanged()
{
  emit commitData( qobject_cast<QgsScaleWidget *>( sender() ) );
}

void QgsSnappingLayerDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QVariant val = index.model()->data( index, Qt::UserRole );
  if ( !val.isValid() )
    return;

  if ( index.column() == QgsSnappingLayerTreeModel::TypeColumn )
  {
    const Qgis::SnappingTypes type = static_cast<Qgis::SnappingTypes>( val.toInt() );
    QToolButton *tb = qobject_cast<QToolButton *>( editor );
    if ( tb )
    {
      const QList<QAction *> actions = tb->menu()->actions();
      for ( QAction *action : actions )
      {
        action->setChecked( type & static_cast< Qgis::SnappingTypes >( action->data().toInt() ) );
      }
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::ToleranceColumn )
  {
    QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox *>( editor );
    if ( w )
    {
      w->setValue( val.toDouble() );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::UnitsColumn )
  {
    const QgsTolerance::UnitType units = static_cast<QgsTolerance::UnitType>( val.toInt() );
    QComboBox *w = qobject_cast<QComboBox *>( editor );
    if ( w )
    {
      w->setCurrentIndex( w->findData( units ) );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::MinScaleColumn )
  {
    QgsScaleWidget *w = qobject_cast<QgsScaleWidget *>( editor );
    if ( w )
    {
      w->setScale( val.toDouble() );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::MaxScaleColumn )
  {
    QgsScaleWidget *w = qobject_cast<QgsScaleWidget *>( editor );
    if ( w )
    {
      w->setScale( val.toDouble() );
    }
  }
}

void QgsSnappingLayerDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( index.column() == QgsSnappingLayerTreeModel::TypeColumn )
  {
    QToolButton *t = qobject_cast<QToolButton *>( editor );
    if ( t )
    {
      const QList<QAction *> actions = t->menu()->actions();
      Qgis::SnappingTypes type = Qgis::SnappingType::NoSnap;

      for ( QAction *action : actions )
      {
        if ( action->isChecked() )
        {
          const Qgis::SnappingTypes actionFlag = static_cast<Qgis::SnappingTypes>( action->data().toInt() );
          type = static_cast<Qgis::SnappingTypes>( type | actionFlag );
        }
      }
      model->setData( index, static_cast<int>( type ), Qt::EditRole );
    }

  }
  else if (
    index.column() == QgsSnappingLayerTreeModel::UnitsColumn )
  {
    QComboBox *w = qobject_cast<QComboBox *>( editor );
    if ( w )
    {
      model->setData( index, w->currentData(), Qt::EditRole );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::ToleranceColumn )
  {
    QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox *>( editor );
    if ( w )
    {
      model->setData( index, w->value(), Qt::EditRole );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::MinScaleColumn )
  {
    QgsScaleWidget *w = qobject_cast<QgsScaleWidget *>( editor );
    if ( w )
    {
      model->setData( index, w->scale(), Qt::EditRole );
    }
  }
  else if ( index.column() == QgsSnappingLayerTreeModel::MaxScaleColumn )
  {
    QgsScaleWidget *w = qobject_cast<QgsScaleWidget *>( editor );
    if ( w )
    {
      model->setData( index, w->scale(), Qt::EditRole );
    }
  }
}


QgsSnappingLayerTreeModel::QgsSnappingLayerTreeModel( QgsProject *project, QgsMapCanvas *canvas, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mProject( project )
  , mCanvas( canvas )
  , mIndividualLayerSettings( project->snappingConfig().individualLayerSettings() )

{
  connect( project, &QgsProject::snappingConfigChanged, this, &QgsSnappingLayerTreeModel::onSnappingSettingsChanged );
  connect( project, &QgsProject::avoidIntersectionsLayersChanged, this, &QgsSnappingLayerTreeModel::onSnappingSettingsChanged );
  connect( project, &QgsProject::readProject, this, [ = ] {resetLayerTreeModel();} );
}

int QgsSnappingLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 7;
}

Qt::ItemFlags QgsSnappingLayerTreeModel::flags( const QModelIndex &idx ) const
{
  if ( idx.column() == LayerColumn )
  {
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }

  QgsVectorLayer *vl = vectorLayer( idx );
  if ( !vl )
  {
    return Qt::NoItemFlags;
  }
  else
  {
    const QModelIndex layerIndex = sibling( idx.row(), LayerColumn, idx );
    if ( data( layerIndex, Qt::CheckStateRole ) == Qt::Checked )
    {
      if ( idx.column() == AvoidIntersectionColumn )
      {
        if ( vl->geometryType() == QgsWkbTypes::PolygonGeometry )
        {
          return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
        }
        else
        {
          return Qt::NoItemFlags;
        }
      }
      else if ( idx.column() == MaxScaleColumn || idx.column() == MinScaleColumn )
      {
        if ( mProject->snappingConfig().scaleDependencyMode() == QgsSnappingConfig::PerLayer )
        {
          return Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
      }
      else
      {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
      }
    }
  }
  return Qt::NoItemFlags;
}

QModelIndex QgsSnappingLayerTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || column < 0 || row >= rowCount( parent ) || column >= columnCount( parent ) )
  {
    return QModelIndex();
  }

  QModelIndex newIndex = QSortFilterProxyModel::index( row, LayerColumn, parent );
  if ( column == LayerColumn )
    return newIndex;

  return createIndex( row, column, newIndex.internalId() );
}

QModelIndex QgsSnappingLayerTreeModel::parent( const QModelIndex &child ) const
{
  return QSortFilterProxyModel::parent( createIndex( child.row(), LayerColumn, child.internalId() ) );
}

QModelIndex QgsSnappingLayerTreeModel::sibling( int row, int column, const QModelIndex &idx ) const
{
  const QModelIndex parent = idx.parent();
  return index( row, column, parent );
}

QgsVectorLayer *QgsSnappingLayerTreeModel::vectorLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *node = nullptr;
  if ( idx.column() == LayerColumn )
  {
    node = mLayerTreeModel->index2node( mapToSource( idx ) );
  }
  else
  {
    node = mLayerTreeModel->index2node( mapToSource( index( idx.row(), LayerColumn, idx.parent() ) ) );
  }

  if ( !node || !QgsLayerTree::isLayer( node ) )
    return nullptr;

  return qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
}

void QgsSnappingLayerTreeModel::setFilterText( const QString &filterText )
{
  if ( filterText == mFilterText )
    return;

  mFilterText = filterText;
  invalidateFilter();
}

void QgsSnappingLayerTreeModel::onSnappingSettingsChanged()
{
  const QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> oldSettings = mIndividualLayerSettings;

  for ( auto it = oldSettings.constBegin(); it != oldSettings.constEnd(); ++it )
  {
    if ( !mProject->snappingConfig().individualLayerSettings().contains( it.key() ) )
    {
      beginResetModel();
      mIndividualLayerSettings = mProject->snappingConfig().individualLayerSettings();
      endResetModel();
      return;
    }
  }
  const auto constKeys = mProject->snappingConfig().individualLayerSettings().keys();
  for ( QgsVectorLayer *vl : constKeys )
  {
    if ( !oldSettings.contains( vl ) )
    {
      beginResetModel();
      mIndividualLayerSettings = mProject->snappingConfig().individualLayerSettings();
      endResetModel();
      return;
    }
  }

  hasRowchanged( mLayerTreeModel->rootGroup(), oldSettings );
}

void QgsSnappingLayerTreeModel::hasRowchanged( QgsLayerTreeNode *node, const QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> &oldSettings )
{
  if ( node->nodeType() == QgsLayerTreeNode::NodeGroup )
  {
    const auto constChildren = node->children();
    for ( QgsLayerTreeNode *child : constChildren )
    {
      hasRowchanged( child, oldSettings );
    }
  }
  else
  {
    const QModelIndex idx = mapFromSource( mLayerTreeModel->node2index( node ) );
    QgsVectorLayer *vl = vectorLayer( idx );
    if ( !vl )
    {
      emit dataChanged( QModelIndex(), idx );
    }
    if ( oldSettings.value( vl ) != mProject->snappingConfig().individualLayerSettings().value( vl ) )
    {
      mIndividualLayerSettings.insert( vl, mProject->snappingConfig().individualLayerSettings().value( vl ) );
      emit dataChanged( idx, index( idx.row(), columnCount( idx ) - 1 ) );
    }
  }
}

QgsLayerTreeModel *QgsSnappingLayerTreeModel::layerTreeModel() const
{
  return mLayerTreeModel;
}

void QgsSnappingLayerTreeModel::setLayerTreeModel( QgsLayerTreeModel *layerTreeModel )
{
  mLayerTreeModel = layerTreeModel;
  QSortFilterProxyModel::setSourceModel( layerTreeModel );
}

bool QgsSnappingLayerTreeModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayerTreeNode *node = mLayerTreeModel->index2node( mLayerTreeModel->index( sourceRow, LayerColumn, sourceParent ) );
  return nodeShown( node );
}

bool QgsSnappingLayerTreeModel::nodeShown( QgsLayerTreeNode *node ) const
{
  if ( !node )
    return false;
  if ( node->nodeType() == QgsLayerTreeNode::NodeGroup )
  {
    const auto constChildren = node->children();
    for ( QgsLayerTreeNode *child : constChildren )
    {
      if ( nodeShown( child ) )
      {
        return true;
      }
    }
    return false;
  }
  else
  {
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
    return layer && layer->isSpatial() && ( mFilterText.isEmpty() || layer->name().contains( mFilterText, Qt::CaseInsensitive ) );
  }
}

QVariant QgsSnappingLayerTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    if ( role == Qt::DisplayRole )
    {
      switch ( section )
      {
        case 0:
          return tr( "Layer" );
        case 1:
          return tr( "Type" );
        case 2:
          return tr( "Tolerance" );
        case 3:
          return tr( "Units" );
        case 4:
          return tr( "Avoid Overlap" );
        case 5:
          return tr( "Min Scale" );
        case 6:
          return tr( "Max Scale" );
        default:
          return QVariant();
      }
    }
  }
  return mLayerTreeModel->headerData( section, orientation, role );
}

QVariant QgsSnappingLayerTreeModel::data( const QModelIndex &idx, int role ) const
{
  if ( idx.column() == LayerColumn )
  {
    if ( role == Qt::CheckStateRole )
    {
      QgsVectorLayer *vl = vectorLayer( idx );
      if ( vl  && mIndividualLayerSettings.contains( vl ) )
      {
        const QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
        if ( !ls.valid() )
        {
          return QVariant();
        }
        if ( ls.enabled() )
        {
          return Qt::Checked;
        }
        else
        {
          return Qt::Unchecked;
        }
      }
      else
      {
        // i.e. this is a group, analyze its children
        bool hasChecked = false, hasUnchecked = false;
        int n;
        for ( n = 0; !hasChecked || !hasUnchecked; n++ )
        {
          const QVariant v = data( index( n, LayerColumn, idx ), role );
          if ( !v.isValid() )
            break;

          switch ( v.toInt() )
          {
            case Qt::PartiallyChecked:
              // parent of partially checked child shared state
              return Qt::PartiallyChecked;

            case Qt::Checked:
              hasChecked = true;
              break;

            case Qt::Unchecked:
              hasUnchecked = true;
              break;
          }
        }

        // unchecked leaf
        if ( n == 0 )
          return Qt::Unchecked;

        // both
        if ( hasChecked &&  hasUnchecked )
          return Qt::PartiallyChecked;

        if ( hasChecked )
          return Qt::Checked;

        Q_ASSERT( hasUnchecked );
        return Qt::Unchecked;
      }
    }
    else
    {
      return mLayerTreeModel->data( mapToSource( idx ), role );
    }
  }
  else
  {
    QgsVectorLayer *vl = vectorLayer( idx );

    if ( !vl || !mIndividualLayerSettings.contains( vl ) )
    {
      return QVariant();
    }

    const QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );

    // type
    if ( idx.column() == TypeColumn )
    {
      if ( role == Qt::DisplayRole )
      {
        if ( ls.typeFlag().testFlag( Qgis::SnappingType::NoSnap ) )
        {
          return QgsSnappingConfig::snappingTypeToString( Qgis::SnappingType::NoSnap );
        }
        else
        {
          QString modes;
          int activeTypes = 0;

          for ( Qgis::SnappingType type : qgsEnumList<Qgis::SnappingType>() )
          {
            if ( ls.typeFlag().testFlag( type ) )
            {
              if ( activeTypes == 2 )
              {
                modes.append( tr( ", …" ) );
                break;
              }
              if ( activeTypes > 0 )
                modes.append( tr( ", " ) );
              modes.append( QgsSnappingConfig::snappingTypeToString( type ) );
              activeTypes++;
            }
          }

          return modes;
        }
      }

      if ( role == Qt::UserRole )
        return static_cast<int>( ls.typeFlag() );
    }

    // tolerance
    if ( idx.column() == ToleranceColumn )
    {
      if ( role == Qt::DisplayRole )
      {
        return QLocale().toString( ls.tolerance() );
      }

      if ( role == Qt::UserRole )
      {
        return ls.tolerance();
      }
    }

    // units
    if ( idx.column() == UnitsColumn )
    {
      if ( role == Qt::DisplayRole )
      {
        switch ( ls.units() )
        {
          case QgsTolerance::Pixels:
            return tr( "pixels" );
          case QgsTolerance::ProjectUnits:
            return QgsUnitTypes::toString( mCanvas->mapSettings().mapUnits() );
          default:
            return QVariant();
        }
      }

      if ( role == Qt::UserRole )
      {
        return ls.units();
      }
    }

    // avoid intersection(Overlap)
    if ( idx.column() == AvoidIntersectionColumn )
    {
      if ( role == Qt::CheckStateRole && vl->geometryType() == QgsWkbTypes::PolygonGeometry )
      {
        if ( mProject->avoidIntersectionsLayers().contains( vl ) )
        {
          return Qt::Checked;
        }
        else
        {
          return Qt::Unchecked;
        }
      }
    }

    if ( idx.column() == MinScaleColumn )
    {
      if ( role == Qt::DisplayRole )
      {
        if ( ls.minimumScale() <= 0.0 )
        {
          return tr( "not set" );
        }
        else
        {
          return QLocale().toString( ls.minimumScale() );
        }
      }

      if ( role == Qt::UserRole )
      {
        return ls.minimumScale();
      }
    }

    if ( idx.column() == MaxScaleColumn )
    {
      if ( role == Qt::DisplayRole )
      {
        if ( ls.maximumScale() <= 0.0 )
        {
          return tr( "not set" );
        }
        else
        {
          return QLocale().toString( ls.maximumScale() );
        }
      }

      if ( role == Qt::UserRole )
      {
        return ls.maximumScale();
      }
    }
  }

  return QVariant();
}

bool QgsSnappingLayerTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == LayerColumn )
  {
    if ( role == Qt::CheckStateRole )
    {
      int i = 0;
      for ( i = 0; ; i++ )
      {
        const QModelIndex child = QgsSnappingLayerTreeModel::index( i, LayerColumn, index );
        if ( !child.isValid() )
          break;

        setData( child, value, role );
      }

      if ( i == 0 )
      {
        QgsVectorLayer *vl = vectorLayer( index );
        if ( !vl || !mIndividualLayerSettings.contains( vl ) )
        {
          return false;
        }
        QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
        if ( !ls.valid() )
          return false;
        if ( value.toInt() == Qt::Checked )
          ls.setEnabled( true );
        else if ( value.toInt() == Qt::Unchecked )
          ls.setEnabled( false );
        else
          Q_ASSERT( false ); // expected checked or unchecked

        QgsSnappingConfig config = mProject->snappingConfig();
        config.setIndividualLayerSettings( vl, ls );
        mProject->setSnappingConfig( config );
      }
      emit dataChanged( index, index );
      return true;
    }

    return mLayerTreeModel->setData( mapToSource( index ), value, role );
  }

  if ( index.column() == TypeColumn && role == Qt::EditRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
      if ( !ls.valid() )
        return false;

      ls.setTypeFlag( static_cast<Qgis::SnappingTypes>( value.toInt() ) );
      QgsSnappingConfig config = mProject->snappingConfig();
      config.setIndividualLayerSettings( vl, ls );
      mProject->setSnappingConfig( config );
      emit dataChanged( index, index );
      return true;
    }
  }

  if ( index.column() == ToleranceColumn && role == Qt::EditRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
      if ( !ls.valid() )
        return false;

      ls.setTolerance( value.toDouble() );
      QgsSnappingConfig config = mProject->snappingConfig();
      config.setIndividualLayerSettings( vl, ls );
      mProject->setSnappingConfig( config );
      emit dataChanged( index, index );
      return true;
    }
  }

  if ( index.column() == UnitsColumn && role == Qt::EditRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
      if ( !ls.valid() )
        return false;

      ls.setUnits( static_cast<QgsTolerance::UnitType>( value.toInt() ) );
      QgsSnappingConfig config = mProject->snappingConfig();
      config.setIndividualLayerSettings( vl, ls );
      mProject->setSnappingConfig( config );
      emit dataChanged( index, index );
      return true;
    }
  }

  if ( index.column() == AvoidIntersectionColumn && role == Qt::CheckStateRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QList<QgsVectorLayer *> avoidIntersectionsList = mProject->avoidIntersectionsLayers();

      if ( value.toInt() == Qt::Checked && !avoidIntersectionsList.contains( vl ) )
        avoidIntersectionsList.append( vl );
      else
        avoidIntersectionsList.removeAll( vl );

      mProject->setAvoidIntersectionsLayers( avoidIntersectionsList );
      emit dataChanged( index, index );
      return true;
    }
  }

  if ( index.column() == MinScaleColumn && role == Qt::EditRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
      if ( !ls.valid() )
        return false;

      ls.setMinimumScale( value.toDouble() );
      QgsSnappingConfig config = mProject->snappingConfig();
      config.setIndividualLayerSettings( vl, ls );
      mProject->setSnappingConfig( config );
      emit dataChanged( index, index );
      return true;
    }
  }

  if ( index.column() == MaxScaleColumn && role == Qt::EditRole )
  {
    QgsVectorLayer *vl = vectorLayer( index );
    if ( vl )
    {
      if ( !mIndividualLayerSettings.contains( vl ) )
        return false;

      QgsSnappingConfig::IndividualLayerSettings ls = mIndividualLayerSettings.value( vl );
      if ( !ls.valid() )
        return false;

      ls.setMaximumScale( value.toDouble() );
      QgsSnappingConfig config = mProject->snappingConfig();
      config.setIndividualLayerSettings( vl, ls );
      mProject->setSnappingConfig( config );
      emit dataChanged( index, index );
      return true;
    }
  }

  return false;
}
