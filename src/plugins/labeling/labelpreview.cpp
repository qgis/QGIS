#include "labelpreview.h"

#include <QPainter>

#include "pallabeling.h"

LabelPreview::LabelPreview(QWidget* parent)
 : QLabel(parent)
{
}

void LabelPreview::setTextColor(QColor color)
{
  mTextColor = color;
  update();
}

void LabelPreview::setBuffer(int size, QColor color)
{
  mBufferSize = size;
  mBufferColor = color;
  update();
}

void LabelPreview::paintEvent(QPaintEvent* e)
{
  QPainter p(this);

  p.setRenderHint(QPainter::Antialiasing);
  p.setFont(font());
  p.translate(10, 20); // uhm...

  if (mBufferSize != 0)
    PalLabeling::drawLabelBuffer(&p, text(), font(), mBufferSize, mBufferColor);

  p.setPen(mTextColor);
  p.drawText(0,0, text());
}
