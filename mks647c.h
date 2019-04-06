#ifndef MKS647C_H
#define MKS647C_H

#include "flowcontroller.h"

class Mks647c : public FlowController
{
public:
    explicit Mks647c(QObject *parent = nullptr);

    // FlowController interface
public slots:
    double setFlowSetpoint(const int ch, const double val);
    double setPressureSetpoint(const double val);
    double readFlowSetpoint(const int ch);
    double readPressureSetpoint();
    double readFlow(const int ch);
    double readPressure();
    void setPressureControlMode(bool enabled);
    bool readPressureControlMode();

    // HardwareObject interface
    void sleep(bool b);

protected:
    bool testConnection();

    // FlowController interface
    virtual void fcInitialize();



private:
    QList<double> d_gasRangeList;
    QList<double> d_pressureRangeList;
    int d_pressureRangeIndex;
    QList<int> d_rangeIndexList;
    QList<double> d_gcfList;

    QByteArray mksQueryCmd(QString cmd, int respLength);
    int d_maxTries;

    // HardwareObject interface
public slots:
    Experiment prepareForExperiment(Experiment exp);
    void beginAcquisition();
    void endAcquisition();
};

#endif // MKS647C_H
