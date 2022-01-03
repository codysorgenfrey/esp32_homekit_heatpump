#include "common.h"

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
        // printDiagnostic();
        updateACState();
        updateHomekitState();
        return true;
    }

    void loop() {

    }

    void updateACState() {
        hp.update();
    }

    void updateHomekitState() {

    }

    void printDiagnostic() {
        Serial.println();

        Serial.print("swingMode: ");
        Serial.println(swingMode->getNewVal() ? "On" : "Off");

        Serial.print("curAngle: ");
        Serial.println(curAngle->getNewVal());

        Serial.print("tarAngle: ");
        Serial.println(tarAngle->getNewVal());
    }
};