#ifndef QGS3DVIEWSMANAGER_H
#define QGS3DVIEWSMANAGER_H

#include "ui_qgs3dviewsmanager.h"

#include <QDialog>
#include <QStringListModel>
#include <QDomElement>

class Qgs3DMapCanvasDockWidget;

class Qgs3DViewsManager : public QDialog, private Ui::Qgs3DViewsManager
{
    Q_OBJECT

  public:
    explicit Qgs3DViewsManager( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    ~Qgs3DViewsManager();

    void reload();

    void set3DMapViewsDom( QMap<QString, QDomElement> &mapViews3DDom );
    void set3DMapViewsWidgets( QMap<QString, Qgs3DMapCanvasDockWidget *> &mapViews3DWidgets );
  private:
    QStringListModel mListModel;
    int mSelectedViewIndex = -1;

    QMap<QString, QDomElement> *m3DMapViewsDom = nullptr;
    QMap<QString, Qgs3DMapCanvasDockWidget *> *m3DMapViewsWidgets = nullptr;

    void reloadListModel();
};

#endif // QGS3DVIEWSMANAGER_H
