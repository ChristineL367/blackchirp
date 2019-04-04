#ifndef VIRTUALAWG_H
#define VIRTUALAWG_H

#include "awg.h"

class VirtualAwg : public AWG
{
    Q_OBJECT
public:
    explicit VirtualAwg(QObject *parent = nullptr);
    ~VirtualAwg();

    // HardwareObject interface
public slots:
    void readSettings();
    bool testConnection();
    void initialize();
    Experiment prepareForExperiment(Experiment exp);
    void beginAcquisition();
    void endAcquisition();
};

#endif // VIRTUALAWG_H
