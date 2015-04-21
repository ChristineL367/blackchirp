#ifndef PULSEGENERATOR_H
#define PULSEGENERATOR_H

#include "hardwareobject.h"
#include "pulsegenconfig.h"

#if BC_PGEN==1
class Qc9528;
typedef Qc9528 PulseGeneratorHardware;

//NOTE: Gas, AWG, excimer, and LIF channels are hardcoded
#define BC_PGEN_GASCHANNEL 0
#define BC_PGEN_AWGCHANNEL 1
#define BC_PGEN_XMERCHANNEL 2
#define BC_PGEN_LIFCHANNEL 3
#define BC_PGEN_NUMCHANNELS 8

#else
class VirtualPulseGenerator;
typedef VirtualPulseGenerator PulseGeneratorHardware;

//NOTE: Gas, AWG, excimer, and LIF channels are hardcoded
#define BC_PGEN_GASCHANNEL 0
#define BC_PGEN_AWGCHANNEL 1
#define BC_PGEN_XMERCHANNEL 2
#define BC_PGEN_LIFCHANNEL 3
#define BC_PGEN_NUMCHANNELS 8

#endif

class PulseGenerator : public HardwareObject
{
    Q_OBJECT
public:
    PulseGenerator(QObject *parent = nullptr);
    ~PulseGenerator();

public slots:
    PulseGenConfig config() const { return d_config; }
    virtual QVariant read(const int index, const PulseGenConfig::Setting s) =0;

    virtual PulseGenConfig::ChannelConfig read(const int index);

    virtual void set(const int index, const PulseGenConfig::Setting s, const QVariant val) =0;
    virtual void setChannel(const int index, const PulseGenConfig::ChannelConfig cc);
    virtual void setAll(const PulseGenConfig cc);

    virtual void setRepRate(double d) =0;

signals:
    void settingUpdate(int,PulseGenConfig::Setting,QVariant);
    void configUpdate(const PulseGenConfig);
    void repRateUpdate(double);

protected:
    PulseGenConfig d_config;
    virtual void readAll();
};

#endif // PULSEGENERATOR_H
