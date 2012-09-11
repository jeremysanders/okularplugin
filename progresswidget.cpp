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
						  m_lastTimestamp(0), m_bytesDownloadedTotalLastInterval(0),
						  m_progressBarValue(0), m_avgDownloadSpeedWeighted(0),
						  m_vecDownloadSpeeds(QVector<float>())
{
  m_ui->setupUi(this);
  
  m_ui->lblDownloadSpeed->setText("");
  m_ui->lblRemainingDlTime->setText("");
  
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(updateDownloadSpeed()));
  m_timer->setInterval(1500);
  m_timer->start();
}

void ProgressWidget::updateDownloadSpeed()
{
  
  qint64 currTimestamp = QDateTime::currentMSecsSinceEpoch();
  if (m_lastTimestamp == 0) {
    m_lastTimestamp = currTimestamp - m_timer->interval();
  }  
  qint64 intervalLength = currTimestamp - m_lastTimestamp;
  qint64 amountDownloadedTotal = m_progressBarValue;
  qint64 amountDownloaded = amountDownloadedTotal - m_bytesDownloadedTotalLastInterval;
  qint64 amountRemaining = m_ui->progressBar->maximum() - amountDownloadedTotal;
  
  float currDownloadSpeed = (float)amountDownloaded / (float)intervalLength;
  
  if (m_avgDownloadSpeedWeighted == 0)
    m_avgDownloadSpeedWeighted = currDownloadSpeed;
    
  if (m_vecDownloadSpeeds.size() >= 180) {
    m_vecDownloadSpeeds.pop_front();
  }
  m_vecDownloadSpeeds.push_back(currDownloadSpeed);
  
#ifdef DEBUG_PROGRESSWIDGET
  m_vecDownloadSpeedsForEstimation.push_back(currDownloadSpeed);
#endif

  float avgDownloadSpeed = 0;
  for (int i=0; i<m_vecDownloadSpeeds.size(); ++i) {
    avgDownloadSpeed += m_vecDownloadSpeeds[i];
  }
  avgDownloadSpeed /= m_vecDownloadSpeeds.size();
  
  
  m_avgDownloadSpeedWeighted = 0.02 * currDownloadSpeed + 0.70 * m_avgDownloadSpeedWeighted + 0.28 * avgDownloadSpeed;

  QTime remainingDlTime = QTime();
  remainingDlTime = remainingDlTime.addSecs(amountRemaining / (float) m_avgDownloadSpeedWeighted / 1000.0f);  

  
#ifdef DEBUG_PROGRESSWIDGET  
  kWarning() << "remaining msces = " << amountRemaining / m_avgDownloadSpeedWeighted;
  kWarning() << remainingDlTime;
  kWarning() << "remainingTime = " << remainingDlTime.toString();
  kWarning() << "currDownload Speed = " << currDownloadSpeed;
  
  if (m_vecDownloadSpeedsForEstimation.size() % 60 == 0) {
    calcEstimationError();
  }
#endif  
  
  m_ui->lblRemainingDlTime->setText(remainingDlTime.toString("h:mm:ss"));
  m_ui->lblDownloadSpeed->setText(QString::number(m_avgDownloadSpeedWeighted, 'f', 2) + " KB/s");  
  
  m_bytesDownloadedTotalLastInterval = amountDownloadedTotal;
  m_lastTimestamp = currTimestamp;
}

#ifdef DEBUG_PROGRESSWIDGET
void ProgressWidget::calcEstimationError() {
    int sumDownloadSpeeds = 0;
    for (int i=0; i<m_vecDownloadSpeedsForEstimation.size(); ++i) {
      sumDownloadSpeeds += m_vecDownloadSpeedsForEstimation[i];
    }
    kWarning() << "avg = " << (float) sumDownloadSpeeds / (float) m_vecDownloadSpeedsForEstimation.size();
    kWarning() << "deviation = " << (float) sumDownloadSpeeds / (float) m_vecDownloadSpeedsForEstimation.size() - m_avgDownloadSpeedWeighted;
}
#endif

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
  m_progressBarValue = size;
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
