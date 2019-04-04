#ifndef QC9518_H
#define QC9518_H

#include "pulsegenerator.h"

class Qc9518 : public PulseGenerator
{
public:
    explicit Qc9518(QObject *parent = nullptr);

	// HardwareObject interface
public slots:
    void readSettings();
	bool testConnection();
	void initialize();
	void sleep(bool b);

	// PulseGenerator interface
public slots:
    QVariant read(const int index, const BlackChirp::PulseSetting s);
	double readRepRate();
    bool set(const int index, const BlackChirp::PulseSetting s, const QVariant val);
	bool setRepRate(double d);

private:
	bool pGenWriteCmd(QString cmd);

    // HardwareObject interface
public slots:
    Experiment prepareForExperiment(Experiment exp);
    void beginAcquisition();
    void endAcquisition();
    void readTimeData();
};

#endif // QC9518_H
