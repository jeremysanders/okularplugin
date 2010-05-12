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

#include <QWidget>
#include <QString>
#include <QIODevice>
#include <QMetaClassInfo>

#include <kaction.h>
#include <kstdaction.h>
#include <QTimer>

#include <kparts/mainwindow.h>
#include <qtbrowserplugin.h>

class PartWin : public KParts::MainWindow, QtNPBindable
{
  Q_OBJECT

  Q_PROPERTY(QString src READ dataSourceUrl WRITE setDataSourceUrl)
    
  Q_CLASSINFO("MIME", "application/pdf:pdf:PDF document")

public:
    PartWin(QWidget *parent = 0);
    ~PartWin();

    void setDataSourceUrl(const QString &url);
    QString dataSourceUrl() const;

    bool readData(QIODevice *source, const QString &format);

    void transferComplete(const QString &url, int id, Reason r);

protected:
    void enterEvent(QEvent *event);

private:
    KParts::ReadOnlyPart *m_part;

    QString sourceUrl;
    int lastReqId, lastConfId;
    QString lastConfUrl;
    Reason lastConfReason;

    KAction* m_printAction;
};
