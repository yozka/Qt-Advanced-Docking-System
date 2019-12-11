
/*******************************************************************************
** Qt Advanced Docking System
** Copyright (C) 2017 Uwe Kindler
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/


//============================================================================
/// \file   MainWindow.cpp
/// \author Uwe Kindler
/// \date   13.02.2018
/// \brief  Implementation of CMainWindow demo class
//============================================================================


//============================================================================
//                                   INCLUDES
//============================================================================
#include <MainWindow.h>
#include "ui_mainwindow.h"

#include <iostream>

#include <QTime>
#include <QLabel>
#include <QTextEdit>
#include <QCalendarWidget>
#include <QFrame>
#include <QTreeView>
#include <QFileSystemModel>
#include <QBoxLayout>
#include <QSettings>
#include <QDockWidget>
#include <QDebug>
#include <QResizeEvent>
#include <QAction>
#include <QWidgetAction>
#include <QComboBox>
#include <QInputDialog>
#include <QRubberBand>
#include <QPlainTextEdit>
#include <QTableWidget>

#include <QMap>
#include <QElapsedTimer>

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "FloatingDockContainer.h"


//============================================================================
static ads::CDockWidget* createLongTextLabelDockWidget(QMenu* ViewMenu)
{
	static int LabelCount = 0;
	QLabel* l = new QLabel();
	l->setWordWrap(true);
	l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	l->setText(QString("Label %1 %2 - Lorem ipsum dolor sit amet, consectetuer adipiscing elit. "
		"Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque "
		"penatibus et magnis dis parturient montes, nascetur ridiculus mus. "
		"Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. "
		"Nulla consequat massa quis enim. Donec pede justo, fringilla vel, "
		"aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, "
		"imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede "
		"mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum "
		"semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, "
		"porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, "
		"dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla "
		"ut metus varius laoreet.")
		.arg(LabelCount)
		.arg(QTime::currentTime().toString("hh:mm:ss:zzz")));

	ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Label %1").arg(LabelCount++));
	DockWidget->setWidget(l);
	ViewMenu->addAction(DockWidget->toggleViewAction());
	return DockWidget;
}


/**
 * Function returns a features string with closable (c), movable (m) and floatable (f)
 * features. i.e. The following string is for a not closable but movable and floatable
 * widget: c- m+ f+
 */
static QString featuresString(ads::CDockWidget* DockWidget)
{
	auto f = DockWidget->features();
	return QString("c%1 m%2 f%3")
		.arg(f.testFlag(ads::CDockWidget::DockWidgetClosable) ? "+" : "-")
		.arg(f.testFlag(ads::CDockWidget::DockWidgetMovable) ? "+" : "-")
		.arg(f.testFlag(ads::CDockWidget::DockWidgetFloatable) ? "+" : "-");
}


/**
 * Appends the string returned by featuresString() to the window title of
 * the given DockWidget
 */
static void appendFeaturStringToWindowTitle(ads::CDockWidget* DockWidget)
{
	DockWidget->setWindowTitle(DockWidget->windowTitle()
		+  QString(" (%1)").arg(featuresString(DockWidget)));
}

/**
 * Helper function to create an SVG icon
 */
static QIcon svgIcon(const QString& File)
{
	// This is a workaround, because because in item views SVG icons are not
	// properly scaled an look blurry or pixelate
	QIcon SvgIcon(File);
	SvgIcon.addPixmap(SvgIcon.pixmap(92));
	return SvgIcon;
}


//============================================================================
static ads::CDockWidget* createCalendarDockWidget(QMenu* ViewMenu)
{
	static int CalendarCount = 0;
	QCalendarWidget* w = new QCalendarWidget();
	ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Calendar %1").arg(CalendarCount++));
	DockWidget->setWidget(w);
	DockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	DockWidget->setIcon(svgIcon(":/adsdemo/images/date_range.svg"));
	ViewMenu->addAction(DockWidget->toggleViewAction());
	return DockWidget;
}


