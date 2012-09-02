/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include "ui_progresswidget.h"

#include <QtGui>
#include <QProgressBar>
#include <qtimer.h>
#include <QTimer>

class ProgressWidget : public QWidget
{

  Q_OBJECT
  
public:
    ProgressWidget(QWidget *parent);
    ~ProgressWidget();
    
    void setFileName(const QString &fileName);
    
    void setMinimum(int size);
    void setMaximum(int size);
    int getMaximum();
    
    void setValue(int size);
    int getValue();
    
public slots:
    void updateDownloadSpeed();
  
private:
  Ui::ProgressWidget *m_ui;
  QString m_fileName;
  QTimer *m_timer;
  
  qint64 m_lastTimestamp;
  qint64 m_lastIntervalSize;
  
};

#endif // PROGRESSWIDGET_H
