#ifndef QGSATTRIBUTELISTVIEWDELEGATE_H
#define QGSATTRIBUTELISTVIEWDELEGATE_H

#include <QItemDelegate>
#include <QItemSelectionModel>

#include "qgsfeature.h"

class QgsVectorLayer;
class QgsFeatureListModel;
class QgsFeatureSelectionModel;
class QPosition;

class GUI_EXPORT QgsFeatureListViewDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    static int const sIconSize = 24;

    enum Element
    {
      EditElement,
      SelectionElement
    };

    explicit QgsFeatureListViewDelegate( QgsFeatureListModel* listModel, QObject *parent = 0 );

    void setEditSelectionModel( QItemSelectionModel* editSelectionModel );

    Element positionToElement( const QPoint& pos );

    void setFeatureSelectionModel( QgsFeatureSelectionModel* featureSelectionModel );

  signals:
    void editButtonClicked( QModelIndex& index );

  public slots:

  protected:
    virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

  private:
    QgsFeatureSelectionModel* mFeatureSelectionModel;
    QItemSelectionModel* mEditSelectionModel;
    QgsFeatureListModel* mListModel;
};

#endif // QGSATTRIBUTELISTVIEWDELEGATE_H
