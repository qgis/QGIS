/***************************************************************************
     testqgscogointersectioncircles.cpp
     -------------------------
    Date                 : November 2021
    Copyright            : (C) 2021 by Antoine Facchini
    Email                : antoine dot facchini at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

#include "cogo/qgsintersection2circles.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the intresection 2 circles dialog
 */
class TestQgsIntersection2CirclesDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsIntersection2CirclesDialog();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void selectCircleWithFields();
    void selectCircleByHands();
    void createPointWhenTwoIntersections();
    void createPointWhenOneIntersection();

    void checkNoIntersection( QgsIntersection2CirclesDialog *dialog );
    void checkOneIntersection( QgsIntersection2CirclesDialog *dialog );
    void checkTwoIntersections( QgsIntersection2CirclesDialog *dialog );

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsIntersection2CirclesDialog::TestQgsIntersection2CirclesDialog() = default;

//runs before all tests
void TestQgsIntersection2CirclesDialog::initTestCase()
{
  qDebug() << "TestQgsIntersection2CirclesDialog::initTestCase()";
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );

  mLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:3946" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer->isValid() );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayer );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer );
  mCanvas->setCurrentLayer( mLayer );
}

//runs after all tests
void TestQgsIntersection2CirclesDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsIntersection2CirclesDialog::checkNoIntersection( QgsIntersection2CirclesDialog *dialog )
{
  QVERIFY( !dialog->mBtnIntersection1->isEnabled() );
  QVERIFY( !dialog->mBtnIntersection2->isEnabled() );
  QVERIFY( !dialog->mRubberInter1->isVisible() );
  QVERIFY( !dialog->mRubberInter2->isVisible() );
}

void TestQgsIntersection2CirclesDialog::checkOneIntersection( QgsIntersection2CirclesDialog *dialog )
{
  QVERIFY( dialog->mBtnIntersection1->isEnabled() );
  QVERIFY( !dialog->mBtnIntersection2->isEnabled() );
  QVERIFY( dialog->mRubberInter1->isVisible() );
  QVERIFY( !dialog->mRubberInter2->isVisible() );
}

void TestQgsIntersection2CirclesDialog::checkTwoIntersections( QgsIntersection2CirclesDialog *dialog )
{
  QVERIFY( dialog->mBtnIntersection1->isEnabled() );
  QVERIFY( dialog->mBtnIntersection2->isEnabled() );
  QVERIFY( dialog->mRubberInter1->isVisible() );
  QVERIFY( dialog->mRubberInter2->isVisible() );
}

void TestQgsIntersection2CirclesDialog::selectCircleWithFields()
{
  std::unique_ptr< QgsIntersection2CirclesDialog > dialog( new QgsIntersection2CirclesDialog( mCanvas ) );

  checkNoIntersection( dialog.get() );
  QVERIFY( !dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );

  // when there is 1 intersection
  dialog->mX1->setValue( 0.0 );
  dialog->mY1->setValue( 1.0 );
  dialog->mRadius1->setValue( 1.0 );

  dialog->mX2->setValue( 0.0 );
  dialog->mY2->setValue( -1.0 );
  dialog->mRadius2->setValue( 1.0 );

  checkOneIntersection( dialog.get() );
  QVERIFY( dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );

  // when there are 2 intersections
  dialog->mX2->setValue( 0.0 );
  dialog->mY2->setValue( 0.0 );
  dialog->mRadius2->setValue( 1.0 );

  checkTwoIntersections( dialog.get() );

  // select an intersection
  dialog->mBtnIntersection1->click();

  QVERIFY( dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );

  QVERIFY( dialog->mBtnIntersection1->isChecked() );
  QVERIFY( !dialog->mBtnIntersection2->isChecked() );
  QCOMPARE( dialog->mRubberInter1->strokeColor(), dialog->mSelectedColor );
  QCOMPARE( dialog->mRubberInter2->strokeColor(), dialog->mDefaultColor );

  dialog->mBtnIntersection2->click();

  QVERIFY( !dialog->mBtnIntersection1->isChecked() );
  QVERIFY( dialog->mBtnIntersection2->isChecked() );
  QCOMPARE( dialog->mRubberInter1->strokeColor(), dialog->mDefaultColor );
  QCOMPARE( dialog->mRubberInter2->strokeColor(), dialog->mSelectedColor );

  // then 1 intersection, selection should switch on mBtnIntersection1
  dialog->mX2->setValue( 0.0 );
  dialog->mY2->setValue( -1.0 );
  dialog->mRadius2->setValue( 1.0 );

  QVERIFY( dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );

  checkOneIntersection( dialog.get() );
  QVERIFY( dialog->mBtnIntersection1->isChecked() );
  QVERIFY( !dialog->mBtnIntersection2->isChecked() );
  QCOMPARE( dialog->mRubberInter1->strokeColor(), dialog->mSelectedColor );

  // finally no intersection
  dialog->mX2->setValue( 10.0 );
  dialog->mY2->setValue( -1.0 );

  checkNoIntersection( dialog.get() );
  QVERIFY( !dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );
}

