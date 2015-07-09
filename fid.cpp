#include "fid.h"

Fid::Fid() : data(new FidData)
{
}

Fid::Fid(const Fid &rhs) : data(rhs.data)
{
}

Fid::Fid(const double sp, const double p, const QVector<qint64> d, BlackChirp::Sideband sb, double vMult, qint64 shots)
{
    data = new FidData;
    data->spacing = sp;
    data->probeFreq = p;
    data->fid = d;
    data->sideband = sb;
    data->vMult = vMult;
    data->shots = shots;
}

Fid &Fid::operator=(const Fid &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

Fid::~Fid()
{
}

void Fid::setSpacing(const double s)
{
    data->spacing = s;
}

void Fid::setProbeFreq(const double f)
{
    data->probeFreq = f;
}

void Fid::setData(const QVector<qint64> d)
{
    data->fid = d;
}

void Fid::setSideband(const BlackChirp::Sideband sb)
{
    data->sideband = sb;
}

void Fid::setVMult(const double vm)
{
    data->vMult = vm;
}

void Fid::setShots(const qint64 s)
{
    data->shots = s;
}

Fid &Fid::operator +=(const Fid other)
{
    Q_ASSERT(size() == other.size());
    for(int i=0;i<size();i++)
        data->fid[i] += other.atRaw(i);

    data->shots += other.shots();

    return *this;
}

Fid &Fid::operator +=(const QVector<qint64> other)
{
    Q_ASSERT(size() == other.size());
    for(int i=0; i<size();i++)
        data->fid[i] += other.at(i);

    data->shots++;

    return *this;
}

Fid &Fid::operator +=(const qint64 *other)
{
    for(int i=0; i<size();i++)
        data->fid[i] += other[i];

    data->shots++;

    return *this;
}

void Fid::add(const qint64 *other, const unsigned int offset)
{
    for(int i=0; i<size(); i++)
        data->fid[i] += other[i+offset];

    data->shots++;
}

void Fid::copyAdd(const qint64 *other, const unsigned int offset)
{
    memcpy(data->fid.data(),other + offset,sizeof(qint64)*size());
    data->shots++;
}

void Fid::rollingAverage(const Fid other, qint64 targetShots)
{
    qint64 totalShots = shots() + other.shots();
    if(totalShots <= targetShots)
        *this += other;
    else
    {
        for(int i=0; i<size(); i++)
            data->fid[i] = targetShots*(atRaw(i) + other.atRaw(i))/totalShots;

        data->shots = targetShots;
    }
}

int Fid::size() const
{
    return data->fid.size();
}

double Fid::at(const int i) const
{
    return atNorm(i)*data->vMult;
}

double Fid::spacing() const
{
    return data->spacing;
}

double Fid::probeFreq() const
{
    return data->probeFreq;
}

QVector<QPointF> Fid::toXY() const
{
    //create a vector of points that can be displayed
    QVector<QPointF> out;
    out.reserve(size());

    for(int i=0; i<size(); i++)
        out.append(QPointF(spacing()*(double)i,at(i)));

    return out;
}

QVector<double> Fid::toVector() const
{
    QVector<double> out;
    out.reserve(size());
    for(int i=0;i<size();i++)
        out.append(at(i));

    return out;
}

QVector<qint64> Fid::rawData() const
{
    return data->fid;
}

qint64 Fid::atRaw(const int i) const
{
    return data->fid.at(i);
}

double Fid::atNorm(const int i) const
{
    if(data->shots > 1)
        return (double)data->fid.at(i)/(double)data->shots;
    else
        return (double)data->fid.at(i);
}

qint64 Fid::shots() const
{
    return data->shots;
}

BlackChirp::Sideband Fid::sideband() const
{
    return data->sideband;
}

double Fid::maxFreq() const
{
    if(spacing()>0.0)
    {
        if(sideband() == BlackChirp::UpperSideband)
            return probeFreq() + 1.0/2.0/spacing()/1e6;
        else
            return probeFreq();
    }
    else
        return 0.0;
}

double Fid::minFreq() const
{
    if(spacing()>0.0)
    {
        if(sideband() == BlackChirp::LowerSideband)
            return probeFreq() - 1.0/2.0/spacing()/1e6;
        else
            return probeFreq();
    }
    else
        return 0.0;
}

double Fid::vMult() const
{
    return data->vMult;
}

QByteArray Fid::magicString()
{
    return QByteArray("BCFIDv1.0");
}


QDataStream &operator<<(QDataStream &stream, const Fid fid)
{
    stream << fid.spacing() << fid.probeFreq() << fid.vMult() << fid.shots();
    qint16 sb = static_cast<qint16>(fid.sideband());
    stream << sb << fid.rawData();
    return stream;
}


QDataStream &operator>>(QDataStream &stream, Fid fid)
{
    double spacing, probeFreq, vMult;
    qint64 shots;
    qint16 sb;
    stream >> spacing >> probeFreq >> vMult >> shots >> sb;
    QVector<qint64> dat;
    stream >> dat;

    BlackChirp::Sideband sideband = static_cast<BlackChirp::Sideband>(sb);
    fid.setSpacing(spacing);
    fid.setProbeFreq(probeFreq);
    fid.setVMult(vMult);
    fid.setShots(shots);
    fid.setSideband(sideband);
    fid.setData(dat);

    return stream;
}
