#include "common.h"

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
        // printDiagnostic();
        updateACState();
        updateHomekitState();
        return true;
    }

    void loop() {

    }

    void updateHomekitState() {
        curState->setVal(hp.getOperating() ? 2 : 1);
        speed->setVal(roundFanSpeed(speed->getNewVal())); // Tell homekit we're rounding
    }

    void updateACState() {
        /* Fan speed: 1-4, AUTO, or QUIET */
        if (!active->getNewVal() && !tarState->getNewVal()) {
            hp.setFanSpeed("QUIET");
            return;
        } 
        
        if (tarState->getNewVal() == 1) {
            hp.setFanSpeed("AUTO");
            return;
        } 
        
        int speed0_4 = roundFanSpeed(speed->getNewVal());
        const char *fanSpeed = speed0_4 != 0 ? String(speed0_4).c_str() : "QUIET";
        hp.setFanSpeed(fanSpeed);

        hp.update();
    }

    int roundFanSpeed(double speed) {
        return floor((speed / 25) + 0.5);
    }

    void printDiagnostic() {
        Serial.println();

        Serial.print("Active: ");
        Serial.println(active->getNewVal() ? "On" : "Off");

        Serial.print("Speed: ");
        Serial.println(speed->getNewVal());

        Serial.print("TarState: ");
        Serial.println(tarState->getNewVal() ? "Manual" : "Auto");
    }
};