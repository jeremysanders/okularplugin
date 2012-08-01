// okular firefox plugin
// Copyright (C) 2010 Jeremy Sanders

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA

#include <QtGui>

#include <kservice.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <KDebug>
#include <KMenuBar>
#include <KToolBar>
#include <QToolButton>

#include "partwin.h"
#include <KStandardDirs>
#include <KXMLGUIFactory>


#define PART_DEBUG() kDebugDevNull()

PartWin::PartWin(QWidget *parent)
  : KParts::MainWindow(), m_guiInitialized(false)
{
  PART_DEBUG() << " me == " << this;
  QString dataDir = KStandardDirs::locate("data", "okularplugin/");
  
  setXMLFile("okularplugin/okularpluginui.rc", false);
  
  
  if (! KStandardDirs::exists(dataDir)) {
    KStandardDirs::makeDir(dataDir);
  }
  
  // Set the full path to the "local" xml file, the one used for saving toolbar and shortcut changes
  setLocalXMLFile("okularplugin/okularpluginui.rc");
  
//  if( parent != 0 )
//    setParent(parent);

  setFocusPolicy(Qt::StrongFocus);
  //QApplication::setActiveWindow(this);

  // query the .desktop file to load the requested Part
  KService::Ptr service =
    KService::serviceByDesktopPath("okular_part.desktop");

  if (service) {
    
      PART_DEBUG() << " createInstance";
      m_part = service->createInstance<KParts::ReadOnlyPart>(this);

      if (!m_part)
	return;
      
      if (m_part) {
	
	// replace ui definition
	m_part->replaceXMLFile(dataDir + "okularplugin_okularui.rc",
			       "okularplugin/okularplugin_okularui.rc", 
			       false);	
	
	// make part window the main widget
	setCentralWidget(m_part->widget());
	
	setupActions();

	setupGUI(ToolBar | Keys | StatusBar | Save);
	toolBar()->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolBar("okularToolBar")->setToolButtonStyle(Qt::ToolButtonIconOnly);
	

	// integrate the part's GUI with the shell's
	createGUI(m_part);

	menuBar()->clear();
	m_guiInitialized = true;
	//menuBar()->setVisible(false);
      }
      
  } else {
    // couldn't load anything up
    KMessageBox::error(this, "service okular_part.desktop not found");
    qApp->quit();
    return;
  }
}


void PartWin::setupActions() {
  // make a print action, as we don't get this by
  // default in the okular kpart
  m_printAction =
    KStandardAction::print(m_part, SLOT( slotPrint() ), actionCollection());
    connect( m_part, SIGNAL( enablePrintAction(bool) ),
	     m_printAction, SLOT( setEnabled(bool)));
   
    //
    //
    //
    //m_printAction->setParent(m_part->parent());
}

PartWin::~PartWin()
{
  QDir d;
  for( QList<QString>::const_iterator i = toDeleteFiles.begin(); i != toDeleteFiles.end(); ++i ) {
      d.remove(*i);
  }
  
  this->guiFactory()->removeClient(m_part);
  this->guiFactory()->removeClient(this);
  
  delete m_part;
  delete m_printAction;
}

bool PartWin::readData(QIODevice *source, const QString &format)
{
  QString filetype;
  if( format == "application/postscript" )
    filetype = ".ps";
  else if ( format == "application/x-dvi" )
    filetype = ".dvi";
  else if ( format == "image/vnd.djvu" )
    filetype = ".djvu";
  else if ( format == "image/x.djvu" )
    filetype = ".djvu";
  else if ( format == "image/x-djvu" )
    filetype = ".djvu";
  else if ( format == "application/x-chm" )
    filetype = ".chm";
  else if ( format == "application/epub+zip" )
    filetype = ".epub";
  else if ( format == "image/tiff" )
    filetype = ".tiff";
  else if ( format == "image/tiff-fx" )
    filetype = ".tiff";
  else
    filetype= ".pdf";
  
  QFileInfo fileInfo(QUrl(sourceUrl).path());
  QString fileName = fileInfo.fileName();
  
  if (! fileName.endsWith(filetype, Qt::CaseInsensitive)) {
    fileName += filetype;
  }

  QTemporaryFile file("/tmp/okularplugin_XXXXXX_" + fileName);
  file.setAutoRemove(false);

  if (!source->open(QIODevice::ReadOnly))
    return false;

  if( file.open() )
    {
      while( ! source->atEnd() ) {
  	QByteArray data = source->read(102400);
  	file.write(data);
      }
      file.flush();
    }

  toDeleteFiles.push_back( file.fileName() );
  QString url = QString("file://") + file.fileName();
  m_part->openUrl( url );

  return true;
}

void PartWin::setDataSourceUrl(const QString &url)
{
  sourceUrl = url;
}

QString PartWin::dataSourceUrl() const
{
  return sourceUrl;
}

void PartWin::transferComplete(const QString &url, int id, Reason r)
{
  lastConfId = id;
  lastConfUrl = url;
  lastConfReason = r;
}


void PartWin::enterEvent(QEvent *event)
{
  // this is required because firefox stops sending keyboard
  // events to the plugin after opening windows (e.g. download dialog)
  // setting the active window brings the events back
  if ( QApplication::activeWindow() == NULL ) {
      QApplication::setActiveWindow(this);
  }
    
   QWidget::enterEvent(event);
}


// required for signals and slots - generated by Qt
#include "partwin.moc"

QTNPFACTORY_BEGIN("Okular plugin",
		  "Okular plugin using KParts");
QTNPCLASS(PartWin)
QTNPFACTORY_END()
