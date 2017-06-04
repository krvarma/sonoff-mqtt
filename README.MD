Home Automation using Sonoff WiFi Switch and Google Assistant
===

Recently I got one Sonoff WiFi Switch from [Itead.cc](https://www.itead.cc/). Sonofff WiFi Switch is an ESP8266 based wireless switch and has a 5V Relay with a voltage range of 90-250V. Itead provides a mobile application eWeLinK to operate the switch remotely. The switch also works with Amazon Alexa and Nest which is awesome. You can find more details about how to use the switch [here](https://www.itead.cc/blog/user-guide-for-sonoff-slampher) and about Alexa integration [here](https://www.itead.cc/blog/ewelink-works-with-alexa-tutorial).

The switch comes with default ESP8266 firmware which provides the functionality to operate this remotely using the eWeLinK mobile application. But more advanced users can program theESP8266 chip on it. There is an unsoldered pins which brings out the Rx, Tx, GND and Vcc of the ESP8266. You can use this pins to re-program the ESP8266 which is another awesome thing about this.  Following picture shows the pins.

**Sonoff Pins**

![enter image description here](https://raw.githubusercontent.com/krvarma/sonoff-mqtt/master/images/pins.png)

To program the ESP8266 on it, you solder headers to these pins and connect to 3.3v FTDI board. *Please note that it should be 3.3v compatible, don't use 5v FTDI board since it will damage the board*.

**Wiring**

|Sonoff |FTDI |
|---|---|
|Vcc (Square Pad)|Vcc|
|Rx|Tx|
|Tx|Rx|
|GND|GND|

Once the wiring is completed connect the FTDI board to the computer. This will power the ESP8266. But this will not put the board in programming mode, it will just power the ESP8266. In order to put the board in programming mode, press and hold the button on the board ad connect the FTDI to the computer. This will put the board in programming mode. Following picture shows the button on the board.

![enter image description here](https://raw.githubusercontent.com/krvarma/sonoff-mqtt/master/images/programming%20button.png)

Once the board is in programming mode you can use the Arduino IDE to program it. Install the ESP8266 platform packages using Package Manager (You can find more information [here](https://github.com/esp8266/Arduino)) and select the board *Generic ESP8266*.

**Arduino Board Selection**

![enter image description here](https://raw.githubusercontent.com/krvarma/sonoff-mqtt/master/images/arduino.png)

The GPIO pin 12 is connected to the relay and GPIO pin 13 is connected to the LED. So in order to turn on the relay you can do digitalWrite on pin 12.

**Demo Application**

This demo connects to a MQTT server and listens for MQTT messages. The application listen the topic "***mqtteclipse/cmd***" for commands. For purpose of this demo the commands are simple 1 or 0. When the the payload 1 is received the relay is turned on and for 0 the relay is turned off. For this demo I am using publicly available Eclipse MQTT Sandbox server (***iot.eclipse.org***). If you have any other servers, replace the server ([line 13 of the firmware](https://github.com/krvarma/sonoff-mqtt/blob/master/firmware/sonoff-mqtt.ino#L13)). 

    const char* mqtt_server = "iot.eclipse.org";

You can use any MQTT client to send message to the board. 

**Google Assistant integration**

The demo application works with Google Assistant. For this purpose I am using [Actions on Google](https://developers.google.com/actions/). I created a simple action using [Google Action SDK](https://developers.google.com/actions/). I am using the following action.

    {
	    "actions": [
	      {
	        "description": "Default Welcome Intent",
	        "name": "MAIN",
	        "fulfillment": {
	          "conversationName": "sonoff_app"
	        },
	        "intent": {
	          "name": "actions.intent.MAIN",
	          "trigger": {
	            "queryPatterns": [
	              "talk to sonos"
	            ]
	          }
	        }
	      },
	      {
	        "description": "Turn on/off intent",
	        "name": "TURNONOFF",
	        "fulfillment": {
	          "conversationName": "sonoff_app"
	        },
	        "intent": {
	            "name": "com.sonoff.example.home",
	            "parameters": [
	                {
	                    "name": "cmd",
	                    "type": "SchemaOrg_Text"
	                }
	            ],
	            "trigger": {
	                "queryPatterns": [
	                    "Turn $SchemaOrg_Text:cmd kitchen light"
	                ]
	            }
	        }
	      }
	    ],
	    "conversations": {
	      "sonoff_app": {
	        "name": "sonoff_app",
	        "url": "https://hook.io/krvarma/eclipsemqtt"
	      }
	    }
    }

As you can see from the above action, it is pretty simple. The actions has only one query pattern `"Turn $SchemaOrg_Text:cmd kitchen light"`.  User can say `Turn on kitchen light` or `Turn off kitchen light`. You may notice the `$SchemaOrg_Text:cmd` on the query pattern. This a query parameter defined in action JSON file. When the user say `Turn on kitchen light` the query parameter *cmd* will contain the text "**on**", and will contain the word "**off**" when the user says `Turn off kitchen light`.  [This link](https://developers.google.com/actions/components/actions) describes defining actions.

Another point of interest is the fulfillment URL used to fulfill the action. This URL will be called to process the Action. I have hosted a node.js program on on [hook.io](http://hook.io/) microservice hosting platform. The URL `https://hook.io/krvarma/eclipsemqtt` points to a Node.js script I am using. Following is the script I am using:

    module['exports'] = function myService (hook) {  
	  	console.log(hook.params.data);
  
	  	var intent = hook.params.inputs[0].intent;
  
	  	if(intent === "assistant.intent.action.MAIN"){
	      	var response = {
				"expect_user_response": false,
	  			"final_response": {
		    		"speech_response": {
	    	  			"text_to_speech": "Hello!"
					}
				}
			};
      	
      
	      	console.log(response);
	      
		      	hook.res.writeHead(200, { 'Content-Type': 'application/json'});
		    hook.res.write(JSON.stringify(response));
	  	    hook.res.end();      
	    }
	  	else{
			var mqtt = require('mqtt')
				var mqttClient  = mqtt.connect('mqtt://iot.eclipse.org', 1883)
	
	      	mqttClient.on('connect', function () {
	          	var arguments = hook.params.inputs[0].arguments;
	          	var cmdText = '';
	          
	          	for(var arg of arguments){
	              	console.log(arg);
	              	console.log(arg.name + "=" + arg.raw_text);
	              
	              	if(arg.name === "cmd"){
	                	cmdText = arg.raw_text;
	              	}
	            }
	          	
	          	var cmd = (cmdText === "on" ? '1' : '0');
	          
				mqttClient.publish('mqtteclipse/cmd', cmd);
          
	          	var response = {
					"expect_user_response": false,
	  				"final_response": {
		    			"speech_response": {
	    	  				"text_to_speech": "Done!"
						}
					}
				};

				hook.res.writeHead(200, { 'Content-Type': 'application/json'});
		    	hook.res.write(JSON.stringify(response));
	  	    	hook.res.end();  
	      })
	   	}
	};

You can see that the script sends the payload 1 or 0 depending on the Action on Google request JSON data. This JSON data will contain the details of the user voice input. So if the the input argument "**cmd**" contains "**on**" then the script send payload 1 using the topic *mqtteclipse/cmd* otherwise it sends the payload 0. The firmware running on the device receives the message and turn on/off based on the payload.

You can follow this [Google Documentation](https://developers.google.com/actions/sdk/) to create a project in Actions on Google. Basically you have to do following steps to define the action.

1. Logon to Actions on Google Console
2. Create a Project and give it a name
3. Choose Use Action SDK options to define the action
4. Fill the App Information page. Here you can change the app name, invocation name, etc...
5. Deploy the [hook.io](http://hook.io/) microservice.
a. Sign in or Sign up on [hook.io](http://hook.io/)
b. Create a microservice using node.js as the language
c. Name it as *eclipseqmtt*.
d. Copy and paste the microservice and save the service
6. Save and test the action using any Google Assistant platform.

[This documentation](https://developers.google.com/actions/sdk/) describes all the necessary steps to create Actions on Google. I am using "Jarvis Home Manager" as the invocation name. But you can use any name as you want, just replace the invocation name on the Settings page of the action. Following image shows our Actions on Google project:

![enter image description here](https://raw.githubusercontent.com/krvarma/sonoff-mqtt/master/images/action-on-google.png)

**Demo Video**

[<img src="https://img.youtube.com/vi/EzTtGxZzP8E/0.jpg">](https://www.youtube.com/watch?v=EzTtGxZzP8E)