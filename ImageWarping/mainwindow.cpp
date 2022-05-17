#include "mainwindow.h"
#include <QtWidgets>
#include <QImage>
#include <QPainter>
#include "ImageWidget.h"


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	setGeometry(300, 150, 800, 450);

	imagewidget_ = new ImageWidget();
	setCentralWidget(imagewidget_);

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent* e)
{

}

void MainWindow::paintEvent(QPaintEvent* paintevent)
{

}

void MainWindow::CreateActions()
{
	
	action_open_ = new QAction(QIcon("./images/open.jpg"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, &QAction::triggered, imagewidget_, &ImageWidget::Open);

	action_save_ = new QAction(QIcon("./images/save.jpg"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
	connect(action_save_, &QAction::triggered, imagewidget_, &ImageWidget::Save);
	// connect ...

	action_saveas_ = new QAction(tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, &QAction::triggered, imagewidget_, &ImageWidget::SaveAs);

	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, &QAction::triggered, imagewidget_, &ImageWidget::Restore);

	action_addPoints_ = new QAction(tr("add Points"), this);
	action_addPoints_->setStatusTip(tr("add Point pairs"));
	connect(action_addPoints_, &QAction::triggered, imagewidget_, &ImageWidget::AddPointPairs);

	action_IDW_ = new QAction(tr("IDW method"), this);
	action_IDW_->setStatusTip(tr("Image Warping by IDW"));
	connect(action_IDW_, &QAction::triggered, imagewidget_, &ImageWidget::IDWMethod);

	action_RBF_ = new QAction(tr("RBF method"), this);
	action_RBF_->setStatusTip(tr("Image Warping by RBF"));
	connect(action_RBF_, &QAction::triggered, imagewidget_, &ImageWidget::RBFMethod);
}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);

	menu_edit_ = menuBar()->addMenu(tr("&Edit"));
	menu_edit_->setStatusTip(tr("Edit menu"));
	menu_edit_->addAction(action_addPoints_);
	menu_edit_->addAction(action_IDW_);
	menu_edit_->addAction(action_RBF_);
	menu_edit_->addAction(action_restore_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);

	// Add separator in toolbar 
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_addPoints_);
	toolbar_file_->addAction(action_IDW_);
	toolbar_file_->addAction(action_RBF_);
	toolbar_file_->addAction(action_restore_);
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}
