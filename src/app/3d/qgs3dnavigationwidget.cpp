#include <QGridLayout>
#include <QToolButton>
#include <QDial>
#include <QObject>
#include <QDebug>

#include "qgsapplication.h"

#include "qgs3dnavigationwidget.h"
#include "qgscameracontroller.h"

Qgs3DNavigationWidget::Qgs3DNavigationWidget(Qgs3DMapCanvas *parent) : QWidget(parent)
{
    // Zoom in button
    mZoomInButton = new QToolButton(this);
    mZoomInButton->setToolTip(QStringLiteral("Zoom In"));
    mZoomInButton->setAutoRepeat(true);
    mZoomInButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
    mZoomInButton->setAutoRaise(true);

    QObject::connect(
        mZoomInButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->zoom(5);
    }
    );

    // Zoom out button
    mZoomOutButton = new QToolButton(this);
    mZoomOutButton->setToolTip(QStringLiteral("Zoom Out"));
    mZoomOutButton->setAutoRepeat(true);
    mZoomOutButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
    mZoomOutButton->setAutoRaise(true);

    QObject::connect(
        mZoomOutButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->zoom(-5);
    }
    );

    // Tilt up button
    mTiltUpButton = new QToolButton(this);
    mTiltUpButton->setToolTip(QStringLiteral("Tilt Up"));
    mTiltUpButton->setAutoRepeat(true);
    mTiltUpButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowUp.svg" ) ) );
    mTiltUpButton->setAutoRaise(true);

    QObject::connect(
        mTiltUpButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->tiltUpAroundViewCenter(1);
    }
    );

    // Tilt down button
    mTiltDownButton = new QToolButton(this);
    mTiltDownButton->setToolTip(QStringLiteral("Tilt Down"));
    mTiltDownButton->setAutoRepeat(true);
    mTiltDownButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowDown.svg" ) ) );
    mTiltDownButton->setAutoRaise(true);

    QObject::connect(
        mTiltDownButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->tiltUpAroundViewCenter(-1);
    }
    );

    // Rotate scene dial
    mRotateSceneDial = new QDial(this);
    mRotateSceneDial->setToolTip(QStringLiteral("Rotate view"));
    mRotateSceneDial->setWrapping(true);
    mRotateSceneDial->setMaximum(359);
    mRotateSceneDial->setValue(180);

    QObject::connect(
        mRotateSceneDial,
        &QDial::valueChanged,
        parent,
        [ = ]{
            qInfo() << "Dial value: " << mRotateSceneDial->value();
            parent->cameraController()->setCameraHeadingAngle(mRotateSceneDial->value());
    }
    );

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(mTiltUpButton, 0, 0);
    gridLayout->addWidget(mTiltDownButton, 3, 0);
    gridLayout->addWidget(mZoomInButton, 0, 3);
    gridLayout->addWidget(mZoomOutButton, 3, 3);
    gridLayout->addWidget(mRotateSceneDial, 1, 1, 2, 2);
    gridLayout->setAlignment(Qt::AlignTop);

}

Qgs3DNavigationWidget::~Qgs3DNavigationWidget()
{

}
