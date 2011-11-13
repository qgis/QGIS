#ifndef QGSBROWSERDOCKWIDGET_H
#define QGSBROWSERDOCKWIDGET_H

#include <QDockWidget>

class QgsBrowserModel;
class QModelIndex;
class QTreeView;
class QToolButton;

class QgsBrowserDockWidget : public QDockWidget
{
    Q_OBJECT
  public:
    explicit QgsBrowserDockWidget( QWidget *parent = 0 );

  signals:

  public slots:
    void itemClicked( const QModelIndex& index );
    void showContextMenu( const QPoint & );

    void addFavourite();
    void removeFavourite();

    void refresh();

  protected:

    void refreshModel( const QModelIndex& index );

    void showEvent( QShowEvent * event );

    QTreeView* mBrowserView;
    QToolButton* mRefreshButton;
    QgsBrowserModel* mModel;
};

#endif // QGSBROWSERDOCKWIDGET_H
