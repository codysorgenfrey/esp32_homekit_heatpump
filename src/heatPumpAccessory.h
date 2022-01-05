#include "common.h"

struct HeatPumpAccessory : Service::HeaterCooler
{
    SpanCharacteristic *active;
    SpanCharacteristic *roomTemp;
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SpanCharacteristic *displayUnits;
    SpanCharacteristic *coolingThresholdTemp;
    SpanCharacteristic *heatingThresholdTemp;
    HeatPump *hp;
    const char *hpModes[4] = {
        "AUTO",
        "HEAT",
        "COOL",
        "OFF"
    };

    HeatPumpAccessory(HeatPump *inHp) : Service::HeaterCooler()
    { 
        hp = inHp;
        active = new Characteristic::Active();
        curState = new Characteristic::CurrentHeaterCoolerState();                // 0 inactive, 1 idle, 2 heating, 3 cooling
        tarState = new Characteristic::TargetHeaterCoolerState();                 // 0 auto, 1 heating, 2 cooling, 3 off
        coolingThresholdTemp = new Characteristic::CoolingThresholdTemperature(); // 10-35
        heatingThresholdTemp = new Characteristic::HeatingThresholdTemperature(); // 0-25
        roomTemp = new Characteristic::CurrentTemperature();                      // current device temp

        #if !TESTING_HP
            updateHomekitState();
        #endif
    }

    boolean update()
    { 
        #if DEBUG_HOMEKIT
            printDiagnostic();
        #endif
        
        #if TESTING_HP
            updateHeatPumpState();
            return true;
        #else
            return updateHeatPumpState();
        #endif
    }

    void loop() {
        #if !TESTING_HP
            if (millis() % (HK_UPDATE_TIMER * 1000) == 0)
                updateHomekitState();
        #endif
    }

    void updateHomekitState() {
        //////////////////////////////////////////////////////////////////
        // Get variables from Heat Pump & Homekit                       //
        //////////////////////////////////////////////////////////////////
        
        double hpTemp = hp->getTemperature();
        double hpRoomTemp = hp->getRoomTemperature();

        int hpTarState;
        for (hpTarState = 0; hpTarState < 3; hpTarState += 1) { // has to be 1 less then count since we use hpTarState later
            if (hpModes[hpTarState] == hp->getModeSetting())
                break;
        }
        if (hpTarState == 3) hpTarState = 0; // Unsupported modes, setting Homekit to auto
        
        int hpCurState;
        double hkTemp;
        switch (hpTarState)
        {
        case 1: // Heating
            hkTemp = heatingThresholdTemp->getVal();
            hpCurState = 2;
            break;

        case 2: // Cooling
            hkTemp = coolingThresholdTemp->getVal();
            hpCurState = 3;
            break;
        
        default: // Auto and Off
            hkTemp = ((heatingThresholdTemp->getVal() - coolingThresholdTemp->getVal()) / 2) + coolingThresholdTemp->getVal(); // set temp in the middle
            hpCurState = hkTemp >= hpRoomTemp ? 2 : 3;
            break;
        }
        
        if (!hp->getOperating() && !(hpCurState == 0)) // Idle
            hpCurState = 1;

        //////////////////////////////////////////////////////////////////
        // Update homekit if things don't match                         //
        //////////////////////////////////////////////////////////////////

        // Active
        if (active->getVal() != hp->getPowerSettingBool())
            active->setVal(hp->getPowerSettingBool());

        // Mode
        if (hpModes[hpTarState] != hp->getModeSetting())
            tarState->setVal(hpTarState);

        // State
        if (hpCurState != curState->getVal())
            curState->setVal(hpCurState);

        // Temperature
        if (hkTemp != hpTemp)
            switch (hpTarState)
            {
            case 1: // Heating
                heatingThresholdTemp->setVal(hpTemp);
                break;

            case 2: // Cooling
                coolingThresholdTemp->setVal(hpTemp);
                break;
            
            default: // Auto and Off
                double diff = hpTemp - hkTemp;
                coolingThresholdTemp->setVal(coolingThresholdTemp->getVal() + diff);
                heatingThresholdTemp->setVal(heatingThresholdTemp->getVal() + diff);
                break;
            }

        // Room temperature
        if (roomTemp->getVal() != hp->getRoomTemperature()) 
            roomTemp->setVal(hp->getRoomTemperature());
    }

    bool updateHeatPumpState() {
        // Active
        hp->setPowerSetting(active->getNewVal() ? "ON" : "OFF");

        // Mode
        int hkTarState = tarState->getNewVal();
        if (hkTarState == 3) hkTarState = 0; // If set to off, set to auto
        hp->setModeSetting(hpModes[hkTarState]);

        // Temperature
        double hkTemp;
        switch (hkTarState)
        {
        case 1: // Heating
            hkTemp = heatingThresholdTemp->getNewVal();
            break;

        case 2: // Cooling
            hkTemp = coolingThresholdTemp->getNewVal();
            break;
        
        default: // Auto and Off
            hkTemp = ((heatingThresholdTemp->getNewVal() - coolingThresholdTemp->getNewVal()) / 2) + coolingThresholdTemp->getNewVal(); // set temp in the middle
            break;
        }
        
        /* Temperature Between 16 and 31 */
        hkTemp = max(hkTemp, 16.0);
        hkTemp = min(hkTemp, 31.0);
        
        hp->setTemperature(hkTemp);

        return hp->update();
    }

    void printDiagnostic() {
        Serial.println();

        Serial.print("Active: ");
        Serial.println(active->getNewVal() ? "On" : "Off");

        Serial.print("tarState: ");
        Serial.println(hpModes[tarState->getNewVal()]);

        Serial.print("coolingThresholdTemp: ");
        Serial.println(coolingThresholdTemp->getNewVal());

        Serial.print("heatingThresholdTemp: ");
        Serial.println(heatingThresholdTemp->getNewVal());
    }
};

