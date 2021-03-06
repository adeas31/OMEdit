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

#include <limits>

#include "ModelicaClassDialog.h"
#include "StringHandler.h"
#include "ModelWidgetContainer.h"

LibraryBrowseDialog::LibraryBrowseDialog(QString title, QLineEdit *pLineEdit, LibraryTreeWidget *pParent)
  : QDialog(0, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(title));
  resize(250, 500);
  mpLineEdit = pLineEdit;
  mpLibraryTreeWidget = pParent;
  mpFindClassTextBox = new QLineEdit;
  mpFindClassTextBox->setPlaceholderText(Helper::findClasses);
  connect(mpFindClassTextBox, SIGNAL(textEdited(QString)), SLOT(findModelicaClasses()));
  connect(mpFindClassTextBox, SIGNAL(returnPressed()), SLOT(useModelicaClass()));
  mpLibraryBrowseTreeWidget = new QTreeWidget;
  mpLibraryBrowseTreeWidget->setObjectName("TreeWithBranches");
  /*! @note: this is needed to hide the icon of the tree item.
      Since the icons of the tree items are only created when we expand the node. So its better to hide them here.
    */
  //mpLibraryBrowseTreeWidget->setIconSize(QSize(0,0));
  /*! @note: Commented the above line because Qt flush too many warnings if IconSize is 0.
      We will see icons for the expanded models otherwise default icon is shown. Fair enough!
    */
  mpLibraryBrowseTreeWidget->setItemDelegate(new ItemDelegate(mpLibraryBrowseTreeWidget));
  mpLibraryBrowseTreeWidget->setTextElideMode(Qt::ElideMiddle);
  mpLibraryBrowseTreeWidget->setHeaderLabel(Helper::libraries);
  mpLibraryBrowseTreeWidget->setIndentation(Helper::treeIndentation);
  mpLibraryBrowseTreeWidget->setExpandsOnDoubleClick(false);
  connect(mpLibraryBrowseTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(useModelicaClass()));
  for (int i = 0; i < mpLibraryTreeWidget->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *pLibraryTreeItem = mpLibraryTreeWidget->topLevelItem(i)->clone();
    mpLibraryBrowseTreeWidget->addTopLevelItem(pLibraryTreeItem);
  }
  // if the text box has some value then expand the tree and select the item accordingly.
  if (!pLineEdit->text().isEmpty())
  {
    QTreeWidgetItemIterator it(mpLibraryBrowseTreeWidget);
    while (*it)
    {
      QTreeWidgetItem *pTreeWidgetItem = dynamic_cast<QTreeWidgetItem*>((*it));
      if (pTreeWidgetItem->data(0, Qt::UserRole).toString().compare(pLineEdit->text()) == 0)
      {
        pTreeWidgetItem->setSelected(true);
        pTreeWidgetItem->setExpanded(true);
        // we must expand the parent items of the item we just selected.
        while (pTreeWidgetItem->parent())
        {
          pTreeWidgetItem = pTreeWidgetItem->parent();
          pTreeWidgetItem->setExpanded(true);
        }
        break;
      }
      ++it;
    }
  }
  // Create the buttons
  mpOkButton = new QPushButton(Helper::ok);
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), SLOT(useModelicaClass()));
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpFindClassTextBox, 0, 0);
  pMainLayout->addWidget(mpLibraryBrowseTreeWidget, 1, 0);
  pMainLayout->addWidget(mpButtonBox, 2, 0, 1, 1, Qt::AlignRight);
  setLayout(pMainLayout);
}

void LibraryBrowseDialog::unHideChildItems(QTreeWidgetItem *pTreeWidgetItem)
{
  QTreeWidgetItem *pChildItem;
  for (int i = 0 ; i < pTreeWidgetItem->childCount() ; i++)
  {
    pChildItem = pTreeWidgetItem->child(i);
    pChildItem->setExpanded(true);
    pChildItem->setHidden(false);
    if (pChildItem->childCount() > 0)
      unHideChildItems(pChildItem);
  }
}

void LibraryBrowseDialog::findModelicaClasses()
{
  mpLibraryBrowseTreeWidget->clearSelection();
  mpLibraryBrowseTreeWidget->collapseAll();
  if (mpFindClassTextBox->text().isEmpty())
  {
    QTreeWidgetItemIterator it(mpLibraryBrowseTreeWidget);
    while (*it)
    {
      QTreeWidgetItem *pTreeWidgetItem = dynamic_cast<QTreeWidgetItem*>((*it));
      pTreeWidgetItem->setHidden(false);
      ++it;
    }
    return;
  }
  QList<QTreeWidgetItem*> foundedItemsList;
  foundedItemsList = mpLibraryBrowseTreeWidget->findItems(mpFindClassTextBox->text(), Qt::MatchContains | Qt::MatchRecursive);
  // hide all the items first
  QTreeWidgetItemIterator it(mpLibraryBrowseTreeWidget);
  while (*it)
  {
    QTreeWidgetItem *pTreeWidgetItem = dynamic_cast<QTreeWidgetItem*>((*it));
    pTreeWidgetItem->setHidden(true);
    ++it;
  }
  // unhide the founded items
  foreach (QTreeWidgetItem *pTreeWidgetItem, foundedItemsList)
  {
    pTreeWidgetItem->setExpanded(true);
    pTreeWidgetItem->setHidden(false);
    // if the item has childs then unhide all the child items as well
    if (pTreeWidgetItem->childCount() > 0)
      unHideChildItems(pTreeWidgetItem);
    // we must unhide all the parent items as well
    while (pTreeWidgetItem->parent())
    {
      pTreeWidgetItem = pTreeWidgetItem->parent();
      pTreeWidgetItem->setExpanded(true);
      pTreeWidgetItem->setHidden(false);
    }
  }
  // select the first found item.
  if (!foundedItemsList.isEmpty())
    foundedItemsList.at(0)->setSelected(true);
}

void LibraryBrowseDialog::useModelicaClass()
{
  QList<QTreeWidgetItem*> selectedTreeWidgetItems = mpLibraryBrowseTreeWidget->selectedItems();
  if (!selectedTreeWidgetItems.isEmpty())
  {
    QTreeWidgetItem *pSelectedTreeWidgetItem = selectedTreeWidgetItems.at(0);
    mpLineEdit->setText(pSelectedTreeWidgetItem->data(0, Qt::UserRole).toString());
  }
  accept();
}

/*!
  \class ModelicaClassDialog
  \brief Creates a dialog to allow users to create new Modelica class restriction.
  */

/*!
  \param pParent - pointer to MainWindow
  */
