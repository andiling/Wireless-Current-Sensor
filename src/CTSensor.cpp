#include "CTSensor.h"

//------------------------------------------
// Function to call when a new Current value has been received from CTSensorReader
void CTSensor::NewIFromCTSensor(float Irec) {

  unsigned long _newIrecMillis = millis();

  //initialized ?
  if (_firstReadDone) {
    //calculate average of I (time dependant) (ESP go too much faster and at boot can call this function 2 time in the same millisecond)
    if (_newIrecMillis != _startMillis) _avgI = ((_avgI * (_lastIrecMillis - _startMillis)) + (_currentI * (_newIrecMillis - _lastIrecMillis))) / (_newIrecMillis - _startMillis);
    else _avgI = 0.0;
  }
  else {
    //init
    _startMillis = _newIrecMillis;
    _firstReadDone = true;
  }

  _currentI = Irec;
  _lastIrecMillis = _newIrecMillis;
}

//------------------------------------------
// Function to get Energy Counter updated and returned
unsigned long CTSensor::GetCounterUpdated() {

  //trigger calculation of AverageI
  NewIFromCTSensor(_currentI);
  //calculate Energy used in the mean time
  float consumedEnergy = ((_avgI * LIVE_VOLTAGE) * (_lastIrecMillis - _startMillis)) / 3600000.0;
  //add previous decimal value
  consumedEnergy += decimalLeft;
  //increase counter (integer)
  _counter += consumedEnergy;
  //save decimal value
  decimalLeft = consumedEnergy - ((unsigned long)consumedEnergy);

  //AND reset timer (current average value will be ignored in next NewIFromCTSensor)
  _startMillis = _lastIrecMillis;

  return _counter;
}
//------------------------------------------
// Function to return currentI (for status infos)
float CTSensor::GetCurrentI(){
  return _currentI;
}
//------------------------------------------
// Function to return averageI (for status infos)
float CTSensor::GetAverageI() {
  return _avgI;
}
//------------------------------------------
// Function to return counter (for status infos)
unsigned long CTSensor::GetCounter() {
  return _counter;
}
//------------------------------------------
// Function to push initial Energy Counter from a remote system
void CTSensor::SetCounterFromRemote(unsigned long remoteCounter) {
  _counter += remoteCounter;
  _counterReady = true;
}
//------------------------------------------
// Function to know if Counter has been initialized (SetCounterFromRemote) and can be send to reporting system
bool CTSensor::GetReady() {
  return _counterReady;
}
//------------------------------------------
// Function to fully reinitialize this CTSensor
void CTSensor::Reset() {
  _firstReadDone = false;
  _currentI = 0.0;
  _avgI = 0.0;
  decimalLeft = 0;
  _counter = 0;
  _counterReady = false;
}