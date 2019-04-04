#ifndef VIRTUALFTMWSCOPE_H
#define VIRTUALFTMWSCOPE_H

#include "ftmwscope.h"

#include <QVector>
#include <QTimer>

class VirtualFtmwScope : public FtmwScope
{
    Q_OBJECT
public:
    explicit VirtualFtmwScope(QObject *parent = nullptr);
    ~VirtualFtmwScope();

    // HardwareObject interface
public slots:
    void readSettings();
    bool testConnection();
    void initialize();
    Experiment prepareForExperiment(Experiment exp);
    void beginAcquisition();
    void endAcquisition();

    void readWaveform();

private:
    QVector<double> d_simulatedData;
    QTimer *d_simulatedTimer = nullptr;
    QTime d_testTime;
};

#endif // VIRTUALFTMWSCOPE_H