//============================================================================
static ads::CDockWidget* createFileSystemTreeDockWidget(QMenu* ViewMenu)
{
	static int FileSystemCount = 0;
	QTreeView* w = new QTreeView();
	w->setFrameShape(QFrame::NoFrame);
	QFileSystemModel* m = new QFileSystemModel(w);
	m->setRootPath(QDir::currentPath());
	w->setModel(m);
	ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Filesystem %1")
		.arg(FileSystemCount++));
	DockWidget->setWidget(w);
	ViewMenu->addAction(DockWidget->toggleViewAction());
    return DockWidget;
}

//============================================================================
static ads::CDockWidget* createEditorWidget(QMenu* ViewMenu)
{
	static int EditorCount = 0;
	QPlainTextEdit* w = new QPlainTextEdit();
	w->setPlaceholderText("This is an editor. If you close the editor, it will be "
		"deleted. Enter your text here.");
	w->setStyleSheet("border: none");
	ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Editor %1").arg(EditorCount++));
	DockWidget->setWidget(w);
	DockWidget->setIcon(svgIcon(":/adsdemo/images/edit.svg"));
	ViewMenu->addAction(DockWidget->toggleViewAction());
	return DockWidget;
}

//============================================================================
static ads::CDockWidget* createTableWidget(QMenu* ViewMenu)
{
   static int TableCount = 0;
   QTableWidget* w = new QTableWidget();
   ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Table %1").arg(TableCount++));
   static int colCount = 5;
   static int rowCount = 30;
   w->setColumnCount(colCount);
   w->setRowCount(rowCount);
   for (int col = 0; col < colCount; ++col)
   {
      w->setHorizontalHeaderItem(col, new QTableWidgetItem(QString("Col %1").arg(col+1)));
      for (int row = 0; row < rowCount; ++row)
      {
         w->setItem(row, col, new QTableWidgetItem(QString("T %1-%2").arg(row + 1).arg(col+1)));
      }
   }
   DockWidget->setWidget(w);
   DockWidget->setIcon(svgIcon(":/adsdemo/images/grid_on.svg"));
   ViewMenu->addAction(DockWidget->toggleViewAction());
   return DockWidget;
}


//============================================================================
/**
 * Private data class pimpl
 */
struct MainWindowPrivate
{
	CMainWindow* _this;
	Ui::MainWindow ui;
	QAction* SavePerspectiveAction = nullptr;
	QWidgetAction* PerspectiveListAction = nullptr;
	QComboBox* PerspectiveComboBox = nullptr;
	ads::CDockManager* DockManager = nullptr;

	MainWindowPrivate(CMainWindow* _public) : _this(_public) {}

	/**
	 * Creates the toolbar actions
	 */
	void createActions();

	/**
	 * Fill the dock manager with dock widgets
	 */
	void createContent();

	/**
	 * Saves the dock manager state and the main window geometry
	 */
	void saveState();

	/**
	 * Save the list of perspectives
	 */
	void savePerspectives();

	/**
	 * Restores the dock manager state
	 */
	void restoreState();

	/**
	 * Restore the perspective listo of the dock manager
	 */
	void restorePerspectives();
};