ModelicaClassDialog::ModelicaClassDialog(MainWindow *pParent)
  : QDialog(pParent, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::createNewModelicaClass));
  setMinimumWidth(400);
  mpMainWindow = pParent;
  // Create the name label and text box
  mpNameLabel = new Label(Helper::name);
  mpNameTextBox = new QLineEdit;
  // Create the specialization label and combo box
  mpSpecializationLabel = new Label(tr("Specialization:"));
  mpSpecializationComboBox = new QComboBox;
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Model));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Class));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Connector));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::ExpandableConnector));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Record));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Block));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Function));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Package));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Type));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::Operator));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::OperatorRecord));
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::OperatorFunction));
  /* Don't add optimization restriction for now.
  mpSpecializationComboBox->addItem(StringHandler::getModelicaClassType(StringHandler::OPTIMIZATION));
  */
  connect(mpSpecializationComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(showHideSaveContentsInOneFileCheckBox(QString)));
  // create the extends the label and text box
  mpExtendsClassLabel = new Label(tr("Extends (optional):"));
  mpExtendsClassTextBox = new QLineEdit;
  mpExtendsClassBrowseButton = new QPushButton(Helper::browse);
  mpExtendsClassBrowseButton->setAutoDefault(false);
  connect(mpExtendsClassBrowseButton, SIGNAL(clicked()), SLOT(browseExtendsClass()));
  // Create the parent package label, text box, browse button
  mpParentClassLabel = new Label(tr("Insert in class (optional):"));
  mpParentClassTextBox = new QLineEdit;
  connect(mpParentClassTextBox, SIGNAL(textChanged(QString)), SLOT(showHideSaveContentsInOneFileCheckBox(QString)));
  mpParentClassBrowseButton = new QPushButton(Helper::browse);
  mpParentClassBrowseButton->setAutoDefault(false);
  connect(mpParentClassBrowseButton, SIGNAL(clicked()), SLOT(browseParentClass()));
  // create partial checkbox
  mpPartialCheckBox = new QCheckBox(tr("Partial"));
  // create encapsulated checkbox
  mpEncapsulatedCheckBox = new QCheckBox(tr("Encapsulated"));
  // create save contents of package in one file checkbox
  mpSaveContentsInOneFileCheckBox = new QCheckBox(tr("Save contents in one file"));
  mpSaveContentsInOneFileCheckBox->setChecked(true);
  mpSaveContentsInOneFileCheckBox->setVisible(false);
  // Create the buttons
  mpOkButton = new QPushButton(Helper::ok);
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), SLOT(createModelicaClass()));
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpNameLabel, 0, 0);
  pMainLayout->addWidget(mpNameTextBox, 0, 1, 1, 2);
  pMainLayout->addWidget(mpSpecializationLabel, 1, 0);
  pMainLayout->addWidget(mpSpecializationComboBox, 1, 1, 1, 2);
  pMainLayout->addWidget(mpExtendsClassLabel, 2, 0);
  pMainLayout->addWidget(mpExtendsClassTextBox, 2, 1);
  pMainLayout->addWidget(mpExtendsClassBrowseButton, 2, 2);
  pMainLayout->addWidget(mpParentClassLabel, 3, 0);
  pMainLayout->addWidget(mpParentClassTextBox, 3, 1);
  pMainLayout->addWidget(mpParentClassBrowseButton, 3, 2);
  pMainLayout->addWidget(mpPartialCheckBox, 4, 0);
  pMainLayout->addWidget(mpSaveContentsInOneFileCheckBox, 4, 1, 1, 2);
  pMainLayout->addWidget(mpEncapsulatedCheckBox, 5, 0, 1, 3);
  pMainLayout->addWidget(mpButtonBox, 6, 0, 1, 3, Qt::AlignRight);
  setLayout(pMainLayout);
}

MainWindow* ModelicaClassDialog::getMainWindow()
{
  return mpMainWindow;
}

QLineEdit* ModelicaClassDialog::getParentClassTextBox()
{
  return mpParentClassTextBox;
}

void ModelicaClassDialog::showHideSaveContentsInOneFileCheckBox(QString text)
{
  QComboBox *pComboBox = qobject_cast<QComboBox*>(sender());
  QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(sender());
  if (pComboBox && pComboBox == mpSpecializationComboBox)
  {
    if ((text.toLower().compare("package") == 0) && mpParentClassTextBox->text().isEmpty())
      mpSaveContentsInOneFileCheckBox->setVisible(true);
    else
      mpSaveContentsInOneFileCheckBox->setVisible(false);
  }
  else if (pLineEdit && pLineEdit == mpParentClassTextBox)
  {
    if (text.isEmpty() && (mpSpecializationComboBox->currentText().toLower().compare("package") == 0))
      mpSaveContentsInOneFileCheckBox->setVisible(true);
    else
      mpSaveContentsInOneFileCheckBox->setVisible(false);
  }
}

void ModelicaClassDialog::browseExtendsClass()
{
  LibraryBrowseDialog *pLibraryBrowseDialog = new LibraryBrowseDialog(tr("Select Extends Class"), mpExtendsClassTextBox, mpMainWindow->getLibraryTreeWidget());
  pLibraryBrowseDialog->exec();
}

void ModelicaClassDialog::browseParentClass()
{
  LibraryBrowseDialog *pLibraryBrowseDialog = new LibraryBrowseDialog(tr("Select Parent Class"), mpParentClassTextBox, mpMainWindow->getLibraryTreeWidget());
  pLibraryBrowseDialog->exec();
}

/*!
  Creates a new Modelica class restriction.\n
  Slot activated when mpOkButton clicked signal is raised.
  */
void ModelicaClassDialog::createModelicaClass()
{
  if (mpNameTextBox->text().isEmpty()) {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                            GUIMessages::ENTER_NAME).arg(mpSpecializationComboBox->currentText()), Helper::ok);
    return;
  }
  /* if extends class doesn't exist. */
  if (!mpExtendsClassTextBox->text().isEmpty()) {
    if (!mpMainWindow->getOMCProxy()->existClass(mpExtendsClassTextBox->text())) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::EXTENDS_CLASS_NOT_FOUND).arg(mpExtendsClassTextBox->text()), Helper::ok);
      return;
    }
  }
  /* if insert in class doesn't exist. */
  if (!mpParentClassTextBox->text().isEmpty()) {
    if (!mpMainWindow->getOMCProxy()->existClass(mpParentClassTextBox->text())) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::INSERT_IN_CLASS_NOT_FOUND).arg(mpParentClassTextBox->text()), Helper::ok);
      return;
    }
  }
  /* if insert in class is system library. */
  LibraryTreeWidget *pLibraryTreeWidget = mpMainWindow->getLibraryTreeWidget();
  LibraryTreeNode *pParentLibraryTreeNode = pLibraryTreeWidget->getLibraryTreeNode(mpParentClassTextBox->text());
  if (pParentLibraryTreeNode) {
    if (pParentLibraryTreeNode->isSystemLibrary()) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::INSERT_IN_SYSTEM_LIBRARY_NOT_ALLOWED).arg(mpParentClassTextBox->text()), Helper::ok);
      return;
    }
  }
  QString model, parentPackage;
  if (mpParentClassTextBox->text().isEmpty()) {
    model = mpNameTextBox->text().trimmed();
    parentPackage = "Global Scope";
  } else {
    model = QString(mpParentClassTextBox->text().trimmed()).append(".").append(mpNameTextBox->text().trimmed());
    parentPackage = QString("in Package '").append(mpParentClassTextBox->text().trimmed()).append("'");
  }
  // Check whether model exists or not.
  if (mpMainWindow->getOMCProxy()->existClass(model) || mpMainWindow->getLibraryTreeWidget()->getLibraryTreeNode(model)) {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                            GUIMessages::MODEL_ALREADY_EXISTS).arg(mpSpecializationComboBox->currentText()).arg(model)
                          .arg(parentPackage), Helper::ok);
    return;
  }
  // create the model.
  QString modelicaClass = mpEncapsulatedCheckBox->isChecked() ? "encapsulated " : "";
  modelicaClass.append(mpPartialCheckBox->isChecked() ? "partial " : "");
  modelicaClass.append(mpSpecializationComboBox->currentText().toLower());
  if (mpParentClassTextBox->text().isEmpty()) {
    if (!mpMainWindow->getOMCProxy()->createClass(modelicaClass, mpNameTextBox->text().trimmed(), mpExtendsClassTextBox->text().trimmed())) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::ERROR_OCCURRED).arg(mpMainWindow->getOMCProxy()->getErrorString()).append("\n\n").
                            append(GUIMessages::getMessage(GUIMessages::NO_OPENMODELICA_KEYWORDS)), Helper::ok);
      return;
    }
  } else {
    if (!mpMainWindow->getOMCProxy()->createSubClass(modelicaClass, mpNameTextBox->text().trimmed(), mpParentClassTextBox->text().trimmed(), mpExtendsClassTextBox->text().trimmed())) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::ERROR_OCCURRED).arg(mpMainWindow->getOMCProxy()->getErrorString()).append("\n\n").
                            append(GUIMessages::getMessage(GUIMessages::NO_OPENMODELICA_KEYWORDS)), Helper::ok);
      return;
    }
  }
  //open the new tab in central widget and add the model to library tree.
  LibraryTreeNode *pLibraryTreeNode;
  pLibraryTreeNode = pLibraryTreeWidget->addLibraryTreeNode(mpNameTextBox->text().trimmed(), mpParentClassTextBox->text().trimmed(), false);
  if (pLibraryTreeNode) {
    pLibraryTreeNode->setSaveContentsType(mpSaveContentsInOneFileCheckBox->isChecked() ? LibraryTreeNode::SaveInOneFile : LibraryTreeNode::SaveFolderStructure);
    pLibraryTreeWidget->addToExpandedLibraryTreeNodesList(pLibraryTreeNode);
    pLibraryTreeWidget->showModelWidget(pLibraryTreeNode, true, !mpExtendsClassTextBox->text().isEmpty());
    accept();
  }
}

