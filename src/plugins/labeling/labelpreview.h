#ifndef LABELPREVIEW_H
#define LABELPREVIEW_H

#include <QLabel>

class LabelPreview : public QLabel
{
  public:
    LabelPreview( QWidget* parent = NULL );

    void setTextColor( QColor color );

    void setBuffer( double size, QColor color );

    void paintEvent( QPaintEvent* e );

  private:
    int mBufferSize;
    QColor mBufferColor;
    QColor mTextColor;
};

#endif // LABELPREVIEW_H
