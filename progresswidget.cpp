/*
    Copyright (C) 2012  André Frimberger okularplugin@frimberger.de

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


#include "progresswidget.h"
#include <KDebug>

ProgressWidget::ProgressWidget(QWidget *parent) : QWidget(parent),  m_ui(new Ui::ProgressWidget()),
						  m_lastTimestamp(0), m_lastIntervalSize(0)
{
  m_ui->setupUi(this);
  
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(updateDownloadSpeed()));
  m_timer->setInterval(2000);
  m_timer->start();
}

void ProgressWidget::updateDownloadSpeed()
{
  
  qint64 currTimestamp = QDateTime::currentMSecsSinceEpoch();
  qint64 timeDiff =  currTimestamp - m_lastTimestamp;
  
  qint64 currSize = m_ui->progressBar->value();
  qint64 sizeDiff = currSize - m_lastIntervalSize;
  
  
  kWarning() << "speed = " << sizeDiff / timeDiff;
  m_ui->lblDownloadProgress->setText(QString::number((sizeDiff / timeDiff)) + " KB/s");
  
  
  m_lastIntervalSize = currSize;
  m_lastTimestamp = currTimestamp;
}


void ProgressWidget::setMaximum(int size)
{
  m_ui->progressBar->setMaximum(size);
}

int ProgressWidget::getMaximum()
{
  return m_ui->progressBar->maximum();
}

void ProgressWidget::setMinimum(int size)
{
  m_ui->progressBar->setMinimum(size);
}

void ProgressWidget::setValue(int size)
{
  if (size >= m_ui->progressBar->maximum()) {
    m_timer->stop();
  } else if (! m_timer->isActive()) {
    m_timer->start();
  }
  m_ui->progressBar->setValue(size);
}

int ProgressWidget::getValue()
{
  return m_ui->progressBar->value();
}

void ProgressWidget::setFileName(const QString& fileName)
{
  m_ui->lblFilename->setText("Downloading " + fileName + " ...");
  m_fileName = fileName;
}

ProgressWidget::~ProgressWidget()
{
  m_timer->stop();
  delete m_timer;
  delete m_ui;
}
