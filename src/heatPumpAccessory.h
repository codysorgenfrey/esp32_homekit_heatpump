#include "common.h"

struct HeatPumpAccessory : Service::HeaterCooler
{
    SpanCharacteristic *active;
    SpanCharacteristic *curTemp;
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SpanCharacteristic *displayUnits;
    SpanCharacteristic *coolingThresholdTemp;
    SpanCharacteristic *heatingThresholdTemp;
    HeatPump *hp;
    const char *acModes[4] = {
        "AUTO",
        "HEAT",
        "COOL",
        "AUTO" // supposed to be off, but will be handled by active state
    };

    HeatPumpAccessory(HeatPump *inHp) : Service::HeaterCooler()
    { 
        active = new Characteristic::Active(1); // 1 is active 0 is inactive
        curTemp = new Characteristic::CurrentTemperature(hp->getRoomTemperature()); // init current device temp here
        curState = new Characteristic::CurrentHeaterCoolerState(1); // 0 inactive, 1 idle, 2 heating, 3 cooling
        tarState = new Characteristic::TargetHeaterCoolerState(0); // 0 auto, 1 heating, 2 cooling, 3 off
        coolingThresholdTemp = new Characteristic::CoolingThresholdTemperature(22);
        heatingThresholdTemp = new Characteristic::HeatingThresholdTemperature(18);
        hp = inHp;
    }

    boolean update()
    { 
        // printDiagnostic();
        updateACState();
        updateHomekitState(hp->getRoomTemperature(), hp->getOperating());
        return true;
    }

    void loop() {
        updateHomekitState(hp->getRoomTemperature(), hp->getOperating());
    }

    void updateHomekitState(double roomTemp, bool operating) {
        // Tell homekit what AC is up to

        int state = tarState->getNewVal();
        double temp = hp->getTemperature();
        if (state == 0) // Auto
        {
            double avgTemp = ((heatingThresholdTemp->getNewVal() - coolingThresholdTemp->getNewVal()) / 2) + coolingThresholdTemp->getNewVal(); // set temp in the middle
            if (operating && (roomTemp < avgTemp)) // Heating
                curState->setVal(2); 
            else if (operating && (roomTemp > avgTemp)) // Cooling
                curState->setVal(3); 
            else // Idle
                curState->setVal(1); 

            if (avgTemp != temp) 
            {
                double diff = avgTemp - temp;
                heatingThresholdTemp->setVal(heatingThresholdTemp->getNewVal() + diff);
                coolingThresholdTemp->setVal(coolingThresholdTemp->getNewVal() + diff);
            }
        } 
        else if (state == 1) // Heating
        { 
            if (operating) 
                curState->setVal(2); // Heating
            else 
                curState->setVal(1); // Idle

            if (heatingThresholdTemp->getNewVal() != temp)
                heatingThresholdTemp->setVal(temp);
        } 
        else if (state == 2) // Cooling
        { 
            if (operating) 
                curState->setVal(3); // Cooling
            else 
                curState->setVal(1); // Idle

            if (coolingThresholdTemp->getNewVal() != temp)
                coolingThresholdTemp->setVal(temp);
        }

        if (curTemp->getVal() != roomTemp) 
            curTemp->setVal(roomTemp);
    }

    void updateACState() {
        /* ON/OFF */
        hp->setPowerSetting(active->getNewVal() ? "ON" : "OFF");

        /* HEAT/COOL/FAN/DRY/AUTO */
        int state = tarState->getNewVal();
        hp->setModeSetting(acModes[state]);

        /* Temperature Between 16 and 31 */
        double temp;
        if (state == 1) // Heating
            temp = heatingThresholdTemp->getNewVal();
        else if (state == 2) // Cooling
            temp = coolingThresholdTemp->getNewVal();
        else // Auto
            temp = ((heatingThresholdTemp->getNewVal() - coolingThresholdTemp->getNewVal()) / 2) + coolingThresholdTemp->getNewVal(); // set temp in the middle
        
        temp = max(temp, 16.0);
        temp = min(temp, 31.0);
        hp->setTemperature(temp);

        hp->update();
    }

    void printDiagnostic() {
        Serial.println();

        Serial.print("Active: ");
        Serial.println(active->getNewVal() ? "On" : "Off");

        Serial.print("tarState: ");
        switch (tarState->getNewVal())
        {
        case 0:
            Serial.println("Auto");
            break;
        
        case 1:
            Serial.println("Heating");
            break;

        case 2:
            Serial.println("Cooling");
            break;

        case 3:
            Serial.println("Off");
            break;
        
        default:
            break;
        }

        Serial.print("coolingThresholdTemp: ");
        Serial.println(coolingThresholdTemp->getNewVal());

        Serial.print("heatingThresholdTemp: ");
        Serial.println(heatingThresholdTemp->getNewVal());
    }
};