/*!
  \class OpenModelicaFile
  \brief Creates a dialog to allow users to open Modelica file(s).
  */

/*!
  \param pParent - pointer to MainWindow
  */
OpenModelicaFile::OpenModelicaFile(MainWindow *pParent)
  : QDialog(pParent, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::openConvertModelicaFiles));
  setMinimumWidth(400);
  setModal(true);
  mpMainWindow = pParent;
  // create the File Label, textbox and browse button.
  mpFileLabel = new Label(Helper::file);
  mpFileTextBox = new QLineEdit;
  mpFileBrowseButton = new QPushButton(Helper::browse);
  mpFileBrowseButton->setAutoDefault(false);
  connect(mpFileBrowseButton, SIGNAL(clicked()), SLOT(browseForFile()));
  // create the encoding label, combobox and encoding note.
  mpEncodingLabel = new Label(Helper::encoding);
  mpEncodingComboBox = new QComboBox;
  StringHandler::fillEncodingComboBox(mpEncodingComboBox);
  mpConvertAllFilesCheckBox = new QCheckBox(tr("Convert all files within the selected directory and sub-directories"));
  // Create the buttons
  /* Open with selected encoding button */
  mpOpenWithEncodingButton = new QPushButton(tr("Open with selected encoding"));
  mpOpenWithEncodingButton->setStyleSheet("QPushButton{padding: 5px 15px 5px 15px;}");
  mpOpenWithEncodingButton->setAutoDefault(true);
  connect(mpOpenWithEncodingButton, SIGNAL(clicked()), SLOT(openModelicaFiles()));
  /* Open and convert to utf-8 button */
  mpOpenAndConvertToUTF8Button = new QPushButton(tr("Open and convert to UTF-8"));
  mpOpenAndConvertToUTF8Button->setStyleSheet("QPushButton{padding: 5px 15px 5px 15px;}");
  connect(mpOpenAndConvertToUTF8Button, SIGNAL(clicked()), SLOT(convertModelicaFiles()));
  /* cancel button */
  mpCancelButton = new QPushButton(Helper::cancel);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOpenWithEncodingButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpOpenAndConvertToUTF8Button, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpFileLabel, 0, 0);
  pMainLayout->addWidget(mpFileTextBox, 0, 1);
  pMainLayout->addWidget(mpFileBrowseButton, 0, 2);
  pMainLayout->addWidget(mpEncodingLabel, 1, 0);
  pMainLayout->addWidget(mpEncodingComboBox, 1, 1, 1, 2);
  pMainLayout->addWidget(mpConvertAllFilesCheckBox, 2, 0, 1, 3);
  pMainLayout->addWidget(mpButtonBox, 3, 0, 1, 3, Qt::AlignRight);
  setLayout(pMainLayout);
}

/*!
  Looks for the files and directories for conversion to UTF-8.
  \param filesAndDirectories - the list of files and directories that should be converted.
  \param path - the directory path where files and directories are located.
  */
void OpenModelicaFile::convertModelicaFiles(QStringList filesAndDirectories, QString path, QTextCodec *pCodec)
{
  foreach (QString fileOrDirectory, filesAndDirectories)
  {
    QFileInfo fileOrDirectoryInfo(QString(path).append("/").append(fileOrDirectory));
    if (fileOrDirectoryInfo.isDir())
    {
      QDir directory(QString(path).append("/").append(fileOrDirectory));
      QStringList nameFilter("*.mo");
      QStringList filesAndDirectories = directory.entryList(nameFilter, QDir::AllDirs | QDir::Files | QDir::NoSymLinks |
                                                            QDir::NoDotAndDotDot | QDir::Writable | QDir::CaseSensitive);
      convertModelicaFiles(filesAndDirectories, directory.absolutePath(), pCodec);
    }
    else
    {
      convertModelicaFile(QString(path).append("/").append(fileOrDirectory), pCodec);
    }
  }
}

/*!
  Converts the file to UTF-8 encoding.
  \param fileName - the full name of the file to convert.
  */
void OpenModelicaFile::convertModelicaFile(QString fileName, QTextCodec *pCodec)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  QString fileData(pCodec->toUnicode(file.readAll()));
  file.close();
  file.open(QIODevice::WriteOnly | QIODevice::Truncate);
  QTextStream out(&file);
  out.setCodec(Helper::utf8.toStdString().data());
  out.setGenerateByteOrderMark(false);
  out << fileData;
  file.close();
}

/*!
  Asks the user to select the Modelica file(s) they want to open.\n
  Slot activated when mpFileBrowseButton clicked signal is raised.
  */
void OpenModelicaFile::browseForFile()
{
  QStringList fileNames;
  mFileNames.clear();
  fileNames = StringHandler::getOpenFileNames(this, QString(Helper::applicationName).append(" - ").append(Helper::chooseFiles),
                                              NULL, Helper::omFileTypes, NULL);
  foreach (QString fileName, fileNames)
  {
    mFileNames.append(fileName.replace("\\", "/"));
  }
  mpFileTextBox->setText(mFileNames.join(";"));
}

/*!
  Opens the selected Modelica file(s).\n
  Slot activated when mpOpenButton clicked signal is raised.
  */