//============================================================================
void MainWindowPrivate::createContent()
{
	// Test container docking
	QMenu* ViewMenu = ui.menuView;
	auto DockWidget = createCalendarDockWidget(ViewMenu);
	DockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
	DockManager->addDockWidget(ads::LeftDockWidgetArea, DockWidget);
	DockManager->addDockWidget(ads::LeftDockWidgetArea, createLongTextLabelDockWidget(ViewMenu));
	auto FileSystemWidget = createFileSystemTreeDockWidget(ViewMenu);
	auto ToolBar = FileSystemWidget->createDefaultToolBar();
	ToolBar->addAction(ui.actionSaveState);
	ToolBar->addAction(ui.actionRestoreState);
	DockManager->addDockWidget(ads::BottomDockWidgetArea, FileSystemWidget);

	FileSystemWidget = createFileSystemTreeDockWidget(ViewMenu);
	ToolBar = FileSystemWidget->createDefaultToolBar();
	ToolBar->addAction(ui.actionSaveState);
	ToolBar->addAction(ui.actionRestoreState);
	FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
	FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
	appendFeaturStringToWindowTitle(FileSystemWidget);
	auto TopDockArea = DockManager->addDockWidget(ads::TopDockWidgetArea, FileSystemWidget);
	DockWidget = createCalendarDockWidget(ViewMenu);
	DockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
	DockWidget->setTabToolTip(QString("Tab ToolTip\nHodie est dies magna"));
	DockManager->addDockWidget(ads::CenterDockWidgetArea, DockWidget, TopDockArea);

	// Test dock area docking
	auto RighDockArea = DockManager->addDockWidget(ads::RightDockWidgetArea, createLongTextLabelDockWidget(ViewMenu), TopDockArea);
	DockManager->addDockWidget(ads::TopDockWidgetArea, createLongTextLabelDockWidget(ViewMenu), RighDockArea);
	auto BottomDockArea = DockManager->addDockWidget(ads::BottomDockWidgetArea, createLongTextLabelDockWidget(ViewMenu), RighDockArea);
	DockManager->addDockWidget(ads::RightDockWidgetArea, createLongTextLabelDockWidget(ViewMenu), RighDockArea);
	DockManager->addDockWidget(ads::CenterDockWidgetArea, createLongTextLabelDockWidget(ViewMenu), BottomDockArea);

    auto Action = ui.menuView->addAction(QString("Set %1 floating").arg(DockWidget->windowTitle()));
    DockWidget->connect(Action, SIGNAL(triggered()), SLOT(setFloating()));

	for (auto DockWidget : DockManager->dockWidgetsMap())
	{
		_this->connect(DockWidget, SIGNAL(viewToggled(bool)), SLOT(onViewToggled(bool)));
	}
}


//============================================================================
void MainWindowPrivate::createActions()
{
	ui.toolBar->addAction(ui.actionSaveState);
	ui.toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.actionSaveState->setIcon(svgIcon(":/adsdemo/images/save.svg"));
	ui.toolBar->addAction(ui.actionRestoreState);
	ui.actionRestoreState->setIcon(svgIcon(":/adsdemo/images/restore.svg"));

	SavePerspectiveAction = new QAction("Create Perspective", _this);
	SavePerspectiveAction->setIcon(svgIcon(":/adsdemo/images/picture_in_picture.svg"));
	_this->connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(savePerspective()));
	PerspectiveListAction = new QWidgetAction(_this);
	PerspectiveComboBox = new QComboBox(_this);
	PerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	PerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	PerspectiveListAction->setDefaultWidget(PerspectiveComboBox);
	ui.toolBar->addSeparator();
	ui.toolBar->addAction(PerspectiveListAction);
	ui.toolBar->addAction(SavePerspectiveAction);

	QAction* a = ui.toolBar->addAction("Create Editor");
	a->setToolTip("Creates floating dynamic dockable editor windows that are deleted on close");
	a->setIcon(svgIcon(":/adsdemo/images/note_add.svg"));
	_this->connect(a, SIGNAL(triggered()), SLOT(createEditor()));

	a = ui.toolBar->addAction("Create Table");
	a->setToolTip("Creates floating dynamic dockable table with millions of entries");
	a->setIcon(svgIcon(":/adsdemo/images/grid_on.svg"));
	_this->connect(a, SIGNAL(triggered()), SLOT(createTable()));
}


//============================================================================
void MainWindowPrivate::saveState()
{
	QSettings Settings("Settings.ini", QSettings::IniFormat);
	Settings.setValue("mainWindow/Geometry", _this->saveGeometry());
	Settings.setValue("mainWindow/State", _this->saveState());
	Settings.setValue("mainWindow/DockingState", DockManager->saveState());
}


//============================================================================
void MainWindowPrivate::restoreState()
{
	QSettings Settings("Settings.ini", QSettings::IniFormat);
	_this->restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
	_this->restoreState(Settings.value("mainWindow/State").toByteArray());
	DockManager->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
}



