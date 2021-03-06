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

#ifndef SHAPEANNOTATION_H
#define SHAPEANNOTATION_H

#include <QGraphicsItem>
#include <QSettings>
#include <QGroupBox>
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "Helper.h"
#include "StringHandler.h"
#include "Transformation.h"

class MainWindow;
class GraphicsView;
class CornerItem;
class ResizerItem;

class GraphicItem
{
public:
  GraphicItem() {}
  void setDefaults();
  void parseShapeAnnotation(QString annotation);
  QStringList getShapeAnnotation();
  void setOrigin(QPointF origin);
  QPointF getOrigin();
  void setRotationAngle(qreal rotation);
  qreal getRotation();
protected:
  bool mVisible;
  QPointF mOrigin;
  qreal mRotation;
};

class FilledShape
{
public:
  FilledShape() {}
  void setDefaults();
  void parseShapeAnnotation(QString annotation);
  QStringList getShapeAnnotation();
  void setLineColor(QColor color);
  QColor getLineColor();
  void setFillColor(QColor color);
  QColor getFillColor();
  void setLinePattern(StringHandler::LinePattern pattern);
  StringHandler::LinePattern getLinePattern();
  void setFillPattern(StringHandler::FillPattern pattern);
  StringHandler::FillPattern getFillPattern();
  void setLineThickness(qreal thickness);
  qreal getLineThickness();
protected:
  QColor mLineColor;
  QColor mFillColor;
  StringHandler::LinePattern mLinePattern;
  StringHandler::FillPattern mFillPattern;
  qreal mLineThickness;
};

class ShapeAnnotation : public QObject, public QGraphicsItem, public GraphicItem, public FilledShape
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
private:
  bool mIsCustomShape;
  bool mIsInheritedShape;
  QPointF mOldPosition;
  bool mIsCornerItemClicked;
  QAction *mpShapePropertiesAction;
  QAction *mpManhattanizeShapeAction;
public:
  enum LineGeometryType {VerticalLine, HorizontalLine};
  ShapeAnnotation(QGraphicsItem *pParent);
  ShapeAnnotation(bool inheritedShape, GraphicsView *pGraphicsView, QGraphicsItem *pParent = 0);
  ~ShapeAnnotation();
  void setDefaults();
  void setUserDefaults();
  bool isInheritedShape();
  void createActions();
  QPainterPath addPathStroker(QPainterPath &path) const;
  QRectF getBoundingRect() const;
  void applyLinePattern(QPainter *painter);
  void applyFillPattern(QPainter *painter);
  virtual QString getShapeAnnotation();
  void initializeTransformation();
  void drawCornerItems();
  void setCornerItemsActive();
  void setCornerItemsPassive();
  void removeCornerItems();
  void setOldPosition(QPointF oldPosition);
  QPointF getOldPosition();
  virtual void addPoint(QPointF point);
  virtual void clearPoints();
  virtual void replaceExtent(int index, QPointF point);
  virtual void updateEndExtent(QPointF point);
  GraphicsView* getGraphicsView() {return mpGraphicsView;}
  Transformation* getTransformation() {return mpTransformation;}
  void setPoints(QList<QPointF> points) {mPoints = points;}
  QList<QPointF> getPoints() {return mPoints;}
  void setStartArrow(StringHandler::Arrow startArrow);
  StringHandler::Arrow getStartArrow();
  void setEndArrow(StringHandler::Arrow endArrow);
  StringHandler::Arrow getEndArrow();
  void setArrowSize(qreal arrowSize);
  qreal getArrowSize();
  void setSmooth(StringHandler::Smooth smooth);
  StringHandler::Smooth getSmooth();
  void setExtents(QList<QPointF> extents);
  QList<QPointF> getExtents();
  void setBorderPattern(StringHandler::BorderPattern pattern);
  StringHandler::BorderPattern getBorderPattern();
  void setRadius(qreal radius);
  qreal getRadius();
  void setStartAngle(qreal startAngle);
  qreal getStartAngle();
  void setEndAngle(qreal endAngle);
  qreal getEndAngle();
  void setTextString(QString textString);
  QString getTextString();
  void setFontName(QString fontName);
  QString getFontName();
  void setFontSize(qreal fontSize);
  qreal getFontSize();
  void setTextStyles(QList<StringHandler::TextStyle> textStyles);
  QList<StringHandler::TextStyle> getTextStyles();
  void setTextHorizontalAlignment(StringHandler::TextAlignment textAlignment);
  StringHandler::TextAlignment getTextHorizontalAlignment();
  void setFileName(QString fileName, Component *pComponent = 0);
  QString getFileName();
  void setImageSource(QString imageSource);
  QString getImageSource();
  void setImage(QImage image);
  QImage getImage();
  void rotateClockwise();
  void rotateAntiClockwise();
  void applyRotation(qreal angle);
  void adjustPointsWithOrigin();
  void adjustExtentsWithOrigin();
  CornerItem* getCornerItem(int index);
  void updateCornerItem(int index);
  void insertPointsGeometriesAndCornerItems(int index);
  void adjustCornerItemsConnectedIndexes();
  void removeRedundantPointsGeometriesAndCornerItems();
  void adjustGeometries();
  virtual void setShapeFlags(bool enable);
signals:
  void updateClassAnnotation();
public slots:
  void deleteConnection();
  void deleteMe();
  virtual void duplicate();
  void bringToFront();
  void bringForward();
  void sendToBack();
  void sendBackward();
  void rotateClockwiseKeyPress();
  void rotateAntiClockwiseKeyPress();
  void rotateClockwiseMouseRightClick();
  void rotateAntiClockwiseMouseRightClick();
  void moveUp();
  void moveShiftUp();
  void moveCtrlUp();
  void moveDown();
  void moveShiftDown();
  void moveCtrlDown();
  void moveLeft();
  void moveShiftLeft();
  void moveCtrlLeft();
  void moveRight();
  void moveShiftRight();
  void moveCtrlRight();
  void cornerItemPressed();
  void cornerItemReleased();
  void updateCornerItemPoint(int index, QPointF point);
  LineGeometryType findLineGeometryType(QPointF point1, QPointF point2);
  bool isLineStraight(QPointF point1, QPointF point2);
  void showShapeProperties();
  void manhattanizeShape();
protected:
  GraphicsView *mpGraphicsView;
  Transformation *mpTransformation;
  QList<QPointF> mPoints;
  QList<LineGeometryType> mGeometries;
  QList<StringHandler::Arrow> mArrow;
  qreal mArrowSize;
  StringHandler::Smooth mSmooth;
  QList<QPointF> mExtents;
  StringHandler::BorderPattern mBorderPattern;
  qreal mRadius;
  qreal mStartAngle;
  qreal mEndAngle;
  QString mOriginalTextString;
  QString mTextString;
  qreal mFontSize;
  QString mFontName;
  QList<StringHandler::TextStyle> mTextStyles;
  StringHandler::TextAlignment mHorizontalAlignment;
  QString mOriginalFileName;
  QString mFileName;
  QString mClassFileName; /* Used to find the bitmap relative locations. */
  QString mImageSource;
  QImage mImage;
  QList<CornerItem*> mCornerItemsList;
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *pEvent);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // SHAPEANNOTATION_H