void OpenModelicaFile::openModelicaFiles(bool convertedToUTF8)
{
  int progressValue = 0;
  mpMainWindow->getProgressBar()->setRange(0, mFileNames.size());
  mpMainWindow->showProgressBar();
  foreach (QString fileName, mFileNames)
  {
    mpMainWindow->getStatusBar()->showMessage(QString(Helper::loading).append(": ").append(fileName));
    mpMainWindow->getProgressBar()->setValue(++progressValue);
    // if file doesn't exists
    if (!QFile::exists(fileName))
    {
      QMessageBox *pMessageBox = new QMessageBox(mpMainWindow);
      pMessageBox->setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::error));
      pMessageBox->setIcon(QMessageBox::Critical);
      pMessageBox->setAttribute(Qt::WA_DeleteOnClose);
      pMessageBox->setText(QString(GUIMessages::getMessage(GUIMessages::UNABLE_TO_LOAD_FILE).arg(fileName)));
      pMessageBox->setInformativeText(QString(GUIMessages::getMessage(GUIMessages::FILE_NOT_FOUND).arg(fileName)));
      pMessageBox->setStandardButtons(QMessageBox::Ok);
      pMessageBox->exec();
    }
    else
    {
      if (convertedToUTF8)
        mpMainWindow->getLibraryTreeWidget()->openFile(fileName, Helper::utf8, false);
      else
        mpMainWindow->getLibraryTreeWidget()->openFile(fileName, mpEncodingComboBox->itemData(mpEncodingComboBox->currentIndex()).toString(), false);
    }
  }
  mpMainWindow->getStatusBar()->clearMessage();
  mpMainWindow->hideProgressBar();
  accept();
}

/*!
  Converts the selected Modelica file(s).\n
  Slot activated when mpOpenAndConvertToUTF8Button clicked signal is raised.
  */
void OpenModelicaFile::convertModelicaFiles()
{
  QTextCodec *pCodec = QTextCodec::codecForName(mpEncodingComboBox->itemData(mpEncodingComboBox->currentIndex()).toString().toStdString().data());
  if (pCodec != NULL)
  {
    mpMainWindow->getStatusBar()->showMessage(tr("Converting files to UTF-8"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach (QString fileName, mFileNames)
    {
      if (QFile::exists(fileName))
      {
        if (mpConvertAllFilesCheckBox->isChecked())
        {
          QFileInfo fileInfo(fileName);
          QDir directory = fileInfo.absoluteDir();
          QStringList nameFilter("*.mo");
          QStringList filesAndDirectories = directory.entryList(nameFilter, QDir::AllDirs | QDir::Files | QDir::NoSymLinks |
                                                                QDir::NoDotAndDotDot | QDir::Writable | QDir::CaseSensitive);
          convertModelicaFiles(filesAndDirectories, directory.absolutePath(), pCodec);
        }
        else
        {
          convertModelicaFile(fileName, pCodec);
        }
      }
    }
  }
  mpMainWindow->getStatusBar()->clearMessage();
  QApplication::restoreOverrideCursor();
  openModelicaFiles(true);
}

/*!
  \class SaveAsClassDialog
  \brief Creates a dialog to allow users to save as the Modelica class.
  */

/*!
  \param pParent - pointer to MainWindow
  */
SaveAsClassDialog::SaveAsClassDialog(ModelWidget *pModelWidget, MainWindow *pParent)
  : QDialog(pParent, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(tr("Save As Modelica Class")));
  setMinimumWidth(400);
  mpModelWidget = pModelWidget;
  mpMainWindow = pParent;
  setModal(true);
  // Create the name label and text box
  mpNameLabel = new Label(Helper::name);
  mpNameTextBox = new QLineEdit(pModelWidget->getLibraryTreeNode()->getName());
  // Create the parent package label, text box, browse button
  mpParentPackageLabel = new Label(tr("Insert in class (optional):"));
  mpParentClassComboBox = new QComboBox;
  mpParentClassComboBox->setEditable(true);
  /* Since the default QCompleter for QComboBox is case insenstive. */
  QCompleter *pParentClassComboBoxCompleter = mpParentClassComboBox->completer();
  pParentClassComboBoxCompleter->setCaseSensitivity(Qt::CaseSensitive);
  mpParentClassComboBox->setCompleter(pParentClassComboBoxCompleter);
  mpParentClassComboBox->addItem("");
//  mpParentClassComboBox->addItems(mpMainWindow->getLibraryTreeWidget()->getNonSystemLibraryTreeNodeStringList());
  int currentIndex = mpParentClassComboBox->findText(pModelWidget->getLibraryTreeNode()->getParentName(), Qt::MatchExactly);
  if (currentIndex > -1)
    mpParentClassComboBox->setCurrentIndex(currentIndex);
  connect(mpParentClassComboBox, SIGNAL(editTextChanged(QString)), SLOT(showHideSaveContentsInOneFileCheckBox(QString)));
  // create save contents of package in one file checkbox
  mpSaveContentsInOneFileCheckBox = new QCheckBox(tr("Save contents in one file"));
  mpSaveContentsInOneFileCheckBox->setChecked(true);
  if (pModelWidget->getLibraryTreeNode()->getRestriction() == StringHandler::Package && mpParentClassComboBox->currentText().isEmpty())
    mpSaveContentsInOneFileCheckBox->setVisible(true);
  else
    mpSaveContentsInOneFileCheckBox->setVisible(false);
  // Create the buttons
  mpOkButton = new QPushButton(Helper::ok);
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), SLOT(saveAsModelicaClass()));
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpNameLabel, 0, 0);
  pMainLayout->addWidget(mpNameTextBox, 0, 1);
  pMainLayout->addWidget(mpParentPackageLabel, 1, 0);
  pMainLayout->addWidget(mpParentClassComboBox, 1, 1);
  pMainLayout->addWidget(mpSaveContentsInOneFileCheckBox, 2, 0);
  pMainLayout->addWidget(mpButtonBox, 2, 1, Qt::AlignRight);
  setLayout(pMainLayout);
}

QComboBox* SaveAsClassDialog::getParentClassComboBox()
{
  return mpParentClassComboBox;
}

/*!
  Creates a new Modelica class restriction.\n
  Slot activated when mpOkButton clicked signal is raised.
  */
