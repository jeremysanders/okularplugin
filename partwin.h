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

#ifndef PARTWIN_HPP
#define PARTWIN_HPP


#include <QWidget>
#include <QString>
#include <QIODevice>
#include <QMetaClassInfo>
#include <QProgressBar>

#include <kaction.h>
#include <kstdaction.h>
#include <QTimer>

#include <kparts/mainwindow.h>
#include <qtbrowserplugin.h>
#include <progresswidget.h>

class PartWin : public KParts::MainWindow, QtNPBindable
{
  Q_OBJECT

  Q_PROPERTY(QString src READ dataSourceUrl WRITE setDataSourceUrl)

  Q_CLASSINFO("MIME",
	      "application/pdf:pdf:Portable Document Format;"
	      "application/vnd.ms-xpsdocument:xps:Open XML Paper Specification;"
	      "application/epub+zip:epub:EPUB document;"
	      "image/tiff:tif,tiff:Tagged Image File Format;"
	      "image/tiff-fx:tif,tiff:Tagged Image File Format;"
	      "image/x.djvu:djv,djvu:DJVU document;"
	      "image/x-djvu:djv,djvu:DJVU document;"
	      "image/vnd.djvu:djv,djvu:DJVU document;"
	      "application/x-chm:chm:Microsoft Compiled HTML Help;"
	      "application/postscript:ps:PostScript document;"
	      "application/x-dvi:dvi:DVI document"
	      "application/x-cbr:cbr:Comic Book Archive;"
	      "image/g3fax:fax:Fax;"
	      "text/fb2+xml:fb2:Fiction Book;"
	      "application/vnd.palm:pdb:Palm Digital Media;"
	     )

public:
    PartWin(QWidget *parent = 0);
    ~PartWin();

    void setDataSourceUrl(const QString &url);
    QString dataSourceUrl() const;

    bool readData(QIODevice *source, const QString &format);
    
    void readProgress(int lenRead, int size);

    void transferComplete(const QString &url, int id, Reason r);
    
protected:
    void enterEvent(QEvent *event);


private:
    void setupActions();
    
    void setupPart();
    void setupProgressBar();
    
    KParts::ReadOnlyPart *m_part;
    
    ProgressWidget *m_progressWidget;
    bool m_progressBarInited;

    QString sourceUrl;
    int lastReqId, lastConfId;
    QString lastConfUrl;
    Reason lastConfReason;

    KAction* m_printAction;
    bool m_guiInitialized;
    
    QList<QString> toDeleteFiles;
    

};

#endif
