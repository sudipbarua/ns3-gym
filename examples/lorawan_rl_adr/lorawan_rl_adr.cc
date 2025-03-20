#include "lorawan_rl_adr.h"
#include "rl_adr_env.h"

namespace ns3
{
namespace lorawan
{

////////////////////////////////////////
// LinkAdrRequest commands management //
////////////////////////////////////////

NS_LOG_COMPONENT_DEFINE("AdrRL");

NS_OBJECT_ENSURE_REGISTERED(AdrRL);

TypeId
AdrRL::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::AdrRL")
            .SetGroupName("lorawan")
            .AddConstructor<AdrRL>()
            .SetParent<NetworkControllerComponent>()
            .AddAttribute("MultiplePacketsCombiningMethod",
                        "Whether to average SNRs from multiple packets or to use the maximum",
                        EnumValue(AdrRL::AVERAGE),
                        MakeEnumAccessor<CombiningMethod>(&AdrRL::historyAveraging),
                        MakeEnumChecker(AdrRL::AVERAGE,
                                        "avg",
                                        AdrRL::MAXIMUM,
                                        "max",
                                        AdrRL::MINIMUM,
                                        "min"))
            .AddAttribute("HistoryRange",
                        "Number of packets to use for averaging",
                        IntegerValue(3),
                        MakeIntegerAccessor(&AdrRL::historyRange),
                        MakeIntegerChecker<int>(0, 100))
            .AddAttribute("ChangeTransmissionPower",
                        "Whether to toggle the transmission power or not",
                        BooleanValue(true),
                        MakeBooleanAccessor(&AdrRL::m_toggleTxPower),
                        MakeBooleanChecker());
    return tid;
}

AdrRL::AdrRL()
{
    NS_LOG_FUNCTION(this);
    m_gymEnv = 0;
}

AdrRL::~AdrRL()
{
    NS_LOG_FUNCTION(this);
    m_gymEnv = 0;
}

void
AdrRL::CreateGymEnv()
{
    Ptr<LorawanGymEnv> gymEnv = CreateObject<LorawanGymEnv>();
    m_gymEnv = gymEnv;
    NS_LOG_DEBUG("Gym environment created");
}

void
AdrRL::OnReceivedPacket(Ptr<const Packet> packet,
                            Ptr<EndDeviceStatus> status,
                            Ptr<NetworkStatus> networkStatus)
{
    // Check if the environment has been created
    if (!m_gymEnv)
    {
        CreateGymEnv();
    }

    NS_LOG_FUNCTION(this->GetTypeId() << packet << networkStatus);

    uint32_t packetSize = packet->GetSize();
    uint8_t *buffer = new uint8_t[packetSize];
    packet->CopyData(buffer, packetSize);

    double payloadRcv;
    std::memcpy(&payloadRcv, buffer, sizeof(payloadRcv));
    delete[] buffer;
    NS_LOG_INFO("Received packet payload: " << payloadRcv);
    m_gymEnv->SetEnrgyLevel(payloadRcv);

    // We will only act just before reply, when all Gateways will have received
    // the packet, since we need their respective received power.
}

void
AdrRL::BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << status << networkStatus);

    Ptr<Packet> myPacket = status->GetLastPacketReceivedFromDevice()->Copy();
    LorawanMacHeader mHdr;
    LoraFrameHeader fHdr;
    fHdr.SetAsUplink();
    myPacket->RemoveHeader(mHdr);
    myPacket->RemoveHeader(fHdr);

    // Execute the Adaptive Data Rate (ADR) algorithm only if the request bit is set
    if (fHdr.GetAdr())
    {
        if (int(status->GetReceivedPacketList().size()) < historyRange)
        {
            NS_LOG_ERROR("Not enough packets received by this device ("
                        << status->GetReceivedPacketList().size()
                        << ") for the algorithm to work (need " << historyRange << ")");
        }
        else
        {
            NS_LOG_DEBUG("New Adaptive Data Rate (ADR) request");

            // Get the spreading factor used by the device
            uint8_t spreadingFactor = status->GetFirstReceiveWindowSpreadingFactor();

            // Get the device transmission power (dBm)
            uint8_t transmissionPower = status->GetMac()->GetTransmissionPower();

            // Getting the energy level of the device
            // double energyLevel = status->GetEnergyLevel();
            double energyLevel = 100.0;

            // Gettintg the SNR from the gateways
            double m_SNR = 0;
            double m_Rssi = 0;
            switch (historyAveraging)
            {
            case AdrRL::AVERAGE:
                m_SNR = GetAverageSNR(status->GetReceivedPacketList(), historyRange);
                break;
            case AdrRL::MAXIMUM:
                m_SNR = GetMaxSNR(status->GetReceivedPacketList(), historyRange);
                break;
            case AdrRL::MINIMUM:
                m_SNR = GetMinSNR(status->GetReceivedPacketList(), historyRange);
            }

            // Sending the parameters for Gym environment
            m_gymEnv->SetRssiSnr(m_Rssi, m_SNR);
            m_gymEnv->SetDr(SfToDr(spreadingFactor));
            m_gymEnv->SetTxPower(GetTxPowerIndex(transmissionPower));
            m_gymEnv->SetEnergyLevel(energyLevel);

            // New parameters for the end-device
            uint8_t newDataRate = m_gymEnv->GetNewDr();
            uint8_t newTxPower = m_gymEnv->GetNewTxPower();

            // Change the power back to the default if we don't want to change it
            if (!m_toggleTxPower)
            {
                newTxPower = transmissionPower;
            }

            if (newDataRate != SfToDr(spreadingFactor) || newTxPower != transmissionPower)
            {
                // Create a list with mandatory channel indexes
                int channels[] = {0, 1, 2};
                std::list<int> enabledChannels(channels, channels + sizeof(channels) / sizeof(int));

                // Repetitions Setting
                const int rep = 1;

                NS_LOG_DEBUG("Sending LinkAdrReq with DR = " << (unsigned)newDataRate
                                                            << " and TP = " << (unsigned)newTxPower
                                                            << " dBm");

                status->m_reply.frameHeader.AddLinkAdrReq(newDataRate,
                                                        GetTxPowerIndex(newTxPower),
                                                        enabledChannels,
                                                        rep);
                status->m_reply.frameHeader.SetAsDownlink();
                status->m_reply.macHeader.SetMType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);

                status->m_reply.needsReply = true;
            }
            else
            {
                NS_LOG_DEBUG("Skipped request");
            }
        }
    }
    else
    {
        // Do nothing
    }
}

