#include "rl_adr_env.h"

namespace ns3
{
namespace lorawan
{   

NS_LOG_COMPONENT_DEFINE("AiAdrEnv");
NS_OBJECT_ENSURE_REGISTERED(LorawanGymEnv);

LorawanGymEnv::LorawanGymEnv()
{
    NS_LOG_FUNCTION (this);
}

LorawanGymEnv::~LorawanGymEnv()
{
    NS_LOG_FUNCTION (this);
}

TypeId
LorawanGymEnv::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::LorawanGymEnv")
        .SetParent<OpenGymEnv> ()
        .SetGroupName ("OpenGym")
        .AddConstructor<LorawanGymEnv> ()
    ;
    return tid;
}

void
LorawanGymEnv::DoDispose ()
{
    NS_LOG_FUNCTION (this); 
}

void
LorawanGymEnv::SetNodeId(uint32_t id)
{
    NS_LOG_FUNCTION (this);
    m_nodeId = id;
}

void
LorawanGymEnv::SetReward(float value)
{
    NS_LOG_FUNCTION (this);
    m_Reward = value;
}

void 
LorawanGymEnv::SetPenalty(float value)
{
    NS_LOG_FUNCTION (this);
    m_penalty = value;
}

Ptr<OpenGymSpace>
LorawanGymEnv::GetActionSpace()
{
    // space for new data rate and new transmission power
    uint8_t numParameters = 2;
    uint8_t lowDataRate = 0;
    uint8_t highDataRate = 5;
    uint8_t lowTxPower = 0;
    uint8_t highTxPower = 7;

    // Define the OPen Gym Box space 
    std::vector<uint32_t> shape = {numParameters,};
    std::vector<uint8_t> low = {lowDataRate, lowTxPower};
    std::vector<uint8_t> high = {highDataRate, highTxPower};
    std::string dtype = TypeNameGet<uint8_t> ();
    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);    

    NS_LOG_INFO (this << "LoRaWAN action space: " << box);

    return box;
    // This returns the pointer to the object containing the 2 parameters
    // Example: BoxSpace Low: [0, 0] High: [5, 7] Shape: (2,)
}

bool
LorawanGymEnv::GetGameOver()
{
    NS_LOG_FUNCTION (this);
    bool isGameOver = false;

    // Checking if the SNR value is less than the recommended thresholds by Semtech 
    if (m_Snr <= threshold[m_Dr]) {
        isGameOver = true;
    }
    
    if (m_EnergyLevel <= 0.0) {
        isGameOver = true;
    }
    
    // Also we can check whether there are too much packet loss or the SNR vlaue is too low

    return m_isGameOver;    
}

float
LorawanGymEnv::GetReward()
{
    NS_LOG_FUNCTION (this);
    return m_Reward;
}

std::string
LorawanGymEnv::GetExtraInfo()
{
    NS_LOG_FUNCTION (this);
    return m_info;
}

bool
LorawanGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
    Ptr<OpenGymBoxContainer<uint32_t> > box = DynamicCast<OpenGymBoxContainer<uint32_t> >(action);
    uint8_t m_newDr = box->GetValue(0);
    uint8_t m_newTxPower = box->GetValue(1);
    
    NS_LOG_INFO ("MyExecuteActions: " << action);
    return true;
}

Ptr<OpenGymSpace>
LorawanGymEnv::GetObservationSpace()
{
    NS_LOG_FUNCTION (this);
    
    uint32_t parameterNum = 6;
    double lowRssi = -200.0;
    double highRssi = 14.0;
    double lowSnr = -20.0;
    double highSnr = 10.0;
    double lowDr = 0.0;
    double highDr = 5.0;
    double lowTxPower = 0.0;
    double highTxPower = 7.0;
    double lowEnergyLevel = 0.0;
    double highEnergyLevel = 100.0;
    double lowCodeRate = 0.0;    
    double highCodeRate = 5.0;

    // Define the Open Gym Box space
    std::vector<double> low = {lowRssi, lowSnr, lowDr, lowTxPower, lowEnergyLevel, lowCodeRate};
    std::vector<double> high = {highRssi, highSnr, highDr, highTxPower, highEnergyLevel, highCodeRate};
    std::vector<uint32_t> shape = {parameterNum,};
    std::string dtype = TypeNameGet<uint32_t> ();

    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    NS_LOG_INFO ("MyGetObservationSpace: " << box);
    return box; 
}

Ptr<OpenGymDataContainer>
LorawanGymEnv::GetObservation()
{
    NS_LOG_FUNCTION (this);
    uint32_t parameterNum = 6;
    std::vector<uint32_t> shape = {parameterNum,};

    Ptr<OpenGymBoxContainer<uint64_t> > box = CreateObject<OpenGymBoxContainer<uint64_t> >(shape);

    // by default the datatype of some values are not double, so we need to cast them to double before adding them to the box
    box->AddValue(m_Rssi);
    box->AddValue(m_Snr);
    box->AddValue(static_cast<double>(m_Dr));  
    box->AddValue(static_cast<double>(m_TxPower));
    box->AddValue(m_EnergyLevel);
    box->AddValue(static_cast<double>(m_codeRate));

    NS_LOG_INFO ("MyGetObservation: " << box);
    return box; 
}

void LorawanGymEnv::UplinkPktTrace()
{
    NS_LOG_FUNCTION (this);
}

void 
LorawanGymEnv::SetRssiSnr(double rssi, double snr)
{
    NS_LOG_FUNCTION (this);
    m_Rssi = rssi;
    m_Snr = snr;
}

void LorawanGymEnv::SetDr(uint8_t dr)
{
    NS_LOG_FUNCTION (this);
    m_Dr = dr;
}

void LorawanGymEnv::SetTxPower(uint8_t txPower)
{
    NS_LOG_FUNCTION (this);
    m_TxPower = txPower;
}

void LorawanGymEnv::SetEnergyLevel(double energyLevel)
{
    NS_LOG_FUNCTION (this);
    m_EnergyLevel = energyLevel;
}

uint8_t LorawanGymEnv::GetNewDr()
{
    NS_LOG_FUNCTION (this);
    return m_newDr;
}

uint8_t LorawanGymEnv::GetNewTxPower()
{
    NS_LOG_FUNCTION (this);
    return m_newTxPower;
}

} // namespace lorawan
} // namespace ns3
