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
//#define PART_DEBUG() kWarning()

PartWin::PartWin(QWidget *parent)
  : KParts::MainWindow(),  m_progressBarInited(false), m_guiInitialized(false)
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
	
	m_progressWidget = new ProgressWidget(this->parentWidget());
	setCentralWidget(m_progressWidget);
	
	m_progressWidget->setAttribute(Qt::WA_DeleteOnClose);
	m_guiInitialized = true;
      }
      
  } else {
    // couldn't load anything up
    KMessageBox::error(this, "service okular_part.desktop not found");
    qApp->quit();
    return;
  }
}

void PartWin::setupPart() {
  
	// make part window the main widget
	setCentralWidget(m_part->widget());
	
	setupActions();

	setupGUI(ToolBar | Keys | Save);
	toolBar("okularToolBar")->setToolButtonStyle(Qt::ToolButtonIconOnly);
	
	// integrate the part's GUI with the shell's
	createGUI(m_part);
	
	toolBar("okularToolBar")->insertAction(toolBar("okularToolBar")->actions().front(), this->actionCollection()->action("file_print"));
	menuBar()->clear();
	
	toolBar("okularToolBar")->show();
	
}

void PartWin::setupActions() {
  // make a print action, as we don't get this by
  // default in the okular kpart
  m_printAction = KStandardAction::print(m_part, SLOT( slotPrint() ), actionCollection());
}

PartWin::~PartWin()
{
  
  this->guiFactory()->removeClient(m_part);
  this->guiFactory()->removeClient(this);
  
  delete m_part;

  QDir d;
  for( QList<QString>::const_iterator i = m_filesToDelete.begin(); i != m_filesToDelete.end(); ++i ) {
      d.remove(*i);
  }  
}

/** 
 * This method is called from qtbrowserplugin when the download 
 * is already finished.
 * 
 */
bool PartWin::readData(QIODevice *source, const QString &format)
{
  
  // NPRES_NETWORK_ERR:
  if (source->errorString() == "Network error during download.") {
    // this error message is also set when user aborts download before it finished.
    m_progressWidget->resetProgressBar();
    m_progressWidget->setErrorMessage("Error occurred during download. Reload tab to retry.");
    return false;
  }

  // NPRES_USER_BREAK:
  if (source->errorString() == "User cancelled operation.") {
    m_progressWidget->resetProgressBar();
    m_progressWidget->setErrorMessage("Error occurred during download. Reload tab to retry.");    
    return false;
  }  
  
  // Download finished
  m_progressWidget->setValue(m_progressWidget->getMaximum());
  
  QString filetype;
  QFileInfo fileInfo(QUrl(sourceUrl).path());
  QString fileName = QUrl::fromPercentEncoding(fileInfo.fileName().toUtf8());
  QString extension = fileInfo.suffix();
  
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
  else if ( format == "application/vnd.ms-xpsdocument" )
    filetype = ".xps";
  else if ( format == "image/g3fax" )
    filetype = ".fax";
  else if ( format == "text/fb2+xml" )
    filetype = ".fb2";
  else if ( format == "application/vnd.palm" )
    filetype = ".pdb";
  else if ( format == "application/x-cbr" ) {
    if(extension == "cba") 
      filetype = ".cba";
    else if(extension == "cbt")
      filetype = ".cbt";
    else if(extension == "cbz")
      filetype = ".cbz";
    else if(extension == "cb7")
      filetype = ".cb7";
    else 
      filetype = ".cbr";
  } else {
    filetype= ".pdf";
  }

  
  if (fileName.endsWith(extension, Qt::CaseInsensitive)) {
    // foo.pdf -> foo - pdf - .
    fileName = fileName.left(fileName.length() - extension.length() - ((extension.length() > 0) ? 1 : 0));
  }

  QTemporaryFile tmpFile("/tmp/" + fileName + "_XXXXXX" + filetype);
  tmpFile.setAutoRemove(false);
  
  if (!source->open(QIODevice::ReadOnly))
    return false;  

  if(tmpFile.open()) {
      while( ! source->atEnd() ) {
  	QByteArray data = source->read(1024 * 1024 * 1024);
  	tmpFile.write(data);
      }
      tmpFile.flush();
  }
  
  m_filesToDelete.push_back(tmpFile.fileName());  
  this->setupPart();
  
  QString url = QString("file://") + tmpFile.fileName();
  m_part->openUrl(url);

  return true;
}

void PartWin::readProgress(int lenRead, int size) {
    if (!m_progressBarInited) { 
	m_progressWidget->setMaximum(size);
	m_progressWidget->setMinimum(0);

	m_progressBarInited = true;
    }
    m_progressWidget->setValue(m_progressWidget->getValue() + lenRead);
}

void PartWin::setDataSourceUrl(const QString &url)
{
  sourceUrl = url;
  
  QFileInfo fileInfo(QUrl(sourceUrl).path());
  QString fileName = fileInfo.fileName();
  m_progressWidget->setFileName(fileName);
  
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
