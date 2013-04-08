#include "qgsfeaturelistviewdelegate.h"
#include "qgsvectorlayer.h"
#include "qgsattributetablemodel.h"
#include "qgsfeaturelistmodel.h"
#include "qgsapplication.h"
#include "qgsvectorlayereditbuffer.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QMouseEvent>
#include <QObject>

QgsFeatureListViewDelegate::QgsFeatureListViewDelegate( QgsFeatureListModel *listModel, QObject *parent )
    : QItemDelegate( parent )
    , mListModel( listModel )
{
}

QgsFeatureListViewDelegate::Element QgsFeatureListViewDelegate::positionToElement( const QPoint &pos )
{
  if ( pos.x() < sIconSize )
  {
    return EditButtonElement;
  }
  else
  {
    return TextElement;
  }
}

void QgsFeatureListViewDelegate::setEditSelectionModel( QItemSelectionModel* editSelectionModel )
{
  mEditSelectionModel = editSelectionModel;
}

QSize QgsFeatureListViewDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( index )
  return QSize( option.rect.width(), sIconSize );
}

void QgsFeatureListViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QString text = index.model()->data( index, Qt::EditRole ).toString();
  QgsFeatureListModel::FeatureInfo featInfo = index.model()->data( index, Qt::UserRole ).value<QgsFeatureListModel::FeatureInfo>();

  // Edit button state
  bool checked = mEditSelectionModel->isSelected( mListModel->mapToMaster( index ) );

  QStyleOptionButton pbn1Opts;

  pbn1Opts.iconSize = QSize( sIconSize, sIconSize );

  pbn1Opts.state |= QStyle::State_Enabled;
  if ( checked )
  {
    pbn1Opts.icon = QgsApplication::getThemeIcon( "/mIconEditableEdits.png" );
    pbn1Opts.state |= QStyle::State_On;
  }
  else
  {
    pbn1Opts.icon = QgsApplication::getThemeIcon( "/mIconEditable.png" );
    pbn1Opts.state |= QStyle::State_Off;
  }

  QRect pbn1Rect( option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height() );
  pbn1Opts.rect = pbn1Rect;

  QApplication::style()->drawControl( QStyle::CE_PushButton, &pbn1Opts, painter );

  QRect textLayoutBounds( pbn1Rect.x() + pbn1Rect.width(), option.rect.y(), option.rect.width() - ( pbn1Rect.x() + pbn1Rect.width() ), option.rect.height() );

  QStyleOptionViewItem textOption = option;

  if ( featInfo.isNew )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::darkGreen );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::darkGreen );
  }
  else if ( featInfo.isEdited || checked )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::red );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::red );
  }
  drawDisplay( painter, textOption, textLayoutBounds, text );
}
