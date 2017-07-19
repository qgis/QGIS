#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>

class QLabel;

class SidePanel : public QWidget
{
  public:
    explicit SidePanel( QWidget *parent = nullptr );

    void setFps( float fps );

  private:
    QLabel *labelFps;
};

#endif // SIDEPANEL_H
