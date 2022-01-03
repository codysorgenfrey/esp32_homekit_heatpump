#include "heatpump.h"
#include "heatpumpfan.h"
#include "heatpumpslats.h"

void setup()
{
    hp.connect(&Serial);
    hp.enableExternalUpdate();

    Serial.begin(115200);

    homeSpan.enableOTA();
    homeSpan.setStatusPin(LED);
    homeSpan.begin(Category::AirConditioners, NAME, MANUFACTURER, MODEL);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(NAME);
            new Characteristic::Manufacturer(MANUFACTURER);
            new Characteristic::SerialNumber(SERIAL);  
            new Characteristic::Model(MODEL);
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
    hp.sync();
}