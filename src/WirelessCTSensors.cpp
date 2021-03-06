#include "WirelessCTSensors.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
//Function Called by timer when Tick
void WebCTSensors::SendTimerTick()
{

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {

    //if CTSensor is not ready, then request for initial value in Home Automation
    if (clampRatios[i] != 0.0 && !_ctSensors[i].GetReady() && ha.clampIds[i] != 0)
    {

      String completeURI;

      switch (ha.enabled)
      {
      case 1:
        _requests[i] = F("&type=cmd&id=");
        _requests[i] += ha.clampIds[i];
        completeURI = completeURI + F("http") + (ha.tls ? F("s") : F("")) + F("://") + ha.hostname + F("/core/api/jeeApi.php?apikey=") + ha.jeedom.apiKey + _requests[i];
        break;
      }
      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!ha.tls)
        http.begin(completeURI);
      else
      {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, ha.fingerPrint));
      }

      _requestResults[i] = http.GET();
      if (_requestResults[i] == 200)
      {
        uint16_t units = 0;
        uint16_t thousands = 0;
        uint16_t millions = 0;
        uint16_t billions = 0;
        unsigned long payloadValue = 0;

        switch (ha.enabled)
        {
        case 1:
          String payload = http.getString();

          yield();
          units = payload.substring(payload.length() >= 3 ? payload.length() - 3 : 0).toInt();
          if (payload.length() > 3)
            thousands = payload.substring(payload.length() >= 6 ? payload.length() - 6 : 0, payload.length() - 3).toInt();
          if (payload.length() > 6)
            millions = payload.substring(payload.length() >= 9 ? payload.length() - 9 : 0, payload.length() - 6).toInt();
          if (payload.length() > 9)
            billions = payload.substring(payload.length() >= 12 ? payload.length() - 12 : 0, payload.length() - 9).toInt();
          break;
        }

        http.end();

        payloadValue = billions;
        payloadValue = (payloadValue * 1000) + millions;
        payloadValue = (payloadValue * 1000) + thousands;
        payloadValue = (payloadValue * 1000) + units;

        _ctSensors[i].SetCounterFromRemote(payloadValue);
      }
    }

    //this time ctSensor is ready
    if (clampRatios[i] != 0.0 && _ctSensors[i].GetReady() && ha.clampIds[i] != 0)
    {

      String completeURI;

      switch (ha.enabled)
      {
      case 1:
        _requests[i] = String(F("&type=")) + ha.jeedom.commandType + F("&id=") + ha.clampIds[i] + F("&value=") + _ctSensors[i].GetCounterUpdated();
        completeURI = completeURI + F("http") + (ha.tls ? F("s") : F("")) + F("://") + ha.hostname + F("/core/api/jeeApi.php?apikey=") + ha.jeedom.apiKey + _requests[i];
        break;
      }

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!ha.tls)
        http.begin(completeURI);
      else
      {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, ha.fingerPrint));
      }

      _requestResults[i] = http.GET();
      http.end();
    }
  }
}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebCTSensors::SetConfigDefaultValues()
{
  clampRatios[0] = 30.0;
  clampRatios[1] = 30.0;
  clampRatios[2] = 30.0;

  noiseCancellation[0] = 0.0;
  noiseCancellation[1] = 0.0;
  noiseCancellation[2] = 0.0;

  ha.enabled = 0;
  ha.tls = false;
  memset(ha.fingerPrint, 0, 20);
  ha.hostname[0] = 0;
  ha.clampIds[0] = 0;
  ha.clampIds[1] = 0;
  ha.clampIds[2] = 0;
  ha.jeedom.apiKey[0] = 0;
  ha.jeedom.commandType[0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebCTSensors::ParseConfigJSON(JsonObject &root)
{
  //Retrocompatibility block to be removed after v3.1.5 --
  if (root["je"].success())
    ha.enabled = root["je"] ? 1 : 0;
  if (root["jt"].success())
    ha.tls = root["jt"];
  if (root["jh"].success())
    strlcpy(ha.hostname, root["jh"], sizeof(ha.hostname));
  if (root["ct"].success())
    strlcpy(ha.jeedom.commandType, root["ct"], sizeof(ha.jeedom.commandType));
  if (root["c1"].success())
    ha.clampIds[0] = root["c1"];
  if (root["c2"].success())
    ha.clampIds[1] = root["c2"];
  if (root["c3"].success())
    ha.clampIds[2] = root["c3"];
  if (root["jfp"].success())
    Utils::FingerPrintS2A(ha.fingerPrint, root["jfp"]);
  // --

  if (root["cr1"].success())
    clampRatios[0] = root["cr1"];
  if (root["cnc1"].success())
    noiseCancellation[0] = root["cnc1"];
  if (root["cr2"].success())
    clampRatios[1] = root["cr2"];
  if (root["cnc2"].success())
    noiseCancellation[1] = root["cnc2"];
  if (root["cr3"].success())
    clampRatios[2] = root["cr3"];
  if (root["cnc3"].success())
    noiseCancellation[2] = root["cnc3"];

  if (root[F("hae")].success())
    ha.enabled = root[F("hae")];
  if (root[F("hatls")].success())
    ha.tls = root[F("hatls")];
  if (root[F("hah")].success())
    strlcpy(ha.hostname, root["hah"], sizeof(ha.hostname));
  if (root[F("hacid1")].success())
    ha.clampIds[0] = root[F("hacid1")];
  if (root[F("hacid2")].success())
    ha.clampIds[1] = root[F("hacid2")];
  if (root[F("hacid3")].success())
    ha.clampIds[2] = root[F("hacid3")];
  if (root["hafp"].success())
    Utils::FingerPrintS2A(ha.fingerPrint, root["hafp"]);

  if (root["ja"].success())
    strlcpy(ha.jeedom.apiKey, root["ja"], sizeof(ha.jeedom.apiKey));
  if (root["jct"].success())
    strlcpy(ha.jeedom.commandType, root["jct"], sizeof(ha.jeedom.commandType));
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebCTSensors::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam(F("cr1"), true))
    clampRatios[0] = request->getParam(F("cr1"), true)->value().toFloat();
  if (request->hasParam(F("cnc1"), true))
    noiseCancellation[0] = request->getParam(F("cnc1"), true)->value().toFloat();
  if (request->hasParam(F("cr2"), true))
    clampRatios[1] = request->getParam(F("cr2"), true)->value().toFloat();
  if (request->hasParam(F("cnc2"), true))
    noiseCancellation[1] = request->getParam(F("cnc2"), true)->value().toFloat();
  if (request->hasParam(F("cr3"), true))
    clampRatios[2] = request->getParam(F("cr3"), true)->value().toFloat();
  if (request->hasParam(F("cnc3"), true))
    noiseCancellation[2] = request->getParam(F("cnc3"), true)->value().toFloat();

  if (request->hasParam(F("hae"), true))
    ha.enabled = request->getParam(F("hae"), true)->value().toInt();

  //if an home Automation system is enabled then get common param
  if (ha.enabled)
  {
    if (request->hasParam(F("hatls"), true))
      ha.tls = (request->getParam(F("hatls"), true)->value() == F("on"));
    else
      ha.tls = false;
    if (request->hasParam(F("hah"), true) && request->getParam(F("hah"), true)->value().length() < sizeof(ha.hostname))
      strcpy(ha.hostname, request->getParam(F("hah"), true)->value().c_str());
    if (request->hasParam(F("hacid1"), true))
      ha.clampIds[0] = request->getParam(F("hacid1"), true)->value().toInt();
    if (request->hasParam(F("hacid2"), true))
      ha.clampIds[1] = request->getParam(F("hacid2"), true)->value().toInt();
    if (request->hasParam(F("hacid3"), true))
      ha.clampIds[2] = request->getParam(F("hacid3"), true)->value().toInt();
    if (request->hasParam(F("hafp"), true))
      Utils::FingerPrintS2A(ha.fingerPrint, request->getParam(F("hafp"), true)->value().c_str());
  }

  //Now get specific param
  switch (ha.enabled)
  {
  case 1: //Jeedom
    char tempApiKey[48 + 1];
    //put apiKey into temporary one for predefpassword
    if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempApiKey))
      strcpy(tempApiKey, request->getParam(F("ja"), true)->value().c_str());
    if (request->hasParam(F("jct"), true))
      strncpy(ha.jeedom.commandType, request->getParam(F("jct"), true)->value().c_str(), sizeof(ha.jeedom.commandType) - 1);
    //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempApiKey, appDataPredefPassword))
      strcpy(ha.jeedom.apiKey, tempApiKey);
    if (!ha.hostname[0] || !ha.jeedom.apiKey[0])
      ha.enabled = 0;
    break;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebCTSensors::GenerateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

  char fpStr[60];
  gc = gc + F("\"cr1\":") + clampRatios[0];
  gc = gc + F(",\"cr2\":") + clampRatios[1];
  gc = gc + F(",\"cr3\":") + clampRatios[2];
  gc = gc + F(",\"cnc1\":") + noiseCancellation[0];
  gc = gc + F(",\"cnc2\":") + noiseCancellation[1];
  gc = gc + F(",\"cnc3\":") + noiseCancellation[2];

  gc = gc + F(",\"hae\":") + ha.enabled;
  gc = gc + F(",\"hatls\":") + ha.tls;
  gc = gc + F(",\"hah\":\"") + ha.hostname + '"';
  gc = gc + F(",\"hacid1\":") + ha.clampIds[0];
  gc = gc + F(",\"hacid2\":") + ha.clampIds[1];
  gc = gc + F(",\"hacid3\":") + ha.clampIds[2];
  gc = gc + F(",\"hafp\":\"") + Utils::FingerPrintA2S(fpStr, ha.fingerPrint, forSaveFile ? 0 : ':') + '"';

  if (forSaveFile)
  {
    if (ha.enabled == 1)
      gc = gc + F(",\"ja\":\"") + ha.jeedom.apiKey + '"';
  }
  else
    gc = gc + F(",\"ja\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)
  gc = gc + F(",\"jct\":\"") + ha.jeedom.commandType + '"';

  gc = gc + '}';

  return gc;
};
//------------------------------------------
//Generate JSON of application status
String WebCTSensors::GenerateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"ci1\":") + _ctSensors[0].GetCurrentI();
  gs = gs + F(",\"ci2\":") + _ctSensors[1].GetCurrentI();
  gs = gs + F(",\"ci3\":") + _ctSensors[2].GetCurrentI();
  gs = gs + F(",\"cai1\":") + _ctSensors[0].GetAverageI();
  gs = gs + F(",\"cai2\":") + _ctSensors[1].GetAverageI();
  gs = gs + F(",\"cai3\":") + _ctSensors[2].GetAverageI();
  gs = gs + F(",\"c1\":") + _ctSensors[0].GetCounter();
  gs = gs + F(",\"c2\":") + _ctSensors[1].GetCounter();
  gs = gs + F(",\"c3\":") + _ctSensors[2].GetCounter();
  if (ha.enabled)
    for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
    {
      gs = gs + F(",\"lr") + (i + 1) + F("\":\"") + _requests[i] + '"';
      gs = gs + F(",\"lrr") + (i + 1) + F("\":") + _requestResults[i];
    }

  gs += '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebCTSensors::AppInit(bool reInit)
{
  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {
    if (reInit)
      _ctSensors[i].Reset();

    if (!reInit)
      _requests[i].reserve(20);

    _requests[i] = "";
    _requestResults[i] = 0;
  }

  if (reInit && _sendTimer.getNumTimers())
    _sendTimer.deleteTimer(0);

  if (ha.enabled)
    _sendTimer.setInterval(SEND_PERIOD, [this]() {
      this->SendTimerTick();
    });

  //"flush" serial buffer input
  while (Serial.available())
    Serial.read();

  return true;
};
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t* WebCTSensors::GetHTMLContent(WebPageForPlaceHolder wp){
      switch(wp){
    case status:
      return (const uint8_t*) status1htmlgz;
      break;
    case config:
      return (const uint8_t*) config1htmlgz;
      break;
    default:
      return nullptr;
      break;
  };
  return nullptr;
};
//and his Size
size_t WebCTSensors::GetHTMLContentSize(WebPageForPlaceHolder wp){
  switch(wp){
    case status:
      return sizeof(status1htmlgz);
      break;
    case config:
      return sizeof(config1htmlgz);
      break;
    default:
      return 0;
      break;
  };
  return 0;
};
//------------------------------------------
//code to register web request answer to the web server
void WebCTSensors::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication){
    //Nothing to do
};

//------------------------------------------
//Run for timer
void WebCTSensors::AppRun()
{

  //if we receive data from ATTiny
  if (Serial.available())
  {
    char c = Serial.read();

    //don't care about \n
    if (c == '\n')
      return;
    //if \r, we need to process received datas
    else if (c == '\r')
    {

      byte i = serialBuffer[0] - '1';
      if (i > NUMBER_OF_CTSENSOR - 1 || serialBuffer[1] != ':')
      {
        serialBuffer[0] = 0;
        return;
      }

      //convert text to float and pass it to newI
      float newI = atof(serialBuffer + 2) * clampRatios[i] / 1000.0;
      newI -= noiseCancellation[i];
      if (newI <= 0.0)
        newI = 0.0;
      _ctSensors[i].NewIFromCTSensor(newI);
      serialBuffer[0] = 0;
    }
    //else we need to put this char in serialBuffer
    else
    {
      serialBuffer[strlen(serialBuffer) + 1] = 0;
      serialBuffer[strlen(serialBuffer)] = c;
    }
  }

  //Run sendTimer
  if (ha.enabled)
    _sendTimer.run();
}

//------------------------------------------
//Constructor
WebCTSensors::WebCTSensors(char appId, String appName) : Application(appId, appName) {}
