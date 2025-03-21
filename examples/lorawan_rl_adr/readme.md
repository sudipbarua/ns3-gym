# LoRaWAN Adaptive Datarate mechanism using Reinforcement Learning  

## Observations

## Rewards

## Actions 

## Interfacing

## Battery information collection form ED
We have developed a new application for this purpose. 

**Usage:**

Configuring and installation 
````C++
for (uint32_t i = 0; i < endDevices.GetN(); ++i)
     {
         Ptr<BatterySender> app = CreateObject<BatterySender>();
         app->SetEnergySource(sources.Get(i));
         app->SetSendInterval(Seconds(60));
         endDevices.Get(i)->AddApplication(app);
         app->SetStartTime(Seconds(0));
         app->SetStopTime(Hours(24));
     }
````
Extracting the content
````C++    
    uint32_t packetSize = packet->GetSize();
    uint8_t *buffer = new uint8_t[packetSize];
    packet->CopyData(buffer, packetSize);
    double payloadRcv;
    std::memcpy(&payloadRcv, buffer, sizeof(payloadRcv));
    delete[] buffer;
````