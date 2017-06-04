#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int RELAY_PIN = 12;
const int LED_PIN = 13;

// WiFi SSID and Password
const char* ssid = "<<REPLACE WITH YOUR WIFI SSID>>";
const char* password = "<<REPLACE YOUR WIFI PASSWORD>>";

// MQTT Server, for this demo purpose I am using Eclipse MQTT Sandbox, you
// can change it to any MQTT server. Also for this demo 
const char* mqtt_server = "iot.eclipse.org";

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Write the message to the Serial for debugging purpose
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // If the payload is 1 then turn on the Relay and LED, otherwise turn it off
  if ((char)payload[0] == '1') {
    Serial.println("Turning on kitchen light");
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("Turning off kitchen light");
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

// Connect to WiFi network using the SSID and password
void wifiConnect() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect MQTT
void mqttConnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
   
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("mqtteclipse/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Setup LED and Relay PIN
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Begin Serial
  Serial.begin(115200);

  delay(100);

  // Connect WiFi
  wifiConnect();

  // Setup PubSubClient
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
}

void loop() {
  // If PubSubClient is not connected then connect it
  if (!client.connected()) {
    mqttConnect();
  }

  // Loop
  client.loop();
}