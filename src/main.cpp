#include "HomeSpan.h"

#define LED 2

double temp;

struct HeatPump : Service::HeaterCooler
{
    SpanCharacteristic *active;
    SpanCharacteristic *curTemp;
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SpanCharacteristic *displayUnits;
    SpanCharacteristic *coolingThresholdTemp;
    SpanCharacteristic *heatingThresholdTemp;

    HeatPump() : Service::HeaterCooler()
    { 
        active = new Characteristic::Active(1); // 1 is active 0 is inactive
        curTemp = new Characteristic::CurrentTemperature(21.11 /* 70 degrees */); // init current device temp here
        curState = new Characteristic::CurrentHeaterCoolerState(1); // 0 inactive, 1 idle, 2 heating, 3 cooling
        tarState = new Characteristic::TargetHeaterCoolerState(0); // 0 auto, 1 heating, 2 cooling, 3 off
        coolingThresholdTemp = new Characteristic::CoolingThresholdTemperature(22);
        heatingThresholdTemp = new Characteristic::HeatingThresholdTemperature(18);
    }

    boolean update()
    { 
        Serial.println();

        Serial.print("Active: ");
        Serial.println(active->getNewVal() ? "On" : "Off");

        Serial.print("tarState: ");
        switch (tarState->getNewVal())
        {
        case 0:
            Serial.println(active->getNewVal() ? "Auto" : "Off");
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

        // update homekit
        int state = tarState->getNewVal();
        curState->setVal(1); // Set state to idle since we're at the target temp

        temp = curTemp->getVal();
        if (state == 1){
            temp = heatingThresholdTemp->getNewVal();
        } else if (state == 2) {
            temp = coolingThresholdTemp->getNewVal();
        }
        curTemp->setVal(temp);

        return true;
    }

    void loop() {
        // update homekit
        if (curTemp->getVal() != temp)
            curTemp->setVal(temp);
    }
};

struct HeatPumpFan : Service::Fan
{
    SpanCharacteristic *active;
    SpanCharacteristic *speed;
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;

    HeatPumpFan() : Service::Fan()
    { 
        active = new Characteristic::Active(1);
        speed = new Characteristic::RotationSpeed(); // 0-100
        curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing air
        tarState = new Characteristic::TargetFanState(); // 0 manual, 1 auto (home app thinks this is on or off)
    }

    boolean update()
    { 
        Serial.println();

        Serial.print("Active: ");
        Serial.println(active->getNewVal() ? "On" : "Off");

        Serial.print("Speed: ");
        Serial.println(speed->getNewVal());

        Serial.print("TarState: ");
        Serial.println(tarState->getNewVal());

        // update homekit
        curState->setVal(2);

        return true;
    }
};

struct HeatPumpSlats : Service::Slat
{
    SpanCharacteristic *curState;
    SpanCharacteristic *type;
    SpanCharacteristic *swingMode;
    SpanCharacteristic *curAngle;
    SpanCharacteristic *tarAngle;

    HeatPumpSlats() : Service::Slat()
    { 
        // init heatpump stuff here
        curState = new Characteristic::CurrentSlatState(0); // 0 fixed, 1 jammed, 2 swinging
        type = new Characteristic::SlatType(1); // 0 horizontal, 1 vertical
        swingMode = new Characteristic::SwingMode(0); // 0 disabled, 1 enabled
        curAngle = new Characteristic::CurrentTiltAngle(0); // -90 to 90 where -90 is straight out and 90 is straight down
        tarAngle = new Characteristic::TargetTiltAngle(0);
    }

    boolean update()
    { 
        Serial.println();

        Serial.print("swingMode: ");
        Serial.println(swingMode->getNewVal() ? "On" : "Off");

        Serial.print("curAngle: ");
        Serial.println(curAngle->getNewVal());

        Serial.print("tarAngle: ");
        Serial.println(tarAngle->getNewVal());

        // update homekit
        curState->setVal(0); // fixed
        curAngle->setVal(tarAngle->getNewVal());

        return true;
    }
};

void setup()
{
    Serial.begin(115200);
    homeSpan.enableOTA();
    homeSpan.setStatusPin(LED);
    homeSpan.begin(Category::AirConditioners, "Mitsubishi Split System");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name("Split System Heat Pump");
            new Characteristic::Manufacturer("Mitsubishi Electric Company");
            new Characteristic::SerialNumber("88N11479");  
            new Characteristic::Model("MSZ-GL12NA");
            new Characteristic::FirmwareRevision("0.1");
            new Characteristic::Identify();

        new Service::HAPProtocolInformation();
            new Characteristic::Version("2.2.0");

        (new HeatPump())->setPrimary();
        new HeatPumpFan();
        new HeatPumpSlats();
}

void loop()
{
    homeSpan.poll();
}