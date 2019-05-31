#include <QGridLayout>
#include <QPushButton>
#include <QDial>

#include "qgs3dnavigationwidget.h"

Qgs3DNavigationWidget::Qgs3DNavigationWidget(QWidget *parent) : QWidget(parent)
{
    // Zoom in button
    mZoomInButton = new QPushButton(this);
    mZoomInButton->setText(QStringLiteral("+"));
    mZoomInButton->setToolTip(QStringLiteral("Zoom In"));
    mZoomInButton->setAutoRepeat(true);

    // Zoom out button
    mZoomOutButton = new QPushButton(this);
    mZoomOutButton->setText(QStringLiteral("-"));
    mZoomOutButton->setToolTip(QStringLiteral("Zoom Out"));
    mZoomOutButton->setAutoRepeat(true);

    // Tilt up button
    mTiltUpButton = new QPushButton(this);
    mTiltUpButton->setText(QString::fromUtf8("\u25B3"));
    mTiltUpButton->setToolTip(QStringLiteral("Tilt Up"));
    mTiltUpButton->setAutoRepeat(true);

    // Tilt down button
    mTiltDownButton = new QPushButton(this);
    mTiltDownButton ->setText(QString::fromUtf8("\u25BD"));
    mTiltDownButton->setToolTip(QStringLiteral("Tilt Down"));
    mTiltDownButton->setAutoRepeat(true);

    // Rotate scene dial
    mRotateSceneDial = new QDial(this);
    mRotateSceneDial->setWrapping(true);
    mRotateSceneDial->setMaximum(359);
    mRotateSceneDial->setValue(180);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(mTiltUpButton, 0, 0);
    gridLayout->addWidget(mTiltDownButton, 3, 0);
    gridLayout->addWidget(mZoomInButton, 0, 3);
    gridLayout->addWidget(mZoomOutButton, 3, 3);
    gridLayout->addWidget(mRotateSceneDial, 1, 1, 2, 2);

}

Qgs3DNavigationWidget::~Qgs3DNavigationWidget()
{

}
