#include "acquisitionmanager.h"

AcquisitionManager::AcquisitionManager(QObject *parent) : QObject(parent), d_state(Idle)
{

}

AcquisitionManager::~AcquisitionManager()
{

}

void AcquisitionManager::beginExperiment(Experiment exp)
{
	if(!exp.isInitialized() || exp.isDummy())
    {
        if(!exp.errorString().isEmpty())
            emit logMessage(exp.errorString(),BlackChirp::LogError);
		emit experimentComplete(exp);
        return;
    }

    //prepare data files, savemanager, fidmanager, etc
    d_currentExperiment = exp;

    d_state = Acquiring;
    emit logMessage(exp.startLogMessage(),BlackChirp::LogHighlight);
    emit statusMessage(QString("Acquiring"));

    if(d_currentExperiment.timeDataInterval() > 0)
    {
        if(d_timeDataTimer == nullptr)
            d_timeDataTimer = new QTimer(this);
        getTimeData();
        connect(d_timeDataTimer,&QTimer::timeout,this,&AcquisitionManager::getTimeData,Qt::UniqueConnection);
        d_timeDataTimer->start(d_currentExperiment.timeDataInterval()*1000);
    }
    emit beginAcquisition();

}

void AcquisitionManager::processFtmwScopeShot(const QByteArray b)
{
//    static int total = 0;
//    static int count = 0;
    if(d_state == Acquiring && d_currentExperiment.ftmwConfig().isEnabled() && !d_currentExperiment.ftmwConfig().isComplete())
    {
//        d_testTime.restart();
        bool success = true;
        if(d_currentExperiment.ftmwConfig().fidList().isEmpty())
            success = d_currentExperiment.setFids(b);
        else
            success = d_currentExperiment.addFids(b);

        if(!success)
        {
            emit logMessage(d_currentExperiment.errorString(),BlackChirp::LogError);
            abort();
            return;
        }

//        int t = d_testTime.elapsed();
//        total += t;
//        count++;
//        emit logMessage(QString("Elapsed time: %1 ms, avg: %2").arg(t).arg(total/count));

        d_currentExperiment.incrementFtmw();
        emit newFidList(d_currentExperiment.ftmwConfig().fidList());

        if(d_currentExperiment.ftmwConfig().type() == BlackChirp::FtmwTargetTime)
        {
            qint64 elapsedSecs = d_currentExperiment.startTime().secsTo(QDateTime::currentDateTime());
            emit ftmwShotAcquired(elapsedSecs);
        }
        else
            emit ftmwShotAcquired(d_currentExperiment.ftmwConfig().completedShots());

        checkComplete();
    }
}

void AcquisitionManager::processLifScopeShot(const LifTrace t)
{
    if(d_state == Acquiring && d_currentExperiment.lifConfig().isEnabled() && !d_currentExperiment.isLifWaiting())
    {
        //process trace; only send data to UI if point is complete
        if(d_currentExperiment.addLifWaveform(t))
        {
            emit lifPointUpdate(d_currentExperiment.lifConfig().lastUpdatedLifPoint());

            if(!d_currentExperiment.isComplete())
            {
                d_currentExperiment.setLifWaiting(true);
                emit nextLifPoint(d_currentExperiment.lifConfig().currentDelay(),
                                  d_currentExperiment.lifConfig().currentFrequency());

                emit logMessage(QString("Lif Point: %1, %2").arg(d_currentExperiment.lifConfig().currentDelay(),0,'f',3).arg(d_currentExperiment.lifConfig().currentFrequency(),0,'f',0));
            }
        }

        if(d_currentExperiment.lifConfig().completedShots() <= d_currentExperiment.lifConfig().totalShots())
        {
            emit lifShotAcquired(d_currentExperiment.lifConfig().completedShots());
//            emit logMessage(QString("Lif: %1/%2").arg(d_currentExperiment.lifConfig().completedShots()).arg(d_currentExperiment.lifConfig().totalShots()));
        }

        checkComplete();
    }
}

void AcquisitionManager::lifHardwareReady(bool success)
{
    if(d_currentExperiment.lifConfig().isEnabled())
    {
        if(!success)
        {
            emit logMessage(QString("LIF delay and/or frequency could not be set. Aborting."),BlackChirp::LogError);
            abort();
        }
        else
            d_currentExperiment.setLifWaiting(false);
    }
}

void AcquisitionManager::changeRollingAverageShots(int newShots)
{
    if(d_state == Acquiring && d_currentExperiment.ftmwConfig().isEnabled() && d_currentExperiment.ftmwConfig().type() == BlackChirp::FtmwPeakUp)
        d_currentExperiment.overrideTargetShots(newShots);
}

void AcquisitionManager::resetRollingAverage()
{
    if(d_state == Acquiring && d_currentExperiment.ftmwConfig().isEnabled() && d_currentExperiment.ftmwConfig().type() == BlackChirp::FtmwPeakUp)
        d_currentExperiment.resetFids();
}

void AcquisitionManager::getTimeData()
{
    if(d_state == Acquiring)
    {
        emit timeDataSignal();

        d_currentExperiment.addTimeStamp();
        emit logMessage(QString("Timestamp: %1").arg(QDateTime::currentDateTime().toString()));

        if(d_currentExperiment.ftmwConfig().isEnabled())
        {
            QList<QPair<QString,QVariant>> l { qMakePair(QString("ftmwShots"),d_currentExperiment.ftmwConfig().completedShots()) };
            d_currentExperiment.addTimeData(l);
            emit timeData(l);
            emit logMessage(QString("ftmwShots: %1").arg(d_currentExperiment.ftmwConfig().completedShots()));
        }
    }
}

void AcquisitionManager::processTimeData(const QList<QPair<QString, QVariant> > timeDataList)
{
	if(d_state == Acquiring)
	{
		//test for abort conditions here!
		//
		//

//		emit logMessage(QString("Time data received."));
//		for(int i=0; i<timeDataList.size(); i++)
//			emit logMessage(QString("%1: %2").arg(timeDataList.at(i).first,timeDataList.at(i).second.toString()));

		d_currentExperiment.addTimeData(timeDataList);
	}
}

void AcquisitionManager::pause()
{
    if(d_state == Acquiring)
    {
        d_state = Paused;
        emit statusMessage(QString("Paused"));
    }
}

void AcquisitionManager::resume()
{
    if(d_state == Paused)
    {
        d_state = Acquiring;
        emit statusMessage(QString("Acquiring"));
    }
}

void AcquisitionManager::abort()
{
    if(d_state == Paused || d_state == Acquiring)
    {
        d_currentExperiment.setAborted();
        //save!
        endAcquisition();
    }
}

void AcquisitionManager::checkComplete()
{
    if(d_state == Acquiring && d_currentExperiment.isComplete())
    {
        //do final save
        endAcquisition();
    }
}

void AcquisitionManager::endAcquisition()
{
    d_state = Idle;

    disconnect(d_timeDataTimer,&QTimer::timeout,this,&AcquisitionManager::getTimeData);
    d_timeDataTimer->stop();
    d_currentExperiment.save();

    emit experimentComplete(d_currentExperiment);
}

