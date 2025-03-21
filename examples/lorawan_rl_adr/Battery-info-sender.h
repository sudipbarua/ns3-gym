#ifndef BATTERY_INFO_SENDER_H
#define BATTERY_INFO_SENDER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/energy-source-container.h"
#include "ns3/lorawan-mac.h"


namespace ns3 {
namespace lorawan {

class BatteryInfoSender : public Application
{
public:
    static TypeId GetTypeId(void);
    BatteryInfoSender();
    virtual ~BatteryInfoSender();

    void SetEnergySource(Ptr<energy::EnergySource> energySource);
    void SetSendInterval(Time interval);

protected:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

private:
    void ScheduleNextPacket(void);
    void SendPacket(void);

    Ptr<LorawanMac> m_mac;
    EventId m_sendEvent;
    Time m_sendInterval;
    Ptr<energy::EnergySource> m_energySource;
};

} // namespace lorawan
} // namespace ns3

#endif // BATTERY_SENDER_H