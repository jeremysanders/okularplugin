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

PartWin::PartWin(QWidget *parent)
  : KParts::MainWindow(), m_guiInitialized(false)
{
  
  kWarning() << " me == " << this;
  QString dataDir = KStandardDirs::locate("data", "okularplugin/");
  
  kWarning() << " setXMLFile ";
  setXMLFile("okularplugin/okularpluginui.rc", false);
  
  
  if (! KStandardDirs::exists(dataDir)) {
    KStandardDirs::makeDir(dataDir);
  }
  
  // Set the full path to the "local" xml file, the one used for saving toolbar and shortcut changes
  kWarning() << " setLocalXMLFile ";
  setLocalXMLFile("okularplugin/okularpluginui.rc");
  
//  if( parent != 0 )
//    setParent(parent);

  kWarning() << " setFocusPolicy";
  setFocusPolicy(Qt::StrongFocus);
  //QApplication::setActiveWindow(this);

  // query the .desktop file to load the requested Part
  KService::Ptr service =
    KService::serviceByDesktopPath("okular_part.desktop");

  if (service) {
    
      kWarning() << " createInstance";
      m_part = service->createInstance<KParts::ReadOnlyPart>(this);

      if (!m_part)
	return;
      
      if (m_part) {
	
	// replace ui definition
	kWarning() << " replaceXML";
	m_part->replaceXMLFile(dataDir + "okularplugin_okularui.rc",
			       "okularplugin/okularplugin_okularui.rc", 
			       false);	
	
	// make part window the main widget
	kWarning() << " setCentralWidget";
	setCentralWidget(m_part->widget());
	
	kWarning() << " setupActions";
	setupActions();

	kWarning() << " setupGUI";
	setupGUI(ToolBar | Keys | StatusBar | Save);
	toolBar("okularToolBar")->setToolButtonStyle(Qt::ToolButtonIconOnly);

	// integrate the part's GUI with the shell's
	kWarning() << " createGUI";
	createGUI(m_part);

	kWarning() << " clear";
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
  kWarning() << " setupPrintAction";
  m_printAction =
    KStandardAction::print(m_part, SLOT( slotPrint() ), actionCollection());
    connect( m_part, SIGNAL( enablePrintAction(bool) ),
	     m_printAction, SLOT( setEnabled(bool)));
   
    //
    //
    //
    m_printAction->setParent(m_part->parent());
   //m_part->widget()->window()->setVisible(false);
}

PartWin::~PartWin()
{
  QDir d;
  for( QList<QString>::const_iterator i = toDeleteFiles.begin();
       i != toDeleteFiles.end(); ++i )
    {
      d.remove(*i);
    }
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

  kWarning() << " source->open";
  if (!source->open(QIODevice::ReadOnly))
    return false;

  if( file.open() )
    {
      while( ! source->atEnd() ) {
  	QByteArray data = source->read(102400);
  	file.write(data);
      }
      kWarning() << " file.flush()";
      file.flush();
    }

  toDeleteFiles.push_back( file.fileName() );
  QString url = QString("file://") + file.fileName();
  kWarning() << " openUrl";
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
  kWarning() << " transferComplete";
}


void PartWin::enterEvent(QEvent *event)
{
 
  
  //kWarning() << "enterEvent... eventType = " << event->type();
  
  //if (event->type() == QEvent::ContextMenu) {
  //  kWarning() << "Event == ContextMenu";
  //}
  // this is required because firefox stops sending keyboard
  // events to the plugin after opening windows (e.g. download dialog)
  // setting the active window brings the events back
  if ( QApplication::activeWindow() == NULL )
    {
      kWarning() << "setting active Window to this...";
      QApplication::setActiveWindow(this);
    }
    
    //kWarning() << "height focusWidget == " << focusWidget()->frameSize().height();
  //m_part->setParent(this);
  //  this->setCentralWidget(m_part->widget());
  //m_part->widget()->activateWindow();
//QWidget::enterEvent(event);
  //KParts::MainWindow::enterEvent(event);
  
}

QTNPFACTORY_BEGIN("Okular plugin",
		  "Okular plugin using KParts");
QTNPCLASS(PartWin)
QTNPFACTORY_END()
