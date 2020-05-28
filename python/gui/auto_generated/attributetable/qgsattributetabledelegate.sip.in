/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsattributetabledelegate.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAttributeTableDelegate : QItemDelegate
{
%Docstring
A delegate item class for QgsAttributeTable (see Qt documentation for
QItemDelegate).
%End

%TypeHeaderCode
#include "qgsattributetabledelegate.h"
%End
  public:

    QgsAttributeTableDelegate( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor

:param parent: parent object
%End

    virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

%Docstring
Used to create an editor for when the user tries to
change the contents of a cell
%End

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

%Docstring
Overloads the paint method form the QItemDelegate base class
%End

    virtual void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

%Docstring
Sets data from editor back to model. Overloads default method

:param editor: editor which was created by create editor function in this class
:param model: model where data should be updated
:param index: index of field which is to be modified
%End

    virtual void setEditorData( QWidget *editor, const QModelIndex &index ) const;

%Docstring
Sets data from model into the editor. Overloads default method

:param editor: editor which was created by create editor function in this class
:param index: index of field which is to be retrieved
%End

    void setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel );

  signals:

    void actionColumnItemPainted( const QModelIndex &index ) const;
%Docstring
Emitted when an action column item is painted.
The consumer of this signal can initialize the index widget.

.. note::

   This signal is emitted repeatedly whenever the item is being painted.
   It is the consumers responsibility to check if initialization has already
   happened before.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsattributetabledelegate.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
