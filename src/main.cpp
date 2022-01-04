#include "heatPumpAccessory.h"
#include "heatPumpFanAccessory.h"
#include "heatPumpSlatsAccessory.h"

HeatPump hp;

void setup()
{
    Serial.begin(115200);

    homeSpan.enableOTA();
    homeSpan.setStatusPin(LED);
    homeSpan.begin(Category::AirConditioners, NAME, MANUFACTURER, MODEL);

    hp.connect(&Serial1);
    hp.enableExternalUpdate();
    hp.setSettings({ // Set some defaults
        "ON",  /* ON/OFF */
        "AUTO", /* HEAT/COOL/FAN/DRY/AUTO */
        20,    /* Between 16 and 31 */
        "AUTO",   /* Fan speed: 1-4, AUTO, or QUIET */
        "AUTO",   /* Air direction (vertical): 1-5, SWING, or AUTO */
        "|"    /* Air direction (horizontal): <<, <, |, >, >>, <>, or SWING */
    });
    Serial.println(hp.update() ? "Update HP success" : "Update HP failed");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(NAME);
            new Characteristic::Manufacturer(MANUFACTURER);
            new Characteristic::SerialNumber(SERIALNUM);  
            new Characteristic::Model(MODEL);
            new Characteristic::FirmwareRevision("0.1");
            new Characteristic::Identify();

        new Service::HAPProtocolInformation();
            new Characteristic::Version("2.2.0");

        (new HeatPumpAccessory(&hp))->setPrimary();
        new HeatPumpFanAccessory(&hp);
        new HeatPumpSlatsAccessory(&hp);
}

void loop()
{
    homeSpan.poll();

    #if !TESTING_HP
        hp.sync();
    #endif
}