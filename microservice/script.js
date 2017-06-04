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