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
#include <cstdio>

#include <kservice.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include "partwin.h"

PartWin::PartWin(QWidget *parent)
  : KParts::MainWindow()
{
  setXMLFile ("kparttut1ui.rc");

  if( parent != 0 )
    setParent(parent);
  setFocusPolicy(Qt::StrongFocus);
  QApplication::setActiveWindow(this);

  //query the .desktop file to load the requested Part
  KService::Ptr service = KService::serviceByDesktopPath("okular_part.desktop");
  if (service)
    {
      m_part = service->createInstance<KParts::ReadOnlyPart>(0);
      
      if (m_part)
	{
	  // tell the KParts::MainWindow that this is indeed
	  // the main widget
	  setCentralWidget(m_part->widget());
	  
	  
	  m_printAction =
	    KStandardAction::print(m_part, SLOT( slotPrint() ), actionCollection() );
	  connect( m_part, SIGNAL( enablePrintAction(bool) ), m_printAction, SLOT( setEnabled(bool)));
	  
	  
	  setupGUI(ToolBar | Keys | StatusBar | Save);
	  
	  
	  
	  // and integrate the part's GUI with the shell's
	  createGUI(m_part);
	  
	}
      else
	{
          return;
	}
    }
  else
    {
      // if we couldn't find our Part, we exit since the Shell by
      // itself can't do anything useful
      KMessageBox::error(this, "service katepart.desktop not found");
      qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
      return;
    }
  
  activeTimer = new QTimer(this);
  connect( activeTimer, SIGNAL( timeout() ), this,
	   SLOT( slotActiveTimeOut() ) );
  activeTimer->start(1000);
  

}

PartWin::~PartWin()
{
}

bool PartWin::readData(QIODevice *source, const QString &/*format*/)
{

  QTemporaryFile file("/tmp/pdf_kpart_XXXXXX.pdf");
  //file.setAutoRemove(false);

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
  QString url = QString("file://") + file.fileName();
  m_part->openUrl( url );

  // m_part->openUrl(source->objectName());

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

void PartWin::slotActiveTimeOut()
{
  QObject *w =  QApplication::activeWindow();

  if ( w == 0 )
    {
      QApplication::setActiveWindow(this);
      return;
    }
}

QTNPFACTORY_BEGIN("KPart PDF plugin",
		  "NSAPI KPart PDF plugin");
    QTNPCLASS(PartWin)
QTNPFACTORY_END()
