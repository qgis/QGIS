
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
    
  private:
    Navigation* mPlugin;
    static const int context_id = 0;
    
  public slots:
    void gpsStateChanged(QAbstractSocket::SocketState);
    void socketError(QAbstractSocket::SocketError);
    
  private slots:
    void on_pbnStart_clicked();
    void on_pbnStop_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();

};

#endif
