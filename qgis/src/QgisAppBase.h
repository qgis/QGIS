/****************************************************************************
** Form interface generated from reading ui file 'QgisAppBase.ui'
**
** Created: Fri Jul 5 08:47:44 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef QGISAPPBASE_H
#define QGISAPPBASE_H

#include <qvariant.h>
#include <qmainwindow.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QFrame;

class QgisAppBase : public QMainWindow
{ 
    Q_OBJECT

public:
    QgisAppBase( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~QgisAppBase();

    QFrame* frameMain;
    QMenuBar *menubar;
    QPopupMenu *PopupMenu;
    QPopupMenu *PopupMenu_2;
    QToolBar *Toolbar;
    QToolBar *mapNavigationToolbar;
    QToolBar *Toolbar_2;
    QAction* actionFileOpen;
    QAction* actionFileExit;
    QAction* actionZoomIn;
    QAction* actionPan;
    QAction* actionZoomOut;
    QAction* actionAddLayer;


public slots:
    virtual void fileExit();
    virtual void fileOpen();
    virtual void addLayer();
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void init();

protected:
    QGridLayout* QgisAppBaseLayout;
};

#endif // QGISAPPBASE_H
