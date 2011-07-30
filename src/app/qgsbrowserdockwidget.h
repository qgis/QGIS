#ifndef QGSBROWSERDOCKWIDGET_H
#define QGSBROWSERDOCKWIDGET_H

#include <QDockWidget>

class QgsBrowserModel;
class QModelIndex;
class QTreeView;

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

  protected:

    void showEvent( QShowEvent * event );

    QTreeView* mBrowserView;
    QgsBrowserModel* mModel;
};

#endif // QGSBROWSERDOCKWIDGET_H
