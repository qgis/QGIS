class QgsFeatureListViewDelegate : QItemDelegate
{
%TypeHeaderCode
#include <qgsfeaturelistviewdelegate.h>
%End
  public:
    enum Element
    {
      EditElement,
      SelectionElement
    };

    explicit QgsFeatureListViewDelegate( QgsFeatureListModel* listModel, QObject *parent = 0 );

    void setEditSelectionModel( QItemSelectionModel* editSelectionModel );

    Element positionToElement( const QPoint& pos );

    void setFeatureSelectionModel( QgsFeatureSelectionModel* featureSelectionModel );

    void setCurrentFeatureEdited( bool state );

  signals:
    void editButtonClicked( QModelIndex& index );

  protected:
    virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
};
