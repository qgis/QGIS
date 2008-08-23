/***************************************************************************
                         qgscomposerpicturewidget.cpp
                         ----------------------------
    begin                : August 13, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerpicturewidget.h"
#include "qgscomposerpicture.h"
#include "qgscomposeritemwidget.h"
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

QgsComposerPictureWidget::QgsComposerPictureWidget(QgsComposerPicture* picture): QWidget(), mPicture(picture)
{
  setupUi(this);

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget(this, picture);
  gridLayout->addWidget(itemPropertiesWidget, 4, 0, 1, 1);

  mWidthLineEdit->setValidator(new QDoubleValidator(this));
  mHeightLineEdit->setValidator(new QDoubleValidator(this));

  setGuiElementValues();

  connect(mPicture, SIGNAL(settingsChanged()), this, SLOT(setGuiElementValues()));
}

QgsComposerPictureWidget::~QgsComposerPictureWidget()
{

}

void QgsComposerPictureWidget::on_mPictureBrowseButton_clicked()
{
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if(!lineEditText.isEmpty())
    {
      QFileInfo openDirFileInfo(lineEditText);
      openDir = openDirFileInfo.path();
    }
  

  //show file dialog
  QString filePath = QFileDialog::getOpenFileName(0, tr("Select svg or image file"), openDir);
  if(filePath.isEmpty())
    {
      return;
    }

  //check if file exists
  QFileInfo fileInfo(filePath);
  if(!fileInfo.exists() || !fileInfo.isReadable())
    {
      QMessageBox::critical(0, "Invalid file", "Error, file does not exist or is not readable");
      return;
    }

  mPictureLineEdit->blockSignals(true);
  mPictureLineEdit->setText(filePath);
  mPictureLineEdit->blockSignals(false);

  //pass file path to QgsComposerPicture
  if(mPicture)
    {
      mPicture->setPictureFile(filePath);
      mPicture->update();
    }
}

void QgsComposerPictureWidget::on_mPictureLineEdit_editingFinished()
{
  if(mPicture)
    {
      QString filePath = mPictureLineEdit->text();
      
      //check if file exists
      QFileInfo fileInfo(filePath);
      
      if(!fileInfo.exists() || !fileInfo.isReadable())
	{
	  QMessageBox::critical(0, "Invalid file", "Error, file does not exist or is not readable");
	  return;
	}

      mPicture->setPictureFile(filePath);
      mPicture->update();
    }
}

void QgsComposerPictureWidget::on_mWidthLineEdit_editingFinished()
{
  if(mPicture)
    {
      QRectF pictureRect = mPicture->rect();

      bool conversionOk;
      double newWidth = mWidthLineEdit->text().toDouble(&conversionOk);
      if(conversionOk)
	{
	  QRectF newSceneRect(mPicture->transform().dx(), mPicture->transform().dy(), newWidth, pictureRect.height());
	  mPicture->setSceneRect(newSceneRect);
	}
    } 
}

void QgsComposerPictureWidget::on_mHeightLineEdit_editingFinished()
{
  if(mPicture)
    {
      QRectF pictureRect = mPicture->rect();
      
      bool conversionOk;
      double newHeight = mHeightLineEdit->text().toDouble(&conversionOk);
      if(conversionOk)
	{
	  QRectF newSceneRect(mPicture->transform().dx(), mPicture->transform().dy(), pictureRect.width(), newHeight);
	  mPicture->setSceneRect(newSceneRect);
	}
    } 
}

void QgsComposerPictureWidget::on_mRotationSpinBox_valueChanged(double d)
{
  if(mPicture)
    {
      mPicture->setRotation(d);
      mPicture->update();
    }
}

void QgsComposerPictureWidget::setGuiElementValues()
{
  //set initial gui values
  if(mPicture)
    {
      mWidthLineEdit->blockSignals(true);
      mHeightLineEdit->blockSignals(true);
      mRotationSpinBox->blockSignals(true);
      mPictureLineEdit->blockSignals(true);
  
      mPictureLineEdit->setText(mPicture->pictureFile());
      QRectF pictureRect = mPicture->rect();
      mWidthLineEdit->setText(QString::number(pictureRect.width()));
      mHeightLineEdit->setText(QString::number(pictureRect.height()));
      mRotationSpinBox->setValue(mPicture->rotation());
      
      mWidthLineEdit->blockSignals(false);
      mHeightLineEdit->blockSignals(false);
      mRotationSpinBox->blockSignals(false);
      mPictureLineEdit->blockSignals(false);
    }
}
