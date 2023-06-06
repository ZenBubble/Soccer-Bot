//remember to set custom wifi and password
//install esp boards: set additional boards manager url to https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//set board to FireBeetle-ESP32
#include <WiFi.h>
#include <AsyncTCP.h> //must install asynctcp library. Go to tools > manage libraries > search up "asynctcp" > hit install
#include <ESPAsyncWebServer.h> //must install ESPAsyncWebServer library. Go to https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip and it should download a file. Go to sketch > include library > add .zip library > select the downloaded file

const char* ssid = "McRoberts_Guest";
const char* password = "mcrob6600";

const char* apssid = "";  //set your own wifi name 
const char* appassword = "";  //set your own password (at least 8 characters)

int ENA_pin = 21;
int IN1 = 19;
int IN2 = 18;
int IN3 = 13;
int IN4 = 12;
int ENB_pin = 14;
bool usedirect = false; //true = direct connection to board (uses own wifi) false = connects from school wifi

String slider_value = "0";
const char* input_parameter = "value";

AsyncWebServer server(80);

//all of this controls how the webpage looks like and where it redirects when touching a button
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Car Control</title>
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 2.0rem;}
    body {
      max-width: 400px; margin:0px auto; padding-bottom: 25px;
      background-image: url('https://cdn.dribbble.com/users/58639/screenshots/3788063/936.jpg');
      background-color: #03cafc;
      background-size: cover;
      background-repeat: no-repeat;
      }
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #ffffff;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background:#01070a; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #01070a; cursor: pointer; } 
    .pushbutton {
        padding: 10px 20px;
        font-size: 24px;
        text-align: center;   
        outline: none;
        width: 145px;
        color: #000000;
        background-color: #ffffff;
        border: none;
        border-radius: 5px;
        box-shadow: 0 6px #999;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .pushbutton:hover {background-color: #1f2e45}
      .pushbutton:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
      
  </style>
</head>
<body>
  <h2>Soccer Bot</h2>
  <p><span id="textslider_value">%SLIDERVALUE%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="100" max="255" value="%SLIDERVALUE%" step="1" class="slider">Speed Control</p>
  <p><button class="pushbutton" onmousedown="toggleState('forward');" ontouchstart="toggleState('forward');" onmouseup="toggleState('stop');" ontouchend="toggleState('stop');">Accelerate</button></p>
  <p><button class="pushbutton" onmousedown="toggleState('left');" ontouchstart="toggleState('left');" onmouseup="toggleState('stop');" ontouchend="toggleState('stop');">Turn left</button>
  <button class="pushbutton" onmousedown="toggleState('right');" ontouchstart="toggleState('right');" onmouseup="toggleState('stop');" ontouchend="toggleState('stop');">Turn right</button></p>
  <button class="pushbutton" onmousedown="toggleState('backward');" ontouchstart="toggleState('backward');" onmouseup="toggleState('stop');" ontouchend="toggleState('stop');">Reverse</button>
<script>
function updateSliderPWM(element) {
  var slider_value = document.getElementById("pwmSlider").value;
  document.getElementById("textslider_value").innerHTML = slider_value;
  console.log(slider_value);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+slider_value, true);
  xhr.send();
}
function toggleState(state){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/" + state, true);
  xhr.send();
}

</script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  if (var == "SLIDERVALUE"){
    return slider_value;
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  delay(1000);
  pinMode(ENA_pin, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA_pin, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  analogWrite(ENA_pin, slider_value.toInt());
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENB_pin, slider_value.toInt());
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  if (usedirect == true){
  WiFi.softAP(apssid, appassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("connect to your wifi and enter this in your web browser: ");
  Serial.println(IP);
  }

  else if (usedirect == false){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to wifi...");
  }
  Serial.print("make sure you're connected to McRoberts_Guest and enter this in your web browser: ");
  Serial.println(WiFi.localIP());
  }

  //everything below controls happens when certain url links are accessed
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      slider_value = message;
      //motor doesn't function on anything below 100
      if (slider_value.toInt() > 100){
      analogWrite(ENA_pin, slider_value.toInt());
      analogWrite(ENB_pin, slider_value.toInt());
      }
      else{
        analogWrite(ENA_pin, 0);
        analogWrite(ENB_pin, 0);
      }
    }
    else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });
    
    server.on("/forward", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("Moving forward");
    request->send(200, "text/plain", "OK");
  });

    server.on("/backward", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("Reversing");
    request->send(200, "text/plain", "OK");
  });

    server.on("/left", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("Turning left");
    request->send(200, "text/plain", "OK");
  });

    server.on("/right", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("Turning right");
    request->send(200, "text/plain", "OK");
  });

    server.on("/stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    Serial.println("Stopped");
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  
}