void SaveAsClassDialog::saveAsModelicaClass()
{
  QString type = StringHandler::getModelicaClassType(mpModelWidget->getLibraryTreeNode()->getRestriction());
  if (mpNameTextBox->text().isEmpty())
  {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                            GUIMessages::ENTER_NAME).arg(type), Helper::ok);
    return;
  }
  /* if insert in class doesn't exist. */
  if (!mpParentClassComboBox->currentText().isEmpty())
  {
    if (!mpMainWindow->getOMCProxy()->existClass(mpParentClassComboBox->currentText()))
    {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::INSERT_IN_CLASS_NOT_FOUND).arg(mpParentClassComboBox->currentText()), Helper::ok);
      return;
    }
  }
  /* if insert in class is system library. */
  LibraryTreeWidget *pLibraryTreeWidget = mpMainWindow->getLibraryTreeWidget();
  LibraryTreeNode *pParentLibraryTreeNode = pLibraryTreeWidget->getLibraryTreeNode(mpParentClassComboBox->currentText());
  if (pParentLibraryTreeNode)
  {
    if (pParentLibraryTreeNode->isSystemLibrary())
    {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::INSERT_IN_SYSTEM_LIBRARY_NOT_ALLOWED).arg(mpParentClassComboBox->currentText()), Helper::ok);
      return;
    }
  }

  QString model, parentPackage;
  if (mpParentClassComboBox->currentText().isEmpty())
  {
    model = mpNameTextBox->text();
    parentPackage = "Global Scope";
  }
  else
  {
    model = QString(mpParentClassComboBox->currentText()).append(".").append(mpNameTextBox->text());
    parentPackage = QString("in Package '").append(mpParentClassComboBox->currentText()).append("'");
  }
  // Check whether model exists or not.
  if (mpMainWindow->getOMCProxy()->existClass(model))
  {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                            GUIMessages::MODEL_ALREADY_EXISTS).arg(type).arg(model).arg(parentPackage), Helper::ok);
    return;
  }
  // duplicate the model.
  QString sourceModelText = mpMainWindow->getOMCProxy()->list(mpModelWidget->getLibraryTreeNode()->getNameStructure());
  QString duplicateModelText = sourceModelText;
  /* remove the starting and ending text strings of the model. */
  duplicateModelText.remove(0, QString(type.toLower()).append(" ").append(mpModelWidget->getLibraryTreeNode()->getName()).length());
  QString endString = QString("end ").append(mpModelWidget->getLibraryTreeNode()->getName()).append(";");
  duplicateModelText.remove(duplicateModelText.lastIndexOf(endString), endString.length());
  /* add the starting and ending text strings. */
  duplicateModelText.prepend(QString(type.toLower()).append(" ").append(mpNameTextBox->text()));
  duplicateModelText.append(QString("end ").append(mpNameTextBox->text()).append(";"));
  if (!mpParentClassComboBox->currentText().isEmpty())
  {
    duplicateModelText.prepend(QString("within ").append(mpParentClassComboBox->currentText()).append(";"));
  }
  mpMainWindow->getOMCProxy()->sendCommand(duplicateModelText);
  if (mpMainWindow->getOMCProxy()->getResult().toLower().contains("error"))
  {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                            GUIMessages::ERROR_OCCURRED).arg(mpMainWindow->getOMCProxy()->getResult()).append("\n\n").
                          append(GUIMessages::getMessage(GUIMessages::NO_OPENMODELICA_KEYWORDS)), Helper::ok);
    return;
  }
  //open the new tab in central widget and add the model to library tree.
  LibraryTreeNode *pLibraryTreeNode;
  pLibraryTreeNode = pLibraryTreeWidget->addLibraryTreeNode(mpNameTextBox->text(), mpParentClassComboBox->currentText(), false);
  if (pLibraryTreeNode) {
    pLibraryTreeNode->setSaveContentsType(mpSaveContentsInOneFileCheckBox->isChecked() ? LibraryTreeNode::SaveInOneFile : LibraryTreeNode::SaveFolderStructure);
    pLibraryTreeWidget->addToExpandedLibraryTreeNodesList(pLibraryTreeNode);
    pLibraryTreeWidget->showModelWidget(pLibraryTreeNode);
    accept();
  }
}

void SaveAsClassDialog::showHideSaveContentsInOneFileCheckBox(QString text)
{
  if (text.isEmpty() && mpModelWidget->getLibraryTreeNode()->getRestriction() == StringHandler::Package)
    mpSaveContentsInOneFileCheckBox->setVisible(true);
  else
    mpSaveContentsInOneFileCheckBox->setVisible(false);
}

/*!
  \class CopyClassDialog
  \brief Creates a dialog to allow users to copy the Modelica class.
  */

/*!
  \param name - name of Modelica class
  \param nameStructure - qualified name of Modelica class
  \param pParent - pointer to MainWindow
  */
DuplicateClassDialog::DuplicateClassDialog(LibraryTreeNode *pLibraryTreeNode, MainWindow *pMainWindow)
  : QDialog(pMainWindow, Qt::WindowTitleHint), mpLibraryTreeNode(pLibraryTreeNode), mpMainWindow(pMainWindow)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString("%1 - %2 %3").arg(Helper::applicationName).arg(Helper::duplicate).arg(mpLibraryTreeNode->getNameStructure()));
  mpNameLabel = new Label(Helper::name);
  mpNameTextBox = new QLineEdit;
  mpPathLabel = new Label(Helper::path);
  mpPathTextBox = new QLineEdit;
  mpPathBrowseButton = new QPushButton(Helper::browse);
  mpPathBrowseButton->setAutoDefault(false);
  connect(mpPathBrowseButton, SIGNAL(clicked()), SLOT(browsePath()));
  // Create the buttons
  mpOkButton = new QPushButton(Helper::ok);
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), SLOT(duplicateClass()));
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpNameLabel, 0, 0);
  pMainLayout->addWidget(mpNameTextBox, 0, 1, 1, 2);
  pMainLayout->addWidget(mpPathLabel, 1, 0);
  pMainLayout->addWidget(mpPathTextBox, 1, 1);
  pMainLayout->addWidget(mpPathBrowseButton, 1, 2);
  pMainLayout->addWidget(mpButtonBox, 2, 0, 1, 3, Qt::AlignRight);
  setLayout(pMainLayout);
}

void DuplicateClassDialog::browsePath()
{
  LibraryBrowseDialog *pLibraryBrowseDialog = new LibraryBrowseDialog(tr("Select Path"), mpPathTextBox, mpMainWindow->getLibraryTreeWidget());
  pLibraryBrowseDialog->exec();
}

/*!
 * \brief DuplicateClassDialog::duplicateClass
 * Duplicates the class.
 */
void DuplicateClassDialog::duplicateClass()
{
  if (mpNameTextBox->text().isEmpty()) {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error),
                          GUIMessages::getMessage(GUIMessages::ENTER_NAME).arg("class"), Helper::ok);
    return;
  }
  /* if path class doesn't exist. */
  if (!mpPathTextBox->text().isEmpty()) {
    if (!mpMainWindow->getOMCProxy()->existClass(mpPathTextBox->text())) {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), GUIMessages::getMessage(
                              GUIMessages::INSERT_IN_CLASS_NOT_FOUND).arg(mpPathTextBox->text()), Helper::ok);
      return;
    }
  }
  // check if new class already exists
  QString newClassPath = (mpPathTextBox->text().isEmpty() ? "" : mpPathTextBox->text() + ".") + mpNameTextBox->text();
  if (mpMainWindow->getOMCProxy()->existClass(newClassPath) || mpMainWindow->getLibraryTreeWidget()->getLibraryTreeNode(newClassPath)) {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error),
                          GUIMessages::getMessage(GUIMessages::MODEL_ALREADY_EXISTS).arg("class").arg(mpNameTextBox->text())
                          .arg((mpPathTextBox->text().isEmpty() ? "Top Level" : mpPathTextBox->text())), Helper::ok);
    return;
  }
  // if everything is fine then duplicate the class.
  if (mpMainWindow->getOMCProxy()->copyClass(mpLibraryTreeNode->getNameStructure(), mpNameTextBox->text(), mpPathTextBox->text())) {
    LibraryTreeWidget *pLibraryTreeWidget = mpMainWindow->getLibraryTreeWidget();
    LibraryTreeNode *pLibraryTreeNode;
    QString className = mpNameTextBox->text().trimmed();
    pLibraryTreeNode = pLibraryTreeWidget->addLibraryTreeNode(className, mpPathTextBox->text().trimmed(), false);
    if (pLibraryTreeNode) {
      pLibraryTreeNode->setSaveContentsType(mpLibraryTreeNode->getSaveContentsType());
      pLibraryTreeWidget->createLibraryTreeNodes(pLibraryTreeNode);
    }
  }
  accept();
}

