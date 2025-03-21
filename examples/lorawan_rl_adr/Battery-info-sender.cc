#include "Battery-info-sender.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/lora-net-device.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE("BatteryInfoSender");

NS_OBJECT_ENSURE_REGISTERED(BatteryInfoSender);

TypeId
BatteryInfoSender::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::BatteryInfoSender")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<BatteryInfoSender>();
    return tid;
}

BatteryInfoSender::BatteryInfoSender()
{
    NS_LOG_FUNCTION(this);

}

BatteryInfoSender::~BatteryInfoSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
BatteryInfoSender::SetEnergySource(Ptr<energy::EnergySource> energySource)
{
    NS_LOG_FUNCTION(this);
    
    m_energySource = energySource;
}

void
BatteryInfoSender::SetSendInterval(Time interval)
{
    NS_LOG_FUNCTION(this);
    
    m_sendInterval = interval;
}

void
BatteryInfoSender::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    
    if (!m_mac)
    {
        // Assumes there's only one device
        Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(m_node->GetDevice(0));

        m_mac = loraNetDevice->GetMac();
        NS_ASSERT(m_mac);
    }

    ScheduleNextPacket();
}

void
BatteryInfoSender::StopApplication(void)
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
}

void
BatteryInfoSender::ScheduleNextPacket(void)
{
    NS_LOG_FUNCTION(this);
    
    m_sendEvent = Simulator::Schedule(m_sendInterval, &BatteryInfoSender::SendPacket, this);
}

void
BatteryInfoSender::SendPacket(void)
{
    NS_LOG_FUNCTION(this);
    
    if (m_energySource)
    {
        double remainingEnergy = m_energySource->GetRemainingEnergy();

        Ptr<Packet> packet = Create<Packet>((uint8_t*)&remainingEnergy, sizeof(remainingEnergy));
        m_mac->Send(packet);

        NS_LOG_INFO("Sent packet with payload: " << remainingEnergy);

        // // This is how the Receive packet will look like 
        // uint32_t packetSize = packet->GetSize();
        // uint8_t *buffer = new uint8_t[packetSize];
        // packet->CopyData(buffer, packetSize);

        // double payloadRcv;
        // std::memcpy(&payloadRcv, buffer, sizeof(payloadRcv));
        // delete[] buffer;
        // NS_LOG_INFO("Received packet payload as double: " << payloadRcv);
    }

    ScheduleNextPacket();
}

} // namespace lorawan
} // namespace ns3