//============================================================================
void MainWindowPrivate::savePerspectives()
{
	QSettings Settings("Settings.ini", QSettings::IniFormat);
	DockManager->savePerspectives(Settings);
}



//============================================================================
void MainWindowPrivate::restorePerspectives()
{
	QSettings Settings("Settings.ini", QSettings::IniFormat);
	DockManager->loadPerspectives(Settings);
	PerspectiveComboBox->clear();
	PerspectiveComboBox->addItems(DockManager->perspectiveNames());
}


//============================================================================
CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	d(new MainWindowPrivate(this))
{
	using namespace ads;
	d->ui.setupUi(this);
	d->createActions();

	// uncomment the following line if the tab close button should be
	// a QToolButton instead of a QPushButton
	// CDockManager::setConfigFlags(CDockManager::configFlags() | CDockManager::TabCloseButtonIsToolButton);

    // uncomment the following line if you want a fixed tab width that does
	// not change if the visibility of the close button changes
    // CDockManager::setConfigFlag(CDockManager::RetainTabSizeWhenCloseButtonHidden, true);

    // uncomment the follwing line if you want to use non opaque undocking and splitter
    // moevements
    // CDockManager::setConfigFlags(CDockManager::DefaultNonOpaqueConfig);

	// Now create the dock manager and its content
	d->DockManager = new CDockManager(this);

	// Uncomment the following line to have the old style where the dock
	// area close button closes the active tab
	// CDockManager::setConfigFlags({CDockManager::DockAreaHasCloseButton
	//	| CDockManager::DockAreaCloseButtonClosesTab});
	connect(d->PerspectiveComboBox, SIGNAL(activated(const QString&)),
		d->DockManager, SLOT(openPerspective(const QString&)));

	d->createContent();
	// Default window geometry
    resize(1280, 720);

	//d->restoreState();
	d->restorePerspectives();
}


//============================================================================
CMainWindow::~CMainWindow()
{
	delete d;
}


//============================================================================
void CMainWindow::closeEvent(QCloseEvent* event)
{
	d->saveState();
	QMainWindow::closeEvent(event);
}


//============================================================================
void CMainWindow::on_actionSaveState_triggered(bool)
{
	qDebug() << "MainWindow::on_actionSaveState_triggered";
	d->saveState();
}


//============================================================================
void CMainWindow::on_actionRestoreState_triggered(bool)
{
	qDebug() << "MainWindow::on_actionRestoreState_triggered";
	d->restoreState();
}


//============================================================================
void CMainWindow::savePerspective()
{
	QString PerspectiveName = QInputDialog::getText(this, "Save Perspective", "Enter unique name:");
	if (PerspectiveName.isEmpty())
	{
		return;
	}

	d->DockManager->addPerspective(PerspectiveName);
	QSignalBlocker Blocker(d->PerspectiveComboBox);
	d->PerspectiveComboBox->clear();
	d->PerspectiveComboBox->addItems(d->DockManager->perspectiveNames());
	d->PerspectiveComboBox->setCurrentText(PerspectiveName);

	d->savePerspectives();
}


//============================================================================
void CMainWindow::onViewToggled(bool Open)
{
	auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
	if (!DockWidget)
	{
		return;
	}

	qDebug() << DockWidget->objectName() << " viewToggled(" << Open << ")";
}


//============================================================================
void CMainWindow::createEditor()
{
	auto DockWidget = createEditorWidget(d->ui.menuView);
	DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
	auto FloatingWidget = d->DockManager->addDockWidgetFloating(DockWidget);
    FloatingWidget->move(QPoint(20, 20));
}


//============================================================================
void CMainWindow::createTable()
{
	auto DockWidget = createTableWidget(d->ui.menuView);
	DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
	auto FloatingWidget = d->DockManager->addDockWidgetFloating(DockWidget);
    FloatingWidget->move(QPoint(40, 40));
}

