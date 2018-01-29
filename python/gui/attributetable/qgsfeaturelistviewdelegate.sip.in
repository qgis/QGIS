/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistviewdelegate.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsFeatureListViewDelegate : QItemDelegate
{

%TypeHeaderCode
#include "qgsfeaturelistviewdelegate.h"
%End
  public:
    static const int ICON_SIZE;

    enum Element
    {
      EditElement,
      SelectionElement
    };

    explicit QgsFeatureListViewDelegate( QgsFeatureListModel *listModel, QObject *parent = 0 );

    void setEditSelectionModel( QItemSelectionModel *editSelectionModel );

    Element positionToElement( QPoint pos );

    void setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel );

    void setCurrentFeatureEdited( bool state );

  signals:
    void editButtonClicked( QModelIndex &index );

  protected:
    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistviewdelegate.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