/*!
  \class RenameClassDialog
  \brief Creates a dialog to allow users to rename the Modelica class.
  */

/*!
  \param name - name of Modelica class
  \param nameStructure - qualified name of Modelica class
  \param pParent - pointer to MainWindow
  */
RenameClassDialog::RenameClassDialog(QString name, QString nameStructure, MainWindow *parent)
  : QDialog(parent, Qt::WindowTitleHint), mName(name), mNameStructure(nameStructure)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - Rename ").append(name));
  setMinimumSize(300, 100);
  setModal(true);
  mpMainWindow = parent;
  mpModelNameTextBox = new QLineEdit(name);
  mpModelNameLabel = new Label(tr("New Name:"));
  // Create the buttons
  mpOkButton = new QPushButton(tr("Rename"));
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), this, SLOT(renameClass()));
  mpCancelButton = new QPushButton(tr("&Cancel"));
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(mpModelNameLabel, 0, 0);
  mainLayout->addWidget(mpModelNameTextBox, 1, 0);
  mainLayout->addWidget(mpButtonBox, 2, 0);

  setLayout(mainLayout);
}

/*!
  Renames the Modelica class.\n
  Slot activated when mpOkButton clicked signal is raised.
  */
void RenameClassDialog::renameClass()
{
  QString newName = mpModelNameTextBox->text().trimmed();
  QString newNameStructure;
  // if no change in the name then return
  if (newName == mName)
  {
    accept();
    return;
  }

  if (!mpMainWindow->getOMCProxy()->existClass(QString(StringHandler::removeLastWordAfterDot(mNameStructure)).append(".").append(newName)))
  {
    if (mpMainWindow->getOMCProxy()->renameClass(mNameStructure, newName))
    {
      newNameStructure = StringHandler::removeFirstLastCurlBrackets(mpMainWindow->getOMCProxy()->getResult());
      // Change the name in tree
      //mpParentMainWindow->mpLibrary->updateNodeText(newName, newNameStructure);
      accept();
    }
    else
    {
      QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error),
                            GUIMessages::getMessage(GUIMessages::ERROR_OCCURRED).arg(mpMainWindow->getOMCProxy()->getResult())
                            .append("\n\n").append(GUIMessages::getMessage(GUIMessages::NO_OPENMODELICA_KEYWORDS)), Helper::ok);
      return;
    }
  }
  else
  {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error),
                          GUIMessages::getMessage(GUIMessages::ITEM_ALREADY_EXISTS).append("\n\n")
                          .append(GUIMessages::getMessage(GUIMessages::NO_OPENMODELICA_KEYWORDS)), Helper::ok);
    return;
  }
}

/*!
  \class InformationDialog
  \brief Creates a dialog that shows the users the result of OMCProxy::instantiateModel and OMCProxy::checkModel.
  */

/*!
  \param windowTitle - title string for dialog
  \param informationText - main text string for dialog
  \param modelicaTextHighlighter - highlights the modelica code.
  \param pParent - pointer to MainWindow
  */
InformationDialog::InformationDialog(QString windowTitle, QString informationText, bool modelicaTextHighlighter, MainWindow *pMainWindow)
  : QWidget(pMainWindow, Qt::Window)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(windowTitle));
  mpMainWindow = pMainWindow;
  // instantiate the model
  QPlainTextEdit *pPlainTextEdit = new QPlainTextEdit(informationText);
  if (modelicaTextHighlighter) {
    ModelicaTextHighlighter *pModelicaHighlighter = new ModelicaTextHighlighter(mpMainWindow->getOptionsDialog()->getModelicaTextEditorPage(),
                                                                                pPlainTextEdit);
    Q_UNUSED(pModelicaHighlighter);
  }
  // Create the button
  QPushButton *pOkButton = new QPushButton(Helper::ok);
  connect(pOkButton, SIGNAL(clicked()), SLOT(close()));
  // set layout
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setAlignment(Qt::AlignRight);
  buttonLayout->addWidget(pOkButton);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(pPlainTextEdit);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);
  pOkButton->setFocus();
  /* restore the window geometry. */
  if (mpMainWindow->getOptionsDialog()->getGeneralSettingsPage()->getPreserveUserCustomizations())
  {
    QSettings *pSettings = OpenModelica::getApplicationSettings();
    restoreGeometry(pSettings->value("InformationDialog/geometry").toByteArray());
  }
}

void InformationDialog::closeEvent(QCloseEvent *event)
{
  /* save the window geometry. */
  if (mpMainWindow->getOptionsDialog()->getGeneralSettingsPage()->getPreserveUserCustomizations())
  {
    QSettings *pSettings = OpenModelica::getApplicationSettings();
    pSettings->setValue("InformationDialog/geometry", saveGeometry());
  }
  event->accept();
}

/*!
  \class GraphicsViewProperties
  \brief Creates a dialog that shows the icon/diagram GraphicsView properties.
  */

/*!
  \param pGraphicsView - pointer to GraphicsView
  */
