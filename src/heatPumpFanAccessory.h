#include "common.h"

struct HeatPumpFanAccessory : Service::Fan
{
    SpanCharacteristic *active;
    SpanCharacteristic *speed;
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    HeatPump *hp;

    HeatPumpFanAccessory(HeatPump *inHp) : Service::Fan()
    { 
        hp = inHp;
        active = new Characteristic::Active(1);
        speed = new Characteristic::RotationSpeed(); // 0-100
        curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing air
        tarState = new Characteristic::TargetFanState(); // 0 manual, 1 auto (home app thinks this is on or off)
    }

    boolean update()
    { 
        #if DEBUG_HOMEKIT
            printDiagnostic();
        #endif

        bool success = updateACState();
        updateHomekitState();
        
        #if TESTING_HP
            return true;
        #else
            return success;
        #endif
    }

    void loop() {
        updateHomekitState();
    }

    void updateHomekitState() {
        curState->setVal(hp->getOperating() ? 2 : 1);
        speed->setVal(roundFanSpeed(speed->getNewVal()) * 25);
    }

    bool updateACState() {
        /* Fan speed: 1-4, AUTO, or QUIET */
        if (!active->getNewVal() && !tarState->getNewVal()) {
            hp->setFanSpeed("QUIET");
            return hp->update();
        } 
        
        if (tarState->getNewVal() == 1) {
            hp->setFanSpeed("AUTO");
            return hp->update();
        } 
        
        int speed0_4 = roundFanSpeed(speed->getNewVal());
        const char *fanSpeed = speed0_4 != 0 ? String(speed0_4).c_str() : "QUIET";
        hp->setFanSpeed(fanSpeed);

        return hp->update();
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