/***************************************************************************
                          qgsgdalcredentialoptionswidget.h
                             -------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalcredentialoptionswidget.h"
#include "moc_qgsgdalcredentialoptionswidget.cpp"
#include "qgsgdalguiutils.h"
#include "qgsapplication.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"

#include <gdal.h>
#include <cpl_minixml.h>
#include <QComboBox>
#include <QLineEdit>
#include <QHoverEvent>


//
// QgsGdalCredentialOptionsModel
//

QgsGdalCredentialOptionsModel::QgsGdalCredentialOptionsModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

int QgsGdalCredentialOptionsModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsGdalCredentialOptionsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mCredentialOptions.size();
}

QModelIndex QgsGdalCredentialOptionsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsGdalCredentialOptionsModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsGdalCredentialOptionsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  if ( index.row() < 0 || index.row() >= mCredentialOptions.size() || index.column() < 0 || index.column() >= columnCount() )
    return Qt::ItemFlags();

  switch ( index.column() )
  {
    case Column::Key:
    case Column::Value:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable;
    case Column::Actions:
      return Qt::ItemFlag::ItemIsEnabled;
    default:
      break;
  }

  return Qt::ItemFlags();
}

QVariant QgsGdalCredentialOptionsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mCredentialOptions.size() || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const QPair<QString, QString> option = mCredentialOptions.at( index.row() );
  const QgsGdalOption gdalOption = QgsGdalCredentialOptionsModel::option( option.first );

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      switch ( index.column() )
      {
        case Column::Key:
          return option.first;

        case Column::Value:
          return gdalOption.type == QgsGdalOption::Type::Boolean ? ( option.second == QLatin1String( "YES" ) ? tr( "Yes" ) : option.second == QLatin1String( "NO" ) ? tr( "No" )
                                                                                                                                                                    : option.second )
                                                                 : option.second;

        default:
          break;
      }
      break;
    }

    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Column::Key:
          return option.first;

        case Column::Value:
          return option.second;

        default:
          break;
      }
      break;
    }

    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case Column::Key:
        case Column::Value:
          return mDescriptions.value( option.first, option.first );

        case Column::Actions:
          return tr( "Remove option" );

        default:
          break;
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

bool QgsGdalCredentialOptionsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() > mCredentialOptions.size() || index.row() < 0 )
    return false;

  QPair<QString, QString> &option = mCredentialOptions[index.row()];

  switch ( role )
  {
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Column::Key:
        {
          const bool wasInvalid = option.first.isEmpty();
          if ( value.toString().isEmpty() )
          {
            if ( wasInvalid )
              break;
          }
          else
          {
            if ( option.first != value.toString() )
              option.second = QgsGdalCredentialOptionsModel::option( value.toString() ).defaultValue.toString();

            option.first = value.toString();
            if ( wasInvalid )
            {
              emit dataChanged( createIndex( index.row(), 0 ), createIndex( index.row(), columnCount() ) );
            }
          }
          emit dataChanged( createIndex( index.row(), 0 ), createIndex( index.row(), columnCount() ) );
          if ( wasInvalid )
          {
            beginInsertRows( QModelIndex(), mCredentialOptions.size(), mCredentialOptions.size() );
            mCredentialOptions.append( qMakePair( QString(), QString() ) );
            endInsertRows();
          }
          emit optionsChanged();
          break;
        }

        case Column::Value:
        {
          option.second = value.toString();
          emit dataChanged( index, index, QVector<int>() << role );
          emit optionsChanged();
          break;
        }

        default:
          break;
      }
      return true;
    }

    default:
      break;
  }

  return false;
}

bool QgsGdalCredentialOptionsModel::insertRows( int position, int rows, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginInsertRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
  {
    mCredentialOptions.insert( position, qMakePair( QString(), QString() ) );
  }
  endInsertRows();
  emit optionsChanged();
  return true;
}

bool QgsGdalCredentialOptionsModel::removeRows( int position, int rows, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginRemoveRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
    mCredentialOptions.removeAt( position );
  endRemoveRows();

  if ( mCredentialOptions.empty() || !mCredentialOptions.last().first.isEmpty() )
  {
    beginInsertRows( QModelIndex(), mCredentialOptions.size(), mCredentialOptions.size() );
    mCredentialOptions.append( qMakePair( QString(), QString() ) );
    endInsertRows();
  }
  emit optionsChanged();
  return true;
}

void QgsGdalCredentialOptionsModel::setOptions( const QList<QPair<QString, QString>> &options )
{
  beginResetModel();
  mCredentialOptions = options;
  // last entry should always be a blank entry
  if ( mCredentialOptions.isEmpty() || !mCredentialOptions.last().first.isEmpty() )
    mCredentialOptions.append( qMakePair( QString(), QString() ) );
  endResetModel();
  emit optionsChanged();
}

void QgsGdalCredentialOptionsModel::setAvailableOptions( const QList<QgsGdalOption> &options )
{
  mAvailableOptions = options;
  mDescriptions.clear();
  mAvailableKeys.clear();
  for ( const QgsGdalOption &option : options )
  {
    mAvailableKeys.append( option.name );
    mDescriptions[option.name] = option.description;
  }
}

QgsGdalOption QgsGdalCredentialOptionsModel::option( const QString &key ) const
{
  for ( const QgsGdalOption &option : mAvailableOptions )
  {
    if ( option.name == key )
      return option;
  }
  return QgsGdalOption();
}

void QgsGdalCredentialOptionsModel::setCredentialOptions( const QList<QPair<QString, QString>> &options )
{
  beginResetModel();
  mCredentialOptions = options;

  if ( mCredentialOptions.empty() || !mCredentialOptions.last().first.isEmpty() )
  {
    mCredentialOptions.append( qMakePair( QString(), QString() ) );
  }

  endResetModel();
  emit optionsChanged();
}


//
// QgsGdalCredentialOptionsDelegate
//

QgsGdalCredentialOptionsDelegate::QgsGdalCredentialOptionsDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsGdalCredentialOptionsDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsGdalCredentialOptionsModel::Column::Key:
    {
      const QgsGdalCredentialOptionsModel *model = qgis::down_cast<const QgsGdalCredentialOptionsModel *>( index.model() );
      QComboBox *combo = new QComboBox( parent );
      const QStringList availableKeys = model->availableKeys();
      for ( const QString &key : availableKeys )
      {
        if ( key == QLatin1String( "GDAL_HTTP_MAX_RETRY" ) && combo->count() > 0 )
        {
          // add separator before generic settings
          combo->insertSeparator( combo->count() );
        }

        combo->addItem( key );
      }
      return combo;
    }

    case QgsGdalCredentialOptionsModel::Column::Value:
    {
      // need to find out key for this row
      const QgsGdalCredentialOptionsModel *model = qgis::down_cast<const QgsGdalCredentialOptionsModel *>( index.model() );
      const QString key = index.model()->data( model->index( index.row(), QgsGdalCredentialOptionsModel::Column::Key ), Qt::EditRole ).toString();
      if ( key.isEmpty() )
        return nullptr;

      const QgsGdalOption option = model->option( key );
      return QgsGdalGuiUtils::createWidgetForOption( option, parent );
    }

    default:
      break;
  }
  return nullptr;
}

void QgsGdalCredentialOptionsDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsGdalCredentialOptionsModel::Column::Key:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        combo->setCurrentIndex( combo->findText( index.data( Qt::EditRole ).toString() ) );
      }
      return;
    }

    case QgsGdalCredentialOptionsModel::Column::Value:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        combo->setCurrentIndex( combo->findData( index.data( Qt::EditRole ).toString() ) );
        if ( combo->currentIndex() < 0 )
          combo->setCurrentIndex( 0 );
      }
      else if ( QLineEdit *edit = qobject_cast<QLineEdit *>( editor ) )
      {
        edit->setText( index.data( Qt::EditRole ).toString() );
      }
      else if ( QgsSpinBox *spin = qobject_cast<QgsSpinBox *>( editor ) )
      {
        spin->setValue( index.data( Qt::EditRole ).toInt() );
      }
      else if ( QgsDoubleSpinBox *spin = qobject_cast<QgsDoubleSpinBox *>( editor ) )
      {
        spin->setValue( index.data( Qt::EditRole ).toDouble() );
      }
      return;
    }

    default:
      break;
  }
  QStyledItemDelegate::setEditorData( editor, index );
}

void QgsGdalCredentialOptionsDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsGdalCredentialOptionsModel::Column::Key:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        model->setData( index, combo->currentText() );
      }
      break;
    }

    case QgsGdalCredentialOptionsModel::Column::Value:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        model->setData( index, combo->currentData() );
      }
      else if ( QLineEdit *edit = qobject_cast<QLineEdit *>( editor ) )
      {
        model->setData( index, edit->text() );
      }
      else if ( QgsSpinBox *spin = qobject_cast<QgsSpinBox *>( editor ) )
      {
        model->setData( index, spin->value() );
      }
      else if ( QgsDoubleSpinBox *spin = qobject_cast<QgsDoubleSpinBox *>( editor ) )
      {
        model->setData( index, spin->value() );
      }
      break;
    }

    default:
      break;
  }
}


//
// QgsGdalCredentialOptionsRemoveOptionDelegate
//

QgsGdalCredentialOptionsRemoveOptionDelegate::QgsGdalCredentialOptionsRemoveOptionDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

bool QgsGdalCredentialOptionsRemoveOptionDelegate::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove )
  {
    QHoverEvent *hoverEvent = static_cast<QHoverEvent *>( event );
    if ( QAbstractItemView *view = qobject_cast<QAbstractItemView *>( obj->parent() ) )
    {
      const QModelIndex indexUnderMouse = view->indexAt( hoverEvent->pos() );
      setHoveredIndex( indexUnderMouse );
      view->viewport()->update();
    }
  }
  else if ( event->type() == QEvent::HoverLeave )
  {
    setHoveredIndex( QModelIndex() );
    qobject_cast<QWidget *>( obj )->update();
  }
  return QStyledItemDelegate::eventFilter( obj, event );
}

void QgsGdalCredentialOptionsRemoveOptionDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  if ( index == mHoveredIndex )
  {
    QStyleOptionButton buttonOption;
    buttonOption.initFrom( option.widget );
    buttonOption.rect = option.rect;

    option.widget->style()->drawControl( QStyle::CE_PushButton, &buttonOption, painter );
  }

  const QIcon icon = QgsApplication::getThemeIcon( "/mIconClearItem.svg" );
  const QRect iconRect( option.rect.left() + ( option.rect.width() - 16 ) / 2, option.rect.top() + ( option.rect.height() - 16 ) / 2, 16, 16 );

  icon.paint( painter, iconRect );
}

void QgsGdalCredentialOptionsRemoveOptionDelegate::setHoveredIndex( const QModelIndex &index )
{
  mHoveredIndex = index;
}


//
// QgsGdalCredentialOptionsWidget
//

QgsGdalCredentialOptionsWidget::QgsGdalCredentialOptionsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mLabelInfo->setText( tr( "Consult the <a href=\"%1\">GDAL documentation</a> for credential options." ).arg( QLatin1String( "https://gdal.org/user/virtual_file_systems.html#drivers-supporting-virtual-file-systems" ) ) );
  mLabelInfo->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mLabelInfo->setOpenExternalLinks( true );

  mLabelWarning->setText( tr( "Potentially sensitive credentials are configured! <b>These will be stored in plain text within the QGIS project</b>." ) );
  mLabelWarning->setVisible( false );

  mModel = new QgsGdalCredentialOptionsModel( this );
  mTableView->setModel( mModel );

  mTableView->horizontalHeader()->setVisible( false );
  mTableView->verticalHeader()->setVisible( false );
  mTableView->setEditTriggers( QAbstractItemView::AllEditTriggers );

  mDelegate = new QgsGdalCredentialOptionsDelegate( mTableView );
  mTableView->setItemDelegateForColumn( QgsGdalCredentialOptionsModel::Column::Key, mDelegate );
  mTableView->setItemDelegateForColumn( QgsGdalCredentialOptionsModel::Column::Value, mDelegate );
  mTableView->horizontalHeader()->resizeSection( QgsGdalCredentialOptionsModel::Column::Actions, QFontMetrics( mTableView->font() ).horizontalAdvance( '0' ) * 5 );
  mTableView->horizontalHeader()->setSectionResizeMode( QgsGdalCredentialOptionsModel::Column::Value, QHeaderView::ResizeMode::Stretch );

  QgsGdalCredentialOptionsRemoveOptionDelegate *removeDelegate = new QgsGdalCredentialOptionsRemoveOptionDelegate( mTableView );
  mTableView->setItemDelegateForColumn( QgsGdalCredentialOptionsModel::Column::Actions, removeDelegate );
  mTableView->viewport()->installEventFilter( removeDelegate );
  connect( mTableView, &QTableView::clicked, this, [this]( const QModelIndex &index ) {
    if ( index.column() == QgsGdalCredentialOptionsModel::Column::Actions )
    {
      mModel->removeRows( index.row(), 1 );
    }
  } );

  mModel->setOptions( {} );

  connect( mModel, &QgsGdalCredentialOptionsModel::optionsChanged, this, &QgsGdalCredentialOptionsWidget::modelOptionsChanged );
}

void QgsGdalCredentialOptionsWidget::setHandler( const QString &handler )
{
  if ( handler == mHandler )
    return;

  mHandler = handler;

  if ( QgsGdalUtils::vsiHandlerType( mHandler ) != Qgis::VsiHandlerType::Cloud )
  {
    mModel->setAvailableOptions( {} );
    return;
  }

  const QString vsiPrefix = QStringLiteral( "/%1/" ).arg( mHandler );
  const char *pszVsiOptions( VSIGetFileSystemOptions( vsiPrefix.toLocal8Bit().constData() ) );
  if ( !pszVsiOptions )
    return;

  CPLXMLNode *psDoc = CPLParseXMLString( pszVsiOptions );
  if ( !psDoc )
    return;
  CPLXMLNode *psOptionList = CPLGetXMLNode( psDoc, "=Options" );
  if ( !psOptionList )
  {
    CPLDestroyXMLNode( psDoc );
    return;
  }

  const QList<QgsGdalOption> options = QgsGdalOption::optionsFromXml( psOptionList );
  CPLDestroyXMLNode( psDoc );

  int maxKeyLength = 0;
  for ( const QgsGdalOption &option : options )
  {
    if ( option.name.length() > maxKeyLength )
      maxKeyLength = option.name.length();
  }

  mTableView->setColumnWidth( QgsGdalCredentialOptionsModel::Column::Key, static_cast<int>( QFontMetrics( mTableView->font() ).horizontalAdvance( 'X' ) * maxKeyLength * 1.1 ) );

  mModel->setAvailableOptions( options );
}

QVariantMap QgsGdalCredentialOptionsWidget::credentialOptions() const
{
  QVariantMap result;
  const QList<QPair<QString, QString>> options = mModel->credentialOptions();
  for ( const QPair<QString, QString> &option : options )
  {
    if ( option.first.isEmpty() )
      continue;

    result[option.first] = option.second;
  }

  return result;
}

void QgsGdalCredentialOptionsWidget::setCredentialOptions( const QVariantMap &options )
{
  QList<QPair<QString, QString>> modelOptions;
  for ( auto it = options.constBegin(); it != options.constEnd(); ++it )
  {
    modelOptions.append( qMakePair( it.key(), it.value().toString() ) );
  }
  mModel->setCredentialOptions( modelOptions );
}

void QgsGdalCredentialOptionsWidget::modelOptionsChanged()
{
  bool needsSensitiveWarning = false;
  const QVariantMap options = credentialOptions();
  for ( const auto &key :
        { "AWS_ACCESS_KEY_ID",
          "AWS_SECRET_ACCESS_KEY",
          "GS_SECRET_ACCESS_KEY",
          "GS_ACCESS_KEY_ID",
          "GS_OAUTH2_PRIVATE_KEY",
          "GS_OAUTH2_CLIENT_SECRET",
          "AZURE_STORAGE_CONNECTION_STRING",
          "AZURE_STORAGE_ACCESS_TOKEN",
          "AZURE_STORAGE_ACCESS_KEY",
          "AZURE_STORAGE_SAS_TOKEN",
          "OSS_SECRET_ACCESS_KEY",
          "OSS_ACCESS_KEY_ID",
          "SWIFT_AUTH_TOKEN",
          "SWIFT_KEY"
        } )
  {
    if ( !options.value( key ).toString().isEmpty() )
    {
      needsSensitiveWarning = true;
      break;
    }
  }
  mLabelWarning->setVisible( needsSensitiveWarning );

  emit optionsChanged();
}
