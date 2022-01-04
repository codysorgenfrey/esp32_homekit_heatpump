#include "HomeSpan.h"
#include "HeatPump.h"

#define LED 2

#define MANUFACTURER "Mitsubishi-Electric"
#define NAME "Split-System-Heat-Pump"
#define MODEL "MSZ-GL12NA"
#define SERIALNUM "88N11479"

#define DEFAULT_HEAT_THRESH 18.8889 // 66 Deg F
#define DEFAULT_COOL_THRESH 23.3333 // 74 Deg F

#define TESTING_HP true
#define DEBUG_HOMEKIT false