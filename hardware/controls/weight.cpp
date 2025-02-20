#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "../../../WiringPi/wiringPi/wiringPi.h"
#include "HX711.h"

#define LOAD_CELL_DOUT 6 // HX711 Data pin
#define LOAD_CELL_SCK  5 // HX711 Clock pin

HX711 scale;

/*
 * Initializes GPIO, hardware and sensors.
 * If compiled with gcc needs link
 *      Ex: gcc -o myapp myapp.c -l wiringPi
 * @return None
 */
void weightSetup() {
  LOG(INFO) << "Weight system initialization...";
  wiringPiSetupGpio();

  /**
   * Setup HX711 Weight Sensor these are from the HX711 Arduino Library.
   * These need to be either rewritten or a new library needs to be found.
   *
   * scale.begin(LOAD_CELL_DOUT, LOAD_CELL_SCK);
   * scale.set_scale(0.0); // Adjustable calibration factor (varies +-250k)
   * scale.tare();         // Zero the scale
   */
}
/*
 * Check if there is weight on the platform.
 *
 *
 * @return boolean
 * 0 - nothing on scale
 * 1 - valid input, begin scanning
 */
int getWeight() {
  float weight = scale.get_units(10);
  LOG(INFO) << "Reading Weight: " << weight << " g";
  return weight;
}