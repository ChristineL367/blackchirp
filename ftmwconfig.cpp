#include "ftmwconfig.h"

#include <QFile>

FtmwConfig::FtmwConfig() : data(new FtmwConfigData)
{

}

FtmwConfig::FtmwConfig(const FtmwConfig &rhs) : data(rhs.data)
{

}

FtmwConfig &FtmwConfig::operator=(const FtmwConfig &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

FtmwConfig::FtmwConfig(int num) : data(new FtmwConfigData)
{
    //eventually, do all the file parsing etc...
    //for now, just build the fid list
    QFile fid(BlackChirp::getExptFile(num,BlackChirp::FidFile));
    if(fid.open(QIODevice::ReadOnly))
    {
        QDataStream d(&fid);
        QByteArray magic;
        d >> magic;
        if(magic.startsWith("BCFID"))
        {
            if(magic.endsWith("v1.0"))
                d >> data->fidList;
        }
        fid.close();
    }
}

FtmwConfig::~FtmwConfig()
{

}

bool FtmwConfig::isEnabled() const
{
    return data->isEnabled;
}

BlackChirp::FtmwType FtmwConfig::type() const
{
    return data->type;
}

qint64 FtmwConfig::targetShots() const
{
    return data->targetShots;
}

qint64 FtmwConfig::completedShots() const
{
    return data->completedShots;
}

QDateTime FtmwConfig::targetTime() const
{
    return data->targetTime;
}

int FtmwConfig::autoSaveShots() const
{
    return data->autoSaveShots;
}

double FtmwConfig::loFreq() const
{
    return data->loFreq;
}

BlackChirp::Sideband FtmwConfig::sideband() const
{
    return data->sideband;
}

QList<Fid> FtmwConfig::fidList() const
{
    return data->fidList;
}

BlackChirp::FtmwScopeConfig FtmwConfig::scopeConfig() const
{
    return data->scopeConfig;
}

ChirpConfig FtmwConfig::chirpConfig() const
{
    return data->chirpConfig;
}

Fid FtmwConfig::fidTemplate() const
{
    return data->fidTemplate;
}

int FtmwConfig::numFrames() const
{
    return scopeConfig().summaryFrame ? 1 : scopeConfig().numFrames;
}

QList<Fid> FtmwConfig::parseWaveform(QByteArray b) const
{

    int np = scopeConfig().recordLength;
    QList<Fid> out;
    //read raw data into vector in 64 bit integer form
    for(int j=0;j<numFrames();j++)
    {
        QVector<qint64> d(np);

        for(int i=0; i<np;i++)
        {
            if(scopeConfig().bytesPerPoint == 1)
            {
                char y = b.at(j*np+i);
                d[i] = (static_cast<qint64>(y) + static_cast<qint64>(scopeConfig().yOff));
            }
            else
            {
                char y1 = b.at(2*(j*np+i));
                char y2 = b.at(2*(j*np+i) + 1);
                qint16 y = 0;
                if(scopeConfig().byteOrder == QDataStream::LittleEndian)
                {
                    y += (qint8)y1;
                    y += 256*(qint8)y2;
                }
                else
                {
                    y += (qint8)y2;
                    y += 256*(qint8)y1;
                }
                d[i] = (static_cast<qint64>(y) + static_cast<qint64>(scopeConfig().yOff));
            }
        }

        Fid f = fidTemplate();
        f.setData(d);
        out.append(f);
    }

    return out;
}

QString FtmwConfig::errorString() const
{
    return data->errorString;
}

double FtmwConfig::ftMin() const
{
    double sign = 1.0;
    if(data->sideband == BlackChirp::LowerSideband)
        sign = -1.0;
    double lastFreq = data->loFreq + sign*data->scopeConfig.sampleRate/(1e6*2.0);
    return qMin(data->loFreq,lastFreq);
}

double FtmwConfig::ftMax() const
{
    double sign = 1.0;
    if(data->sideband == BlackChirp::LowerSideband)
        sign = -1.0;
    double lastFreq = data->loFreq + sign*data->scopeConfig.sampleRate/(1e6*2.0);
    return qMax(data->loFreq,lastFreq);
}

bool FtmwConfig::writeFidFile(int num) const
{
    QFile fid(BlackChirp::getExptFile(num,BlackChirp::FidFile));
    if(fid.open(QIODevice::WriteOnly))
    {
        QDataStream d(&fid);
        d << Fid::magicString();
        d << data->fidList;
        fid.close();
        return true;
    }
    else
        return false;
}

bool FtmwConfig::prepareForAcquisition()
{
    Fid f(scopeConfig().xIncr,loFreq(),QVector<qint64>(0),sideband(),scopeConfig().yMult,1);
    data->fidTemplate = f;

#ifdef BC_CUDA
    bool success = data->gpuAvg.initialize(scopeConfig().recordLength,numFrames(),scopeConfig().bytesPerPoint,scopeConfig().byteOrder);
    if(!success)
    {
        data->errorString = data->gpuAvg.getErrorString();
        return false;
    }
#endif

    if(!chirpConfig().isValid())
        return false;

    return true;


}

void FtmwConfig::setEnabled()
{
    data->isEnabled = true;
}

void FtmwConfig::setFidTemplate(const Fid f)
{
    data->fidTemplate = f;
}

void FtmwConfig::setType(const BlackChirp::FtmwType type)
{
    data->type = type;
}

void FtmwConfig::setTargetShots(const qint64 target)
{
    data->targetShots = target;
}

void FtmwConfig::increment()
{
    if(type() == BlackChirp::FtmwPeakUp)
        data->completedShots = qMin(completedShots()+1,targetShots());
    else
        data->completedShots++;
}

void FtmwConfig::setTargetTime(const QDateTime time)
{
    data->targetTime = time;
}

void FtmwConfig::setAutoSaveShots(const int shots)
{
    data->autoSaveShots = shots;
}

void FtmwConfig::setLoFreq(const double f)
{
    data->loFreq = f;
}

void FtmwConfig::setSideband(const BlackChirp::Sideband sb)
{
    data->sideband = sb;
}

bool FtmwConfig::setFids(const QByteArray newData)
{
#ifndef BC_CUDA
    data->fidList = parseWaveform(newData);
#else
    QList<QVector<qint64> > l = data->gpuAvg.parseAndAdd(newData.constData());

    if(l.isEmpty())
    {
        data->errorString = data->gpuAvg.getErrorString();
        return false;
    }

    if(!data->fidList.isEmpty())
        data->fidList.clear();

    for(int i=0; i<numFrames(); i++)
    {
        Fid f = fidTemplate();
        f.setData(l.at(i));
        data->fidList.append(f);
    }
#endif
    return true;
}

bool FtmwConfig::addFids(const QByteArray rawData)
{
#ifndef BC_CUDA
    QList<Fid> newList = parseWaveform(rawData);
    if(type() == BlackChirp::FtmwPeakUp)
    {
        for(int i=0; i<data->fidList.size(); i++)
            newList[i].rollingAverage(data->fidList.at(i),targetShots());
    }
    else
    {
        for(int i=0; i<data->fidList.size(); i++)
            newList[i] += data->fidList.at(i);
    }
    data->fidList = newList;
#else
    QList<QVector<qint64> >  l;
    if(type() == BlackChirp::FtmwPeakUp)
        l = data->gpuAvg.parseAndRollAvg(rawData.constData(),completedShots()+1,targetShots());
    else
        l=data->gpuAvg.parseAndAdd(rawData.constData());

    if(l.isEmpty())
    {
        data->errorString = data->gpuAvg.getErrorString();
        return false;
    }

    for(int i=0; i<numFrames(); i++)
    {
        data->fidList.removeFirst();
        Fid f = fidTemplate();
        f.setData(l.at(i));
        if(type() == BlackChirp::FtmwPeakUp)
            f.setShots(qMin(completedShots()+1,targetShots()));
        else
            f.setShots(completedShots()+1);
        data->fidList.append(f);
    }

#endif
    return true;
}

void FtmwConfig::resetFids()
{
    data->fidList.clear();
    data->completedShots = 0;
#ifdef BC_CUDA
    data->gpuAvg.resetAverage();
#endif
}

void FtmwConfig::setScopeConfig(const BlackChirp::FtmwScopeConfig &other)
{
    data->scopeConfig = other;
}

void FtmwConfig::setChirpConfig(const ChirpConfig other)
{
    data->chirpConfig = other;
}

bool FtmwConfig::isComplete() const
{
    if(!isEnabled())
        return true;

    switch(type())
    {
    case BlackChirp::FtmwTargetShots:
        return completedShots() >= targetShots();
        break;
    case BlackChirp::FtmwTargetTime:
        return QDateTime::currentDateTime() >= targetTime();
        break;
    case BlackChirp::FtmwForever:
    case BlackChirp::FtmwPeakUp:
    default:
        return false;
        break;
    }

    //not reached
    return false;
}

QMap<QString, QPair<QVariant, QString> > FtmwConfig::headerMap() const
{
    QMap<QString, QPair<QVariant, QString> > out;

    QString prefix = QString("FtmwConfig");
    QString empty = QString("");

    out.insert(prefix+QString("Enabled"),qMakePair(isEnabled(),empty));
    if(!isEnabled())
        return out;

    out.insert(prefix+QString("Type"),qMakePair((int)type(),empty));
    if(type() == BlackChirp::FtmwTargetShots)
        out.insert(prefix+QString("TargetShots"),qMakePair(targetShots(),empty));
    if(type() == BlackChirp::FtmwTargetTime)
        out.insert(prefix+QString("TargetTime"),qMakePair(targetTime(),empty));
    out.insert(prefix+QString("LoFrequency"),qMakePair(QString::number(loFreq(),'f',6),QString("MHz")));
    out.insert(prefix+QString("Sideband"),qMakePair((int)sideband(),empty));
    out.insert(prefix+QString("FidVMult"),qMakePair(QString::number(fidTemplate().vMult(),'g',12),QString("V")));
    out.insert(prefix+QString("CompletedShots"),qMakePair(completedShots(),empty));

    BlackChirp::FtmwScopeConfig sc = scopeConfig();
    out.unite(sc.headerMap());
    out.unite(data->chirpConfig.headerMap());

    return out;

}

