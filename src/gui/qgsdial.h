#include <QDial>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>

class GUI_EXPORT QgsDial : public QDial
{
    Q_OBJECT
  public:
    QgsDial( QWidget *parent = 0 );
  protected:
    virtual void paintEvent( QPaintEvent * event );
};
