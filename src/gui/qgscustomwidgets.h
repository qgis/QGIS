#include <QDial>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>
#include <QSlider>

class EQDial : public QDial
{
  Q_OBJECT
public:
  EQDial( QWidget *parent = 0 );
protected:
  virtual void paintEvent( QPaintEvent * event );
};

class EQSlider : public QSlider
{
  Q_OBJECT
public:
  EQSlider( QWidget *parent = 0 );
  EQSlider( Qt::Orientation orientation, QWidget * parent = 0 );
protected:
  virtual void paintEvent( QPaintEvent * event );
};
