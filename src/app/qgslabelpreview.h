#ifndef QGSLABELPREVIEW_H
#define QGSLABELPREVIEW_H

#include <QLabel>

class QgsLabelPreview : public QLabel
{
  public:
    QgsLabelPreview( QWidget* parent = NULL );

    void setTextColor( QColor color );

    void setBuffer( double size, QColor color );

    void setFont( QFont f ) { mFont = f; }
    QFont font() { return mFont; }

    void paintEvent( QPaintEvent* e );

  private:
    int mBufferSize;
    QColor mBufferColor;
    QColor mTextColor;
    QFont mFont;
};

#endif // LABELPREVIEW_H
