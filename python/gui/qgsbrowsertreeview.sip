/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbrowsertreeview.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBrowserTreeView : QTreeView
{
%Docstring
 The QgsBrowserTreeView class extends QTreeView with save/restore tree state functionality.

.. seealso:: QgsBrowserModel
.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgsbrowsertreeview.h"
%End
  public:
    QgsBrowserTreeView( QWidget *parent /TransferThis/ = 0 );

    virtual void setModel( QAbstractItemModel *model );
    void setBrowserModel( QgsBrowserModel *model );
%Docstring
Set the browser model
%End
    QgsBrowserModel *browserModel();
%Docstring
Return the browser model
 :rtype: QgsBrowserModel
%End
    virtual void showEvent( QShowEvent *e );
    virtual void hideEvent( QHideEvent *e );

    bool hasExpandedDescendant( const QModelIndex &index ) const;
%Docstring
 :rtype: bool
%End

    void setSettingsSection( const QString &section );

  protected slots:
    virtual void rowsInserted( const QModelIndex &parentIndex, int start, int end );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbrowsertreeview.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
