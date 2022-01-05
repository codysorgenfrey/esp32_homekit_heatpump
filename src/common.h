#include "HomeSpan.h"
#include "HeatPump.h"

#define LED 2
#define SKETCH_VER "0.0.1"

#define MANUFACTURER "Mitsubishi-Electric"
#define NAME "Split-System-Heat-Pump"
#define MODEL "MSZ-GL12NA"
#define SERIALNUM "88N11479"
#define UNIQUE_NAME "88N11479-Split-System-Heat-Pump"

#define DEFAULT_HEAT_THRESH 18.8889 // 66 Deg F
#define DEFAULT_COOL_THRESH 23.3333 // 74 Deg F
#define HK_UPDATE_TIMER 15 // Update homekit every X seconds

#define TESTING_HP true
#define DEBUG_HOMEKIT false