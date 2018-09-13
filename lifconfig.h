#ifndef LIFCONFIG_H
#define LIFCONFIG_H

#include <QSharedDataPointer>

#include <QDataStream>
#include <QList>
#include <QVector>
#include <QPointF>
#include <QMap>
#include <QVariant>

#include "datastructs.h"
#include "liftrace.h"


class LifConfigData;

class LifConfig
{
public:
    LifConfig();
    LifConfig(const LifConfig &);
    LifConfig &operator=(const LifConfig &);
    ~LifConfig();   

    bool isEnabled() const;
    bool isComplete() const;
    bool isValid() const;
    double currentDelay() const;
    double currentFrequency() const;
    QPair<double,double> delayRange() const;
    double delayStep() const;
    QPair<double,double> frequencyRange() const;
    double frequencyStep() const;
    int numDelayPoints() const;
    int numFrequencyPoints() const;
    int shotsPerPoint() const;
    int totalShots() const;
    int completedShots() const;
    BlackChirp::LifScopeConfig scopeConfig() const;
    BlackChirp::LifScanOrder order() const;
    BlackChirp::LifCompleteMode completeMode() const;
    QVector<QPointF> timeSlice(int frequencyIndex) const;
    QVector<QPointF> spectrum(int delayIndex) const;
    QList<QVector<BlackChirp::LifPoint>> lifData() const;
    QMap<QString,QPair<QVariant,QString> > headerMap() const;
    void parseLine(QString key, QVariant val);
    bool loadLifData(int num, const QString path = QString(""));
    QPair<QPoint,BlackChirp::LifPoint> lastUpdatedLifPoint() const;
    bool writeLifFile(int num) const;

    void setEnabled(bool en = true);
    bool validate();
    bool allocateMemory();
    void setLifGate(int start, int end);
    void setRefGate(int start, int end);
    void setDelayParameters(double start, double stop, double step);
    void setFrequencyParameters(double start, double stop, double step);
    void setOrder(BlackChirp::LifScanOrder o);
    void setCompleteMode(BlackChirp::LifCompleteMode mode);
    void setScopeConfig(BlackChirp::LifScopeConfig c);
    void setShotsPerPoint(int pts);
    bool addWaveform(const LifTrace t);

    void saveToSettings() const;
    static LifConfig loadFromSettings();


private:
    QSharedDataPointer<LifConfigData> data;

    bool addPoint(const double d);
    void increment();
};


class LifConfigData : public QSharedData
{
public:
    LifConfigData() : enabled(false), complete(false),  valid(false), memAllocated(false), order(BlackChirp::LifOrderDelayFirst),
        completeMode(BlackChirp::LifContinueUntilExperimentComplete), delayStartUs(-1.0), delayEndUs(-1.0),
        delayStepUs(0.0), frequencyStart(-1.0), frequencyEnd(-1.0), frequencyStep(0.0), lifGateStartPoint(-1), lifGateEndPoint(-1),
        refGateStartPoint(-1), refGateEndPoint(-1), currentDelayIndex(0), currentFrequencyIndex(0) {}

    bool enabled;
    bool complete;
    bool valid;
    bool memAllocated;
    BlackChirp::LifScanOrder order;
    BlackChirp::LifCompleteMode completeMode;
    double delayStartUs;
    double delayEndUs;
    double delayStepUs;
    double frequencyStart;
    double frequencyEnd;
    double frequencyStep;
    int lifGateStartPoint;
    int lifGateEndPoint;
    int refGateStartPoint;
    int refGateEndPoint;
    int currentDelayIndex;
    int currentFrequencyIndex;
    int shotsPerPoint;
    QPoint lastUpdatedPoint;

    BlackChirp::LifScopeConfig scopeConfig;
    QList<QVector<BlackChirp::LifPoint>> lifData;

};

QDataStream &operator<<(QDataStream &stream, const BlackChirp::LifPoint &pt);
QDataStream &operator>>(QDataStream &stream, BlackChirp::LifPoint &pt);

Q_DECLARE_METATYPE(LifConfig)


#endif // LIFCONFIG_H
