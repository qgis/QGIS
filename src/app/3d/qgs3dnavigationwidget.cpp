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
    mTiltUpButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionTiltUp.svg" ) ) );
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
    mTiltDownButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionTiltDown.svg" ) ) );
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
            parent->cameraController()->setCameraHeadingAngle(mRotateSceneDial->value() - 180);
    }
    );

    // Move up button
    mMoveUpButton = new QToolButton(this);
    mMoveUpButton->setToolTip(QStringLiteral("Move up"));
    mMoveUpButton->setAutoRepeat(true);
    mMoveUpButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowUp.svg" ) ) );
    mMoveUpButton->setAutoRaise(true);

    QObject::connect(
        mMoveUpButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->moveView(0, 1);
    }
    );

    // Move right button
    mMoveRightButton = new QToolButton(this);
    mMoveRightButton->setToolTip(QStringLiteral("Move right"));
    mMoveRightButton->setAutoRepeat(true);
    mMoveRightButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowRight.svg" ) ) );
    mMoveRightButton->setAutoRaise(true);

    QObject::connect(
        mMoveRightButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->moveView(1, 0);
    }
    );

    // Move down button
    mMoveDownButton = new QToolButton(this);
    mMoveDownButton->setToolTip(QStringLiteral("Move down"));
    mMoveDownButton->setAutoRepeat(true);
    mMoveDownButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowDown.svg" ) ) );
    mMoveDownButton->setAutoRaise(true);

    QObject::connect(
        mMoveDownButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->moveView(0, -1);
    }
    );

    // Move left button
    mMoveLeftButton = new QToolButton(this);
    mMoveLeftButton->setToolTip(QStringLiteral("Move left"));
    mMoveLeftButton->setAutoRepeat(true);
    mMoveLeftButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowLeft.svg" ) ) );
    mMoveLeftButton->setAutoRaise(true);

    QObject::connect(
        mMoveLeftButton,
        &QToolButton::clicked,
        parent,
        [ = ]{
            parent->cameraController()->moveView(-1, 0);
    }
    );

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(mTiltUpButton, 0, 0);
    gridLayout->addWidget(mTiltDownButton, 3, 0);
    gridLayout->addWidget(mZoomInButton, 0, 3);
    gridLayout->addWidget(mZoomOutButton, 3, 3);
    gridLayout->addWidget(mRotateSceneDial, 1, 1, 2, 2);
    gridLayout->addWidget(mMoveUpButton, 0, 1, 1, 2, Qt::AlignCenter);
    gridLayout->addWidget(mMoveRightButton, 1, 3, 2, 1, Qt::AlignCenter);
    gridLayout->addWidget(mMoveDownButton, 3, 1, 1, 2, Qt::AlignCenter);
    gridLayout->addWidget(mMoveLeftButton, 1, 0, 2, 1, Qt::AlignCenter);
    gridLayout->setAlignment(Qt::AlignTop);
}

Qgs3DNavigationWidget::~Qgs3DNavigationWidget()
{

}