void TestQgsIntersection2CirclesDialog::selectCircleByHands()
{
  std::unique_ptr< QgsIntersection2CirclesDialog > dialog( new QgsIntersection2CirclesDialog( mCanvas ) );

  dialog->mRadius2->setValue( 1.0 );
  dialog->mRadius1->setValue( 1.0 );

  // select 1st center
  dialog->mSelectCenter1->click();
  QVERIFY( dialog->mMapToolPoint == mCanvas->mapTool() );

  TestQgsMapToolUtils utils( dialog->mMapToolPoint );
  utils.mouseClick( 5, 5, Qt::LeftButton );

  checkNoIntersection( dialog.get() );

  // select 2nd center
  dialog->mSelectCenter2->click();
  utils.mouseClick( 4, 4, Qt::LeftButton );

  checkTwoIntersections( dialog.get() );

  // exit hand center selection
  utils.mouseClick( 4, 4, Qt::RightButton );

  QVERIFY( dialog->mMapToolPoint != mCanvas->mapTool() );

  // supperposed circles should not have an intersection
  dialog->mSelectCenter2->click();
  utils.mouseClick( 4, 4, Qt::LeftButton );

  checkTwoIntersections( dialog.get() );
  QVERIFY( dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );

  dialog->mSelectCenter2->click();
  utils.mouseClick( 5, 5, Qt::LeftButton );

  checkNoIntersection( dialog.get() );
  QVERIFY( !dialog->mButtonBox->button( QDialogButtonBox::Ok )->isEnabled() );
}

void TestQgsIntersection2CirclesDialog::createPointWhenTwoIntersections()
{
  mLayer->startEditing();
  QCOMPARE( mLayer->featureCount(), 0 );

  std::unique_ptr< QgsIntersection2CirclesDialog > dialog( new QgsIntersection2CirclesDialog( mCanvas ) );

  dialog->mX1->setValue( 0.0 );
  dialog->mY1->setValue( 1.0 );
  dialog->mRadius1->setValue( 1.0 );

  dialog->mX2->setValue( 0.0 );
  dialog->mY2->setValue( 0.0 );
  dialog->mRadius2->setValue( 1.0 );

  checkTwoIntersections( dialog.get() );

  dialog->mBtnIntersection2->click();
  dialog->mButtonBox->button( QDialogButtonBox::Ok )->click();

  QCOMPARE( mLayer->featureCount(), 1 );

  QgsFeature feature;
  mLayer->getFeatures().nextFeature( feature );
  const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( feature.geometry().constGet() );

  QGSCOMPARENEAR( point->x(), 0.866, 0.001 );
  QCOMPARE( point->y(), 0.5 );

  mLayer->rollBack();
}

void TestQgsIntersection2CirclesDialog::createPointWhenOneIntersection()
{
  mLayer->startEditing();
  QCOMPARE( mLayer->featureCount(), 0 );

  std::unique_ptr< QgsIntersection2CirclesDialog > dialog( new QgsIntersection2CirclesDialog( mCanvas ) );

  dialog->mX1->setValue( 0.0 );
  dialog->mY1->setValue( 1.0 );
  dialog->mRadius1->setValue( 1.0 );

  dialog->mX2->setValue( 0.0 );
  dialog->mY2->setValue( -1.0 );
  dialog->mRadius2->setValue( 1.0 );

  checkOneIntersection( dialog.get() );

  dialog->mButtonBox->button( QDialogButtonBox::Ok )->click();

  QCOMPARE( mLayer->featureCount(), 1 );

  QgsFeature feature;
  mLayer->getFeatures().nextFeature( feature );
  const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( feature.geometry().constGet() );
  QCOMPARE( *point, QgsPoint( 0.0, 0.0 ) );

  QVERIFY( !dialog->mRubberCircle1->isVisible() );
  QVERIFY( !dialog->mRubberCircle2->isVisible() );
  QVERIFY( !dialog->mRubberInter1->isVisible() );
  QVERIFY( !dialog->mRubberInter2->isVisible() );

  QVERIFY( dialog->mIntersection1.isEmpty() );
  QVERIFY( dialog->mIntersection2.isEmpty() );

  QVERIFY( dialog->mMapToolPoint != mCanvas->mapTool() );

  QCOMPARE( dialog->mX1->value(), 0.0 );
  QCOMPARE( dialog->mY1->value(), 0.0 );
  QCOMPARE( dialog->mX2->value(), 0.0 );
  QCOMPARE( dialog->mY2->value(), 0.0 );
  QCOMPARE( dialog->mRadius1->value(), 0.0 );
  QCOMPARE( dialog->mRadius2->value(), 0.0 );

  mLayer->rollBack();
}

QGSTEST_MAIN( TestQgsIntersection2CirclesDialog )
#include "testqgscogointersectioncircles.moc"
