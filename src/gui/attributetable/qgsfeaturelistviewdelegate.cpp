#include "qgsfeaturelistviewdelegate.h"
#include "qgsvectorlayer.h"
#include "qgsattributetablemodel.h"
#include "qgsfeaturelistmodel.h"
#include "qgsapplication.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsfeatureselectionmodel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QMouseEvent>
#include <QObject>

QgsFeatureListViewDelegate::QgsFeatureListViewDelegate( QgsFeatureListModel *listModel, QObject *parent )
    : QItemDelegate( parent )
    , mFeatureSelectionModel( NULL )
    , mListModel( listModel )
{
}

QgsFeatureListViewDelegate::Element QgsFeatureListViewDelegate::positionToElement( const QPoint &pos )
{
  if ( pos.x() > sIconSize )
  {
    return EditElement;
  }
  else
  {
    return SelectionElement;
  }
}

void QgsFeatureListViewDelegate::setFeatureSelectionModel(QgsFeatureSelectionModel *featureSelectionModel)
{
  mFeatureSelectionModel = featureSelectionModel;
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
  bool isEdited = mEditSelectionModel->isSelected( mListModel->mapToMaster( index ) );

  // Icon layout options
  QStyleOptionViewItem iconOption;

  QRect iconLayoutBounds( option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height() );

  QPixmap icon;

  if ( mFeatureSelectionModel->isSelected ( index ) )
  {
    // Item is selected
    icon = QgsApplication::getThemePixmap( "/mIconSelected.svg" );
  }
  else
  {
    icon = QgsApplication::getThemePixmap( "/mIconDeselected.svg" );
  }

  // Text layout options
  QRect textLayoutBounds( iconLayoutBounds.x() + iconLayoutBounds.width(), option.rect.y(), option.rect.width() - ( iconLayoutBounds.x() + iconLayoutBounds.width() ), option.rect.height() );

  QStyleOptionViewItem textOption;
  textOption.state |= QStyle::State_Enabled;
  if ( isEdited )
  {
    textOption.state |= QStyle::State_Selected;
  }

  if ( featInfo.isNew )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::darkGreen );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::darkGreen );
  }
  else if ( featInfo.isEdited || isEdited )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::red );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::red );
  }

  drawDisplay( painter, textOption, textLayoutBounds, text );
  drawDecoration( painter, iconOption, iconLayoutBounds, icon );
}
