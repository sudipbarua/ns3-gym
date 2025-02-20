
#ifndef ADR_RL_H
#define ADR_RL_H

#include "network-controller-components.h"
#include "network-status.h"

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * LinkAdrRequest commands management
 */
class AdrRL : public NetworkControllerComponent
{
    /**
     * Available policies for combining radio metrics in packet history.
     */
    enum CombiningMethod
    {
        AVERAGE,
        MAXIMUM,
        MINIMUM,
    };

public:
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    AdrRL();           //!< Default constructor
    ~AdrRL() override; //!< Destructor

    void OnReceivedPacket(Ptr<const Packet> packet,
                        Ptr<EndDeviceStatus> status,
                        Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

private:
    /**
     * ADR is meant to optimize radio modulation parameters of end devices to improve energy
     * consuption and radio resource utilization.  
     *
     * \param newDataRate [out] new data rate value selected for the end device.
     * \param newTxPower [out] new tx power value selected for the end device.
     * \param status State representation of the current end device.
     */
    void AdrImplementation(uint8_t* newDataRate, uint8_t* newTxPower, Ptr<EndDeviceStatus> status);

    /**
     * Convert spreading factor values [7:12] to respective data rate values [0:5].
     *
     * \param sf The spreading factor value.
     * \return Value of the data rate as uint8_t.
     */
    uint8_t SfToDr(uint8_t sf);

    /**
     * Convert reception power values [dBm] to Signal to Noise Ratio (SNR) values [dB].
     *
     * The conversion comes from the formula \f$P_{rx}=-174+10\log_{10}(B)+SNR+NF\f$ where
     * \f$P_{rx}\f$ is the received transmission power, \f$B\f$ is the transmission bandwidth and
     * \f$NF\f$ is the noise figure of the receiver. The constant \f$-174\f$ is the thermal noise
     * [dBm] in 1 Hz of bandwidth and is influenced the temperature of the receiver, assumed
     * constant in this model. For more details see the SX1301 chip datasheet.
     *
     * \param transmissionPower Value of received transmission power.
     * \return SNR value as double.
     */
    double RxPowerToSNR(double transmissionPower) const;

    /**
     * Get the RSSI (dBm) from all the gateways receiving the same transmission.
     *
     * \param gwList List of gateways paired with reception information.
     * \return Min RSSI of transmission as double.
     */
    double GetRSSIFromGateways(EndDeviceStatus::GatewayList gwList);

    /**
     * Get the min Signal to Noise Ratio (SNR) of the receive packet history.
     *
     * \param packetList History of received packets with reception information.
     * \param historyRange Number of packets to consider going back in time.
     * \return Min SNR among packets as double.
     */
    double GetMinSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange);
    /**
     * Get the max Signal to Noise Ratio (SNR) of the receive packet history.
     *
     * \param packetList History of received packets with reception information.
     * \param historyRange Number of packets to consider going back in time.
     * \return Max SNR among packets as double.
     */
    double GetMaxSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange);
    /**
     * Get the average Signal to Noise Ratio (SNR) of the received packet history.
     *
     * \param packetList History of received packets with reception information.
     * \param historyRange Number of packets to consider going back in time.
     * \return Average SNR of packets as double.
     */
    double GetAverageSNR(EndDeviceStatus::ReceivedPacketList packetList, int historyRange);

    /**
     * Get the LoRaWAN protocol TXPower configuration index from the Equivalent Isotropically
     * Radiated Power (EIRP) in dBm.
     *
     * \param txPower Transission EIRP configuration.
     * \return TXPower index as int.
     */
    int GetTxPowerIndex(int txPower);

    enum CombiningMethod tpAveraging;      //!< TX power from gateways policy
    int historyRange;                      //!< Number of previous packets to consider
    enum CombiningMethod historyAveraging; //!< Received SNR history policy

    const int min_spreadingFactor = 7;    //!< Spreading factor lower limit
    const int min_transmissionPower = 2;  //!< Minimum transmission power (dBm) (Europe)
    const int max_transmissionPower = 14; //!< Maximum transmission power (dBm) (Europe)
    // const int offset = 10;                //!< Device specific SNR margin (dB)
    const int B = 125000; //!< Bandwidth (Hz)
    const int NF = 6;     //!< Noise Figure (dB)
    double threshold[6] = {
        -20.0,
        -17.5,
        -15.0,
        -12.5,
        -10.0,
        -7.5}; //!< Vector containing the required SNR for the 6 allowed spreading factor
            //!< levels ranging from 7 to 12 (the SNR values are in dB).

    bool m_toggleTxPower; //!< Whether to control transmission power of end devices or not
};
} // namespace lorawan
} // namespace ns3

#endif
