
#ifndef NavigationGUI_H
#define NavigationGUI_H

#include <QDialog>
#include <ui_navigationgui.h>

#include <QAbstractSocket>

class Navigation;

class NavigationGui : public QDialog, private Ui::NavigationGui
{
  Q_OBJECT

  public:
    NavigationGui(Navigation* plugin);
    ~NavigationGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    
  private:
    Navigation* mPlugin;
    
  public slots:
    void gpsStateChanged(QAbstractSocket::SocketState);
    void socketError(QAbstractSocket::SocketError);
    
  private slots:
    void on_pbnStart_clicked();
    void on_pbnStop_clicked();

};

#endif
