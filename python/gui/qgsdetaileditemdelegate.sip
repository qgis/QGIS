/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdetaileditemdelegate.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDetailedItemDelegate : QAbstractItemDelegate
{
%Docstring
 A custom model/view delegate that can display an icon, heading
 and detail sections.
.. seealso:: :py:func:`also`
%End

%TypeHeaderCode
#include "qgsdetaileditemdelegate.h"
%End
  public:
    QgsDetailedItemDelegate( QObject *parent /TransferThis/ = 0 );
    ~QgsDetailedItemDelegate();
     virtual void paint( QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index ) const;
%Docstring
Reimplement for parent class
%End
     virtual QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
%Docstring
Reimplement for parent class
 :rtype: QSize
%End

    void setVerticalSpacing( int value );

    int verticalSpacing() const;
%Docstring
 :rtype: int
%End

    void setHorizontalSpacing( int value );

    int horizontalSpacing() const;
%Docstring
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdetaileditemdelegate.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
