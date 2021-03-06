/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 *
 * @author Adeel Asghar <adeel.asghar@liu.se>
 *
 * RCS: $Id$
 *
 */

#include "BitmapAnnotation.h"

BitmapAnnotation::BitmapAnnotation(QString classFileName, QString annotation, Component *pParent)
  : ShapeAnnotation(pParent), mpComponent(pParent)
{
  mClassFileName = classFileName;
  // set the default values
  GraphicItem::setDefaults();
  ShapeAnnotation::setDefaults();
  parseShapeAnnotation(annotation);
  setPos(mOrigin);
  setRotation(mRotation);
}

BitmapAnnotation::BitmapAnnotation(QString classFileName, QString annotation, bool inheritedShape, GraphicsView *pGraphicsView)
  : ShapeAnnotation(inheritedShape, pGraphicsView, 0)
{
  mpComponent = 0;
  mClassFileName = classFileName;
  // set the default values
  GraphicItem::setDefaults();
  ShapeAnnotation::setDefaults();
  // set users default value by reading the settings file.
  ShapeAnnotation::setUserDefaults();
  parseShapeAnnotation(annotation);
  setShapeFlags(true);
  mpGraphicsView->addShapeObject(this);
  mpGraphicsView->scene()->addItem(this);
  connect(this, SIGNAL(updateClassAnnotation()), mpGraphicsView, SLOT(addClassAnnotation()));
}

void BitmapAnnotation::parseShapeAnnotation(QString annotation)
{
  GraphicItem::parseShapeAnnotation(annotation);
  // parse the shape to get the list of attributes of Bitmap.
  QStringList list = StringHandler::getStrings(annotation);
  if (list.size() < 5)
    return;
  // 4th item is the extent points
  QStringList extentsList = StringHandler::getStrings(StringHandler::removeFirstLastCurlBrackets(list.at(3)));
  for (int i = 0 ; i < qMin(extentsList.size(), 2) ; i++)
  {
    QStringList extentPoints = StringHandler::getStrings(StringHandler::removeFirstLastCurlBrackets(extentsList[i]));
    if (extentPoints.size() >= 2)
      mExtents.replace(i, QPointF(extentPoints.at(0).toFloat(), extentPoints.at(1).toFloat()));
  }
  // 5th item is the fileName
  setFileName(StringHandler::removeFirstLastQuotes(list.at(4)), mpComponent);
  // 6th item is the imageSource
  if (list.size() >= 6)
    mImageSource = StringHandler::removeFirstLastQuotes(list.at(5));
  if (!mImageSource.isEmpty())
  {
    mImage.loadFromData(QByteArray::fromBase64(mImageSource.toLatin1()));
  }
  else
  {
    mImage.load(mFileName);
  }
}

QRectF BitmapAnnotation::boundingRect() const
{
  return shape().boundingRect();
}

QPainterPath BitmapAnnotation::shape() const
{
  QPainterPath path;
  path.addRect(getBoundingRect());
  return path;
}

void BitmapAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(option);
  Q_UNUSED(widget);
  if (mVisible)
    drawBitmapAnnotaion(painter);
}

void BitmapAnnotation::drawBitmapAnnotaion(QPainter *painter)
{
  painter->drawImage(getBoundingRect(), mImage.mirrored());
}

QString BitmapAnnotation::getShapeAnnotation()
{
  QStringList annotationString;
  annotationString.append(GraphicItem::getShapeAnnotation());
  // get the extents
  if (mExtents.size() > 1)
  {
    QString extentString;
    extentString.append("extent={");
    extentString.append("{").append(QString::number(mExtents.at(0).x())).append(",");
    extentString.append(QString::number(mExtents.at(0).y())).append("},");
    extentString.append("{").append(QString::number(mExtents.at(1).x())).append(",");
    extentString.append(QString::number(mExtents.at(1).y())).append("}");
    extentString.append("}");
    annotationString.append(extentString);
  }
  // get the file name
  if (!mOriginalFileName.isEmpty())
    annotationString.append(QString("fileName=\"").append(mOriginalFileName).append("\""));
  // get the image source
  if (!mImageSource.isEmpty())
    annotationString.append(QString("imageSource=\"").append(mImageSource).append("\""));
  return QString("Bitmap(").append(annotationString.join(",")).append(")");
}

void BitmapAnnotation::duplicate()
{
  BitmapAnnotation *pBitmapAnnotation = new BitmapAnnotation(mClassFileName, "", false, mpGraphicsView);
  QPointF gridStep(mpGraphicsView->getCoOrdinateSystem()->getHorizontalGridStep(),
                   mpGraphicsView->getCoOrdinateSystem()->getVerticalGridStep());
  pBitmapAnnotation->setOrigin(mOrigin + gridStep);
  pBitmapAnnotation->setRotationAngle(mRotation);
  pBitmapAnnotation->initializeTransformation();
  pBitmapAnnotation->setExtents(getExtents());
  pBitmapAnnotation->setFileName(getFileName());
  pBitmapAnnotation->setImageSource(getImageSource());
  pBitmapAnnotation->setImage(getImage());
  pBitmapAnnotation->drawCornerItems();
  pBitmapAnnotation->setCornerItemsPassive();
  pBitmapAnnotation->update();
  mpGraphicsView->addClassAnnotation();
  mpGraphicsView->setCanAddClassAnnotation(true);
}