GraphicsViewProperties::GraphicsViewProperties(GraphicsView *pGraphicsView)
  : QDialog(pGraphicsView, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::properties));
  setMinimumWidth(400);
  setModal(true);
  mpGraphicsView = pGraphicsView;
  // create extent points group box
  mpExtentGroupBox = new QGroupBox(Helper::extent);
  mpLeftLabel = new Label(QString(Helper::left).append(":"));
  mpLeftSpinBox = new DoubleSpinBox;
  mpLeftSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  mpLeftSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getExtent().at(0).x());
  mpLeftSpinBox->setSingleStep(10);
  mpBottomLabel = new Label(Helper::bottom);
  mpBottomSpinBox = new DoubleSpinBox;
  mpBottomSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  mpBottomSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getExtent().at(0).y());
  mpBottomSpinBox->setSingleStep(10);
  mpRightLabel = new Label(QString(Helper::right).append(":"));
  mpRightSpinBox = new DoubleSpinBox;
  mpRightSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  mpRightSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getExtent().at(1).x());
  mpRightSpinBox->setSingleStep(10);
  mpTopLabel = new Label(Helper::top);
  mpTopSpinBox = new DoubleSpinBox;
  mpTopSpinBox->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
  mpTopSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getExtent().at(1).y());
  mpTopSpinBox->setSingleStep(10);
  // set the extent group box layout
  QGridLayout *pExtentLayout = new QGridLayout;
  pExtentLayout->setColumnStretch(1, 1);
  pExtentLayout->setColumnStretch(3, 1);
  pExtentLayout->addWidget(mpLeftLabel, 0, 0);
  pExtentLayout->addWidget(mpLeftSpinBox, 0, 1);
  pExtentLayout->addWidget(mpBottomLabel, 0, 2);
  pExtentLayout->addWidget(mpBottomSpinBox, 0, 3);
  pExtentLayout->addWidget(mpRightLabel, 1, 0);
  pExtentLayout->addWidget(mpRightSpinBox, 1, 1);
  pExtentLayout->addWidget(mpTopLabel, 1, 2);
  pExtentLayout->addWidget(mpTopSpinBox, 1, 3);
  mpExtentGroupBox->setLayout(pExtentLayout);
  // create the grid group box
  mpGridGroupBox = new QGroupBox(Helper::grid);
  mpHorizontalLabel = new Label(QString(Helper::horizontal).append(":"));
  mpHorizontalSpinBox = new DoubleSpinBox;
  mpHorizontalSpinBox->setRange(0, std::numeric_limits<double>::max());
  mpHorizontalSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getGrid().x());
  mpHorizontalSpinBox->setSingleStep(1);
  mpVerticalLabel = new Label(QString(Helper::vertical).append(":"));
  mpVerticalSpinBox = new DoubleSpinBox;
  mpVerticalSpinBox->setRange(0, std::numeric_limits<double>::max());
  mpVerticalSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getGrid().y());
  mpVerticalSpinBox->setSingleStep(1);
  // set the grid group box layout
  QGridLayout *pGridLayout = new QGridLayout;
  pGridLayout->setColumnStretch(1, 1);
  pGridLayout->addWidget(mpHorizontalLabel, 0, 0);
  pGridLayout->addWidget(mpHorizontalSpinBox, 0, 1);
  pGridLayout->addWidget(mpVerticalLabel, 1, 0);
  pGridLayout->addWidget(mpVerticalSpinBox, 1, 1);
  mpGridGroupBox->setLayout(pGridLayout);
  // create the Component group box
  mpComponentGroupBox = new QGroupBox(Helper::component);
  mpScaleFactorLabel = new Label(Helper::scaleFactor);
  mpScaleFactorSpinBox = new DoubleSpinBox;
  mpScaleFactorSpinBox->setRange(0, std::numeric_limits<double>::max());
  mpScaleFactorSpinBox->setValue(mpGraphicsView->getCoOrdinateSystem()->getInitialScale());
  mpScaleFactorSpinBox->setSingleStep(0.1);
  mpPreserveAspectRatioCheckBox = new QCheckBox(Helper::preserveAspectRatio);
  mpPreserveAspectRatioCheckBox->setChecked(mpGraphicsView->getCoOrdinateSystem()->getPreserveAspectRatio());
  // set the grid group box layout
  QGridLayout *pComponentLayout = new QGridLayout;
  pComponentLayout->setColumnStretch(1, 1);
  pComponentLayout->addWidget(mpScaleFactorLabel, 0, 0);
  pComponentLayout->addWidget(mpScaleFactorSpinBox, 0, 1);
  pComponentLayout->addWidget(mpPreserveAspectRatioCheckBox, 1, 0, 1, 2);
  mpComponentGroupBox->setLayout(pComponentLayout);
  // copy properties check box
  mpCopyProperties = new QCheckBox;
  if (mpGraphicsView->getViewType() == StringHandler::Icon)
    mpCopyProperties->setText(tr("Copy properties to Diagram layer"));
  else
    mpCopyProperties->setText(tr("Copy properties to Icon layer"));
  mpCopyProperties->setChecked(true);
  // Create the buttons
  mpOkButton = new QPushButton(Helper::ok);
  mpOkButton->setAutoDefault(true);
  connect(mpOkButton, SIGNAL(clicked()), SLOT(saveGraphicsViewProperties()));
  if (mpGraphicsView->getModelWidget()->getLibraryTreeNode()->isSystemLibrary())
    mpOkButton->setDisabled(true);
  mpCancelButton = new QPushButton(Helper::cancel);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // set the layout
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  pMainLayout->addWidget(mpExtentGroupBox);
  pMainLayout->addWidget(mpGridGroupBox);
  pMainLayout->addWidget(mpComponentGroupBox);
  pMainLayout->addWidget(mpCopyProperties);
  pMainLayout->addWidget(mpButtonBox, 0, Qt::AlignRight);
  setLayout(pMainLayout);
}

/*!
  Saves the new GraphicsView properties in the form of coordinate system annotation.\n
  Slot activated when mpOkButton clicked signal is raised.
  */
void GraphicsViewProperties::saveGraphicsViewProperties()
{
  qreal left = qMin(mpLeftSpinBox->value(), mpRightSpinBox->value());
  qreal bottom = qMin(mpBottomSpinBox->value(), mpTopSpinBox->value());
  qreal right = qMax(mpLeftSpinBox->value(), mpRightSpinBox->value());
  qreal top = qMax(mpBottomSpinBox->value(), mpTopSpinBox->value());
  QList<QPointF> extent;
  extent << QPointF(left, bottom) << QPointF(right, top);
  mpGraphicsView->getCoOrdinateSystem()->setExtent(extent);
  mpGraphicsView->getCoOrdinateSystem()->setPreserveAspectRatio(mpPreserveAspectRatioCheckBox->isChecked());
  mpGraphicsView->getCoOrdinateSystem()->setInitialScale(mpScaleFactorSpinBox->value());
  qreal horizontal = mpHorizontalSpinBox->value();
  qreal vertical = mpVerticalSpinBox->value();
  mpGraphicsView->getCoOrdinateSystem()->setGrid(QPointF(horizontal, vertical));
  mpGraphicsView->setExtentRectangle(left, bottom, right, top);
  mpGraphicsView->fitInView(mpGraphicsView->getExtentRectangle(), Qt::KeepAspectRatio);
  mpGraphicsView->setIsCustomScale(false);
  mpGraphicsView->addClassAnnotation();
  mpGraphicsView->setCanAddClassAnnotation(true);
  // if copy properties is true
  if (mpCopyProperties->isChecked())
  {
    GraphicsView *pGraphicsView;
    if (mpGraphicsView->getViewType() == StringHandler::Icon)
      pGraphicsView = mpGraphicsView->getModelWidget()->getDiagramGraphicsView();
    else
      pGraphicsView = mpGraphicsView->getModelWidget()->getIconGraphicsView();

    pGraphicsView->getCoOrdinateSystem()->setExtent(extent);
    pGraphicsView->getCoOrdinateSystem()->setPreserveAspectRatio(mpPreserveAspectRatioCheckBox->isChecked());
    pGraphicsView->getCoOrdinateSystem()->setInitialScale(mpScaleFactorSpinBox->value());
    pGraphicsView->getCoOrdinateSystem()->setGrid(QPointF(horizontal, vertical));
    pGraphicsView->setExtentRectangle(left, bottom, right, top);
    pGraphicsView->fitInView(pGraphicsView->getExtentRectangle(), Qt::KeepAspectRatio);
    pGraphicsView->setIsCustomScale(false);
    pGraphicsView->addClassAnnotation();
    pGraphicsView->setCanAddClassAnnotation(true);
  }
  accept();
}

/*!
  \class SaveChangesDialog
  \brief Creates a dialog that shows the list of unsaved Modelica classes.
  */

/*!
  \param pMainWindow - pointer to MainWindow
  */
