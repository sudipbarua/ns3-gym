#ifndef RL_ADR_ENV_H
#define RL_ADR_ENV_H

#include "ns3/opengym-module.h"

namespace ns3
{

    class LorawanGymEnv : public OpenGymEnv
    {
        public:
            LorawanGymEnv ();
            virtual ~LorawanGymEnv ();
            static TypeId GetTypeId (void);
            virtual void DoDispose ();
            
            void SetNodeId(uint32_t id);

            void SetReward(float value);
            void SetPenalty(float value);

            // Opengym interface 
            virtual Ptr<OpenGymSpace> GetActionSpace();
            virtual bool GetGameOver();
            virtual float GetReward();
            virtual std::string GetExtraInfo();
            virtual bool ExecuteActions(Ptr<OpenGymDataContainer> action);
            
            virtual Ptr<OpenGymSpace> GetObservationSpace();
            virtual Ptr<OpenGymDataContainer> GetObservation();

            // Packet tracing and data/state/observation fetching interfaces
            virtual void UplinkPktTrace();
            // virtual void 



        protected:
            uint32_t m_nodeId;

            bool m_isGameOver;
            float m_Reward;
            float m_penalty;
            std::string m_info;

            // State/Observations
            double m_Rssi;
            double m_Snr;
            uint8_t m_Dr;
            uint8_t m_TxPower;
            double m_EnergyLevel;
            // double m_DistanceToGateway;
            uint8_t m_codeRate;
            
            // Actions
            uint8_t m_newDr;
            uint8_t m_newTxPower;
            double threshold[6] = {
                -20.0,
                -17.5,
                -15.0,
                -12.5,
                -10.0,
                -7.5}; //!< Vector containing the required SNR for the 6 allowed spreading factor
    };

}
#endif /* RL_ADR_ENV_H */