void
AdrRL::OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << networkStatus);
}

uint8_t
AdrRL::SfToDr(uint8_t sf)
{
    switch (sf)
    {
    case 12:
        return 0;
        break;
    case 11:
        return 1;
        break;
    case 10:
        return 2;
        break;
    case 9:
        return 3;
        break;
    case 8:
        return 4;
        break;
    default:
        return 5;
        break;
    }
}

double
AdrRL::RxPowerToSNR(double rssi) const
{
    // The following conversion ignores interfering packets
    return rssi + 174 - 10 * log10(B) - NF;
}

// Get the maximum received power (it considers the values in dB!)
double
AdrRL::GetRSSIFromGateways(EndDeviceStatus::GatewayList gwList)
{
    auto it = gwList.begin();
    double max = it->second.rxPower;

    for (; it != gwList.end(); it++)
    {
        if (it->second.rxPower > max)
        {
            max = it->second.rxPower;
        }
    }

    return max;
}

double
AdrRL::GetMinSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange)
{
    double m_SNR;

    // Take elements from the list starting at the end
    auto it = packetList.rbegin();
    double min = RxPowerToSNR(GetRSSIFromGateways(it->second.gwList));

    for (int i = 0; i < historyRange; i++, it++)
    {
        m_SNR = RxPowerToSNR(GetRSSIFromGateways(it->second.gwList));

        NS_LOG_DEBUG("Received power: " << GetRSSIFromGateways(it->second.gwList));
        NS_LOG_DEBUG("m_SNR = " << m_SNR);

        if (m_SNR < min)
        {
            min = m_SNR;
        }
    }

    NS_LOG_DEBUG("SNR (min) = " << min);

    return min;
}

double
AdrRL::GetMaxSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange)
{
    double m_SNR;

    // Take elements from the list starting at the end
    auto it = packetList.rbegin();
    double max = RxPowerToSNR(GetRSSIFromGateways(it->second.gwList));

    for (int i = 0; i < historyRange; i++, it++)
    {
        m_SNR = RxPowerToSNR(GetRSSIFromGateways(it->second.gwList));

        NS_LOG_DEBUG("Received power: " << GetRSSIFromGateways(it->second.gwList));
        NS_LOG_DEBUG("m_SNR = " << m_SNR);

        if (m_SNR > max)
        {
            max = m_SNR;
        }
    }

    NS_LOG_DEBUG("SNR (max) = " << max);

    return max;
}

double
AdrRL::GetAverageSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange)
{
    double sum = 0;
    double m_SNR;

    // Take elements from the list starting at the end
    auto it = packetList.rbegin();
    for (int i = 0; i < historyRange; i++, it++)
    {
        m_SNR = RxPowerToSNR(GetRSSIFromGateways(it->second.gwList));

        NS_LOG_DEBUG("Received power: " << GetRSSIFromGateways(it->second.gwList));
        NS_LOG_DEBUG("m_SNR = " << m_SNR);

        sum += m_SNR;
    }

    double average = sum / historyRange;

    NS_LOG_DEBUG("SNR (average) = " << average);

    return average;
}

int
AdrRL::GetTxPowerIndex(int txPower)
{
    if (txPower >= 16)
    {
        return 0;
    }
    else if (txPower >= 14)
    {
        return 1;
    }
    else if (txPower >= 12)
    {
        return 2;
    }
    else if (txPower >= 10)
    {
        return 3;
    }
    else if (txPower >= 8)
    {
        return 4;
    }
    else if (txPower >= 6)
    {
        return 5;
    }
    else if (txPower >= 4)
    {
        return 6;
    }
    else
    {
        return 7;
    }
}
} // namespace lorawan
} // namespace ns3