SaveChangesDialog::SaveChangesDialog(MainWindow *pMainWindow)
  : QDialog(pMainWindow, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(tr("Save Changes")));
  setMinimumWidth(400);
  mpMainWindow = pMainWindow;
  mpSaveChangesLabel = new Label(tr("Save changes to the following classes?"));
  mpUnsavedClassesListWidget = new QListWidget;
  mpUnsavedClassesListWidget->setObjectName("UnsavedClassesListWidget");
  mpUnsavedClassesListWidget->setItemDelegate(new ItemDelegate(mpUnsavedClassesListWidget));
  mpUnsavedClassesListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  // Create the buttons
  // create the Yes button
  mpYesButton = new QPushButton(tr("Yes"));
  mpYesButton->setAutoDefault(true);
  connect(mpYesButton, SIGNAL(clicked()), SLOT(saveChanges()));
  // create the No button
  mpNoButton = new QPushButton(tr("No"));
  mpNoButton->setAutoDefault(false);
  connect(mpNoButton, SIGNAL(clicked()), SLOT(accept()));
  // create the Cancel button
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpYesButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpNoButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // create a main layout
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->addWidget(mpSaveChangesLabel);
  pMainLayout->addWidget(mpUnsavedClassesListWidget);
  pMainLayout->addWidget(mpButtonBox, 0, Qt::AlignRight);
  setLayout(pMainLayout);
}

/*!
  \return false if no unsaved Modelica classes are present otherwise true.
  */
bool SaveChangesDialog::getUnsavedClasses()
{
  bool hasUnsavedClasses = false;
  foreach (LibraryTreeNode* pLibraryTreeNode, mpMainWindow->getLibraryTreeWidget()->getLibraryTreeNodesList()) {
    if (!pLibraryTreeNode->isSaved()) {
      if (pLibraryTreeNode->getParentName().isEmpty()) {
        hasUnsavedClasses = true;
        QListWidgetItem *pListItem = new QListWidgetItem(mpUnsavedClassesListWidget);
        pListItem->setText(pLibraryTreeNode->getNameStructure());
      } else {
        LibraryTreeNode *pParentLibraryTreeNode = mpMainWindow->getLibraryTreeWidget()->getLibraryTreeNode(StringHandler::getFirstWordBeforeDot(pLibraryTreeNode->getNameStructure()));
        if (pParentLibraryTreeNode) {
          QFileInfo fileInfo(pParentLibraryTreeNode->getFileName());
          if ((pParentLibraryTreeNode->getSaveContentsType() == LibraryTreeNode::SaveFolderStructure) || (fileInfo.fileName().compare("package.mo") == 0)) {
            hasUnsavedClasses = true;
            QListWidgetItem *pListItem = new QListWidgetItem(mpUnsavedClassesListWidget);
            pListItem->setText(pParentLibraryTreeNode->getNameStructure());
          }
        }
      }
    }
  }
  mpUnsavedClassesListWidget->selectAll();
  return hasUnsavedClasses;
}

/*!
  Saves the unsaved classes. \n
  Slot activated when mpYesButton clicked signal is raised.
  */
void SaveChangesDialog::saveChanges()
{
  bool saveResult = true;
  for (int i = 0; i < mpUnsavedClassesListWidget->count(); i++) {
    QListWidgetItem *pListItem = mpUnsavedClassesListWidget->item(i);
    LibraryTreeNode *pLibraryTreeNode = mpMainWindow->getLibraryTreeWidget()->getLibraryTreeNode(pListItem->text());
    if (!mpMainWindow->getLibraryTreeWidget()->saveLibraryTreeNode(pLibraryTreeNode)) {
      saveResult = false;
    }
  }
  if (saveResult) {
    accept();
  } else {
    reject();
  }
}

/*!
  Reimplementation of exec.
  */
int SaveChangesDialog::exec()
{
  if (!getUnsavedClasses())
    return 1;
  return QDialog::exec();
}

/*!
  \class ExportFigaroDialog
  \brief Creates a dialog for Figaro export.
  */

/*!
  \param pMainWindow - pointer to MainWindow
  */
ExportFigaroDialog::ExportFigaroDialog(MainWindow *pMainWindow, LibraryTreeNode *pLibraryTreeNode)
  : QDialog(pMainWindow, Qt::WindowTitleHint)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::exportFigaro));
  mpMainWindow = pMainWindow;
  mpLibraryTreeNode = pLibraryTreeNode;
  // figaro mode
  mpFigaroModeLabel = new Label(tr("Figaro Mode:"));
  mpFigaroModeComboBox = new QComboBox;
  mpFigaroModeComboBox->addItem("figaro0", "figaro0");
  mpFigaroModeComboBox->addItem("fault-tree", "fault-tree");
  // working directory
  mpWorkingDirectoryLabel = new Label(Helper::workingDirectory);
  mpWorkingDirectoryTextBox = new QLineEdit(OpenModelica::tempDirectory());
  mpWorkingDirectoryBrowseButton = new QPushButton(Helper::browse);
  connect(mpWorkingDirectoryBrowseButton, SIGNAL(clicked()), SLOT(browseWorkingDirectory()));
  // create the export button
  mpExportFigaroButton = new QPushButton(Helper::exportFigaro);
  mpExportFigaroButton->setAutoDefault(true);
  connect(mpExportFigaroButton, SIGNAL(clicked()), SLOT(exportModelFigaro()));
  // create the cancel button
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), SLOT(reject()));
  // create buttons box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpExportFigaroButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // layout
  QGridLayout *pMainGridLayout = new QGridLayout;
  pMainGridLayout->setAlignment(Qt::AlignTop);
  pMainGridLayout->addWidget(mpFigaroModeLabel, 0, 0);
  pMainGridLayout->addWidget(mpFigaroModeComboBox, 0, 1, 1, 2);
  pMainGridLayout->addWidget(mpWorkingDirectoryLabel, 1, 0);
  pMainGridLayout->addWidget(mpWorkingDirectoryTextBox, 1, 1);
  pMainGridLayout->addWidget(mpWorkingDirectoryBrowseButton, 1, 2);
  pMainGridLayout->addWidget(mpButtonBox, 2, 0, 1, 3, Qt::AlignRight);
  setLayout(pMainGridLayout);
}

void ExportFigaroDialog::browseWorkingDirectory()
{
  mpWorkingDirectoryTextBox->setText(StringHandler::getExistingDirectory(this, QString(Helper::applicationName).append(" - ").append(Helper::chooseDirectory), NULL));
}

void ExportFigaroDialog::exportModelFigaro()
{
  // set the status message.
  mpMainWindow->getStatusBar()->showMessage(tr("Exporting model as Figaro"));
  // show the progress bar
  mpMainWindow->getProgressBar()->setRange(0, 0);
  mpMainWindow->showProgressBar();
  FigaroPage *pFigaroPage = mpMainWindow->getOptionsDialog()->getFigaroPage();
  QString directory = mpWorkingDirectoryTextBox->text();
  QString library = pFigaroPage->getFigaroDatabaseFileTextBox()->text();
  QString mode = mpFigaroModeComboBox->currentText();
  QString options = pFigaroPage->getFigaroOptionsTextBox()->text();
  QString processor = pFigaroPage->getFigaroProcessTextBox()->text();
  if (mpMainWindow->getOMCProxy()->exportToFigaro(mpLibraryTreeNode->getNameStructure(), directory, library, mode, options, processor)) {
    mpMainWindow->getMessagesWidget()->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                                                 GUIMessages::getMessage(GUIMessages::FIGARO_GENERATED),
                                                                 Helper::scriptingKind, Helper::notificationLevel));
  }
  // hide progress bar
  mpMainWindow->hideProgressBar();
  // clear the status bar message
  mpMainWindow->getStatusBar()->clearMessage();
  accept();
}
