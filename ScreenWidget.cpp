/* $Id: ScreenWidget.cpp,v 1.16 2001/09/12 15:23:17 aspert Exp $ */
#include <ScreenWidget.h>

ScreenWidget::ScreenWidget(model_error *model1, model_error *model2, 
			   QWidget *parent, 
			   const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut, *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *helpMenu;
  QFrame *frameModel1, *frameModel2;
  QHBoxLayout *hLay1, *hLay2;
  QGridLayout *bigGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *colorBar;

  setMinimumSize( 1070, 540 );
  setMaximumSize( 1070, 540 );
  

  fileQuitAction = new QAction( "Quit", "Quit", CTRL+Key_Q, this, "quit" );
  connect(fileQuitAction, SIGNAL(activated()) , 
	  qApp, SLOT(closeAllWindows()));


  // Create the 'File' menu
  mainBar = new QMenuBar(this);
  fileMenu = new QPopupMenu(this);
  mainBar->insertItem("&File",fileMenu);
  fileMenu->insertSeparator();
  fileQuitAction->addTo(fileMenu);

  //Create the 'Help' menu
  helpMenu = new QPopupMenu(this);
  mainBar->insertItem("&Help", helpMenu);
  helpMenu->insertSeparator();
  helpMenu->insertItem("&Key utilities", this, SLOT(aboutKeys()),CTRL+Key_H);
  helpMenu->insertItem("&Bug", this, SLOT(aboutBugs()));

  // --------------
  // Create the GUI
  // --------------

  // Create frames to put around the OpenGL widgets
  frameModel1 = new QFrame(this, "frameModel1");
  frameModel1->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel1->setLineWidth(2);

  frameModel2 = new QFrame(this, "frameModel2");
  frameModel2->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel2->setLineWidth(2);


  // Create the colorbar and the 2 GL windows.
  glModel1 = new RawWidget(model1, RW_COLOR, frameModel1, "glModel1");
  glModel1->setFocusPolicy(StrongFocus);
  glModel2 = new RawWidget(model2, RW_LIGHT_TOGGLE, frameModel2, "glModel2");
  glModel2->setFocusPolicy(StrongFocus);
  colorBar = new ColorMapWidget(model1->min_verror,
                                model1->max_verror, this, "colorBar");

  // Put the 1st GL widget inside the frame
  hLay1 = new QHBoxLayout(frameModel1, 2, 2, "hLay1");
  hLay1->addWidget(glModel1, 1);
  // Put the 2nd GL widget inside the frame
  hLay2 = new QHBoxLayout(frameModel2, 2, 2, "hLay2");
  hLay2->addWidget(glModel2, 1);

  // This is to synchronize the viewpoints of the two models
  // We need to pass the viewing matrix from one RawWidget
  // to another
  connect(glModel1, SIGNAL(transfervalue(double,double*)), 
	  glModel2, SLOT(transfer(double,double*)));
  connect(glModel2, SIGNAL(transfervalue(double,double*)), 
	  glModel1, SLOT(transfer(double,double*)));


  // Build synchro and quit buttons
  syncBut = new QPushButton("Synchronize viewpoints", this);
  syncBut->setMinimumSize(40, 30);
  syncBut->setToggleButton(TRUE);

  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(switchSync(bool))); 
  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(switchSync(bool)));
  connect(glModel1, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));
  connect(glModel2, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));

  quitBut = new QPushButton("Quit", this);
  quitBut->setMinimumSize(20, 30);
  

  // Build the two line/fill toggle buttons
  lineSwitch1 = new QPushButton("Line/Fill", this);
  lineSwitch1->setToggleButton(TRUE);

  connect(lineSwitch1, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(setLine(bool)));

  lineSwitch2 = new QPushButton("Line/Fill", this);
  lineSwitch2->setToggleButton(TRUE);

  connect(lineSwitch2, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(setLine(bool)));


  // Build the topmost grid layout
  bigGrid = new QGridLayout (this, 5, 3, 5);
  bigGrid->addWidget(colorBar, 1, 0);
  bigGrid->addWidget(frameModel1, 1, 1);
  bigGrid->addWidget(frameModel2, 1, 2);
  bigGrid->addWidget(lineSwitch1, 2, 1, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 2, 2, Qt::AlignCenter);
  bigGrid->addWidget(syncBut, 3, 1, Qt::AlignCenter);
  bigGrid->addWidget(quitBut, 4, 1, Qt::AlignCenter);
  

}

void ScreenWidget::aboutKeys()
{
    QMessageBox::about( this, "Key bindings",
			"F1: Toggle Wireframe/Fill\n"
			"F2: Toggle Light/No light (right model only)\n"
			"F3: Toggle viewpoint synchronization\n"
			"F4: Invert normals (if applicable)");
}

void ScreenWidget::aboutBugs()
{
    QMessageBox::about( this, "Bug",
			"If you found a bug, please send an e-mail to :\n"
			"Nicolas.Aspert@epfl.ch or\n"
			"Diego.SantaCruz@epfl.ch");
}
