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

#ifndef LIBRARYTREEWIDGET_H
#define LIBRARYTREEWIDGET_H

#include "MainWindow.h"
#include "StringHandler.h"

class MainWindow;
class Component;
class OMCProxy;
class LibraryTreeWidget;
class LibraryComponent;
class ModelWidget;

class ItemDelegate : public QItemDelegate
{
  Q_OBJECT
private:
  bool mDrawRichText;
  QPoint mLastTextPos;
  bool mDrawGrid;
  QColor mGridColor;
  QObject *mpParent;
public:
  ItemDelegate(QObject *pParent = 0, bool drawRichText = false, bool drawGrid = false);
  QColor getGridColor() {return mGridColor;}
  void setGridColor(QColor color) {mGridColor = color;}
  QString formatDisplayText(QVariant variant) const;
  void initTextDocument(QTextDocument *pTextDocument, QFont font, int width) const;
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void drawHover(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class SearchClassWidget : public QWidget
{
  Q_OBJECT
private:
  MainWindow *mpMainWindow;
  QLineEdit *mpSearchClassTextBox;
  QPushButton *mpSearchClassButton;
  QCheckBox *mpFindInModelicaTextCheckBox;
  Label *mpNoModelicaClassFoundLabel;
  LibraryTreeWidget *mpLibraryTreeWidget;
public:
  SearchClassWidget(MainWindow *pMainWindow);
  QLineEdit* getSearchClassTextBox();
public slots:
  void searchClasses();
};

class LibraryTreeNode : public QTreeWidgetItem
{
public:
  enum LibraryType {
    InvalidType, /* Used to catch errors */
    Modelica,   /* Used to represent Modelica models. */
    Text,       /* Used to represent text based files. */
    TLM         /* Used to represent TLM files. */
  };
  enum SaveContentsType {
    SaveInOneFile,
    SaveFolderStructure,
    SaveUnspecified
  };
  LibraryTreeNode(LibraryType type, QString text, QString parentName, QString nameStructure,
                  OMCInterface::getClassInformation_res classInformation, QString fileName, bool isSaved, LibraryTreeWidget *pParent);
  LibraryType getLibraryType() {return mLibraryType;}
  void setLibraryType(LibraryType libraryType) {mLibraryType = libraryType;}
  void setSystemLibrary(bool systemLibrary) {mSystemLibrary = systemLibrary;}
  bool isSystemLibrary() {return mSystemLibrary;}
  void setModelWidget(ModelWidget *pModelWidget) {mpModelWidget = pModelWidget;}
  ModelWidget* getModelWidget() {return mpModelWidget;}
  void setName(QString name) {mName = name;}
  const QString& getName() const {return mName;}
  void setParentName(QString parentName) {mParentName = parentName;}
  const QString& getParentName() {return mParentName;}
  void setNameStructure(QString nameStructure) {mNameStructure = nameStructure;}
  const QString& getNameStructure() {return mNameStructure;}
  void setClassInformation(OMCInterface::getClassInformation_res classInformation);
  OMCInterface::getClassInformation_res getClassInformation() {return mClassInformation;}
  void setFileName(QString fileName);
  const QString& getFileName() {return mFileName;}
  void setReadOnly(bool readOnly) {mReadOnly = readOnly;}
  bool isReadOnly() {return mReadOnly;}
  void setIsSaved(bool isSaved) {mIsSaved = isSaved;}
  bool isSaved() {return mIsSaved;}
  void setIsProtected(bool isProtected) {mIsProtected = isProtected;}
  bool isProtected() {return mIsProtected;}
  void setIsDocumentationClass(bool documentationClass) {mDocumentationClass = documentationClass;}
  bool isDocumentationClass() {return mDocumentationClass;}
  void setSaveContentsType(LibraryTreeNode::SaveContentsType saveContentsType) {mSaveContentsType = saveContentsType;}
  StringHandler::ModelicaClasses getRestriction() {return StringHandler::getModelicaClassType(mClassInformation.restriction);}
  bool isPartial() {return mClassInformation.partialPrefix;}
  SaveContentsType getSaveContentsType() {return mSaveContentsType;}
  void updateAttributes();
  QIcon getModelicaNodeIcon();
  bool inRange(int lineNumber) {return (lineNumber >= mClassInformation.lineNumberStart) && (lineNumber <= mClassInformation.lineNumberEnd);}
private:
  LibraryType mLibraryType;
  bool mSystemLibrary;
  ModelWidget *mpModelWidget;
  LibraryTreeWidget *mpLibraryTreeWidget;
  QString mName;
  QString mParentName;
  QString mNameStructure;
  OMCInterface::getClassInformation_res mClassInformation;
  QString mFileName;
  bool mReadOnly;
  bool mIsSaved;
  bool mIsProtected;
  bool mDocumentationClass;
  SaveContentsType mSaveContentsType;
};

class LibraryTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  LibraryTreeWidget(bool isSearchTree, MainWindow *pParent);
  ~LibraryTreeWidget();
  MainWindow* getMainWindow();
  void setIsSearchedTree(bool isSearchTree);
  bool isSearchedTree();
  void addToExpandedLibraryTreeNodesList(LibraryTreeNode *pLibraryTreeNode);
  void removeFromExpandedLibraryTreeNodesList(LibraryTreeNode *pLibraryTreeNode);
  void createActions();
  void addModelicaLibraries(QSplashScreen *pSplashScreen);
  void createLibraryTreeNodes(LibraryTreeNode *pLibraryTreeNode);
  void expandLibraryTreeNode(LibraryTreeNode *pLibraryTreeNode);
  void loadLibraryTreeNode(LibraryTreeNode *pParentLibraryTreeNode, LibraryTreeNode *pLibraryTreeNode);
  void addLibraryTreeNodes(QList<LibraryTreeNode*> libraryTreeNodes);
  bool isLibraryTreeNodeExpanded(QTreeWidgetItem *item);
  static bool sortNodesAscending(const LibraryTreeNode *node1, const LibraryTreeNode *node2);
  LibraryTreeNode* addLibraryTreeNode(QString name, QString parentName = QString(""), bool isSaved = true, int insertIndex = 0);
  LibraryTreeNode* addLibraryTreeNode(LibraryTreeNode::LibraryType type, QString name, bool isSaved, int insertIndex = 0);
  LibraryTreeNode* getLibraryTreeNode(QString nameStructure, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);
  QList<LibraryTreeNode*> getLibraryTreeNodesList();
  void addLibraryComponentObject(LibraryComponent *libraryComponent);
  Component *getComponentObject(QString className);
  LibraryComponent* getLibraryComponentObject(QString className);
  bool isFileWritAble(QString filePath);
  void showProtectedClasses(bool enable);
  bool unloadClass(LibraryTreeNode *pLibraryTreeNode, bool askQuestion = true);
  bool unloadTextFile(LibraryTreeNode *pLibraryTreeNode, bool askQuestion = true);
  bool unloadTLMFile(LibraryTreeNode *pLibraryTreeNode, bool askQuestion = true);
  void unloadClassHelper(LibraryTreeNode *pLibraryTreeNode);
  bool saveLibraryTreeNode(LibraryTreeNode *pLibraryTreeNode);
  LibraryTreeNode* findParentLibraryTreeNodeSavedInSameFile(LibraryTreeNode *pLibraryTreeNode, QFileInfo fileInfo);
  QString getUniqueLibraryTreeNodeName(QString name, int number = 1);
  bool isSimulationAllowed(LibraryTreeNode *pLibraryTreeNode);
  void loadDependentLibraries(QStringList libraries);
  LibraryTreeNode* getLibraryTreeNodeFromFile(QString fileName, int lineNumber);
private:
  MainWindow *mpMainWindow;
  bool mIsSearchTree;
  QList<LibraryTreeNode*> mLibraryTreeNodesList;
  QList<LibraryTreeNode*> mExpandedLibraryTreeNodesList;
  QList<LibraryComponent*> mLibraryComponentsList;
  QAction *mpViewClassAction;
  QAction *mpViewDocumentationAction;
  QAction *mpNewModelicaClassAction;
  QAction *mpInstantiateModelAction;
  QAction *mpCheckModelAction;
  QAction *mpCheckAllModelsAction;
  QAction *mpSimulateAction;
  QAction *mpSimulateWithTransformationalDebuggerAction;
  QAction *mpSimulateWithAlgorithmicDebuggerAction;
  QAction *mpSimulationSetupAction;
  QAction *mpDuplicateClassAction;
  QAction *mpUnloadClassAction;
  QAction *mpUnloadTextFileAction;
  QAction *mpUnloadTLMFileAction;
  QAction *mpRefreshAction;
  QAction *mpExportFMUAction;
  QAction *mpExportXMLAction;
  QAction *mpExportFigaroAction;
  QAction *mpFetchInterfaceDataAction;
  QAction *mpTLMCoSimulationAction;
  bool saveModelicaLibraryTreeNode(LibraryTreeNode *pLibraryTreeNode);
  bool saveTextLibraryTreeNode(LibraryTreeNode *pLibraryTreeNode);
  bool saveTLMLibraryTreeNode(LibraryTreeNode *pLibraryTreeNode);
  bool saveLibraryTreeNodeHelper(LibraryTreeNode *pLibraryTreeNode);
  bool saveLibraryTreeNodeOneFileHelper(LibraryTreeNode *pLibraryTreeNode);
  bool setSubModelsFileNameOneFileHelper(LibraryTreeNode *pLibraryTreeNode, QString filePath);
  void setSubModelsSavedOneFileHelper(LibraryTreeNode *pLibraryTreeNode);
  bool saveLibraryTreeNodeFolderHelper(LibraryTreeNode *pLibraryTreeNode);
  bool saveSubModelsFolderHelper(LibraryTreeNode *pLibraryTreeNode, QString directoryName);
  bool saveLibraryTreeNodeOneFileOrFolderHelper(LibraryTreeNode *pLibraryTreeNode);
  void unloadLibraryTreeNodeAndModelWidget(LibraryTreeNode *pLibraryTreeNode);
public slots:
  void expandLibraryTreeNode(QTreeWidgetItem *item);
  void showContextMenu(QPoint point);
  void createNewModelicaClass();
  void viewDocumentation();
  void simulate();
  void simulateWithTransformationalDebugger();
  void simulateWithAlgorithmicDebugger();
  void simulationSetup();
  void instantiateModel();
  void checkModel();
  void checkAllModels();
  void duplicateClass();
  void unloadClass();
  void unloadTextFile();
  void unloadTLMFile();
  void refresh();
  void exportModelFMU();
  void exportModelXML();
  void exportModelFigaro();
  void fetchInterfaceData();
  void TLMSimulate();
  void openFile(QString fileName, QString encoding = Helper::utf8, bool showProgress = true, bool checkFileExists = false);
  void openModelicaFile(QString fileName, QString encoding = Helper::utf8, bool showProgress = true);
  void openTLMFile(QFileInfo fileInfo, bool showProgress = true);
  void parseAndLoadModelicaText(QString modelText);
  void showModelWidget(LibraryTreeNode *pLibraryTreeNode = 0, bool newClass = false, bool extendsClass = false, QString text = QString());
  void openLibraryTreeNode(QString nameStructure);
  void loadLibraryComponent(LibraryTreeNode *pLibraryTreeNode);
protected:
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
  virtual void startDrag(Qt::DropActions supportedActions);
  Qt::DropActions supportedDropActions() const;
};

class LibraryComponent
{
public:
  LibraryComponent(QString value, QString className, OMCProxy *omc);
  ~LibraryComponent();
  QPixmap getComponentPixmap(QSize size);
  void hasIconAnnotation(Component *pComponent);

  QString mClassName;
  Component *mpComponent;
  QGraphicsView *mpGraphicsView;
  QRectF mRectangle;
  bool mHasIconAnnotation;
};

#endif // LIBRARYTREEWIDGET_H
