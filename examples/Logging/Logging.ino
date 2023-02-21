/*
    This is a simple example of the use of the Kasia framework

    It connects to WiFi and starts a WebServer at port 80
    Once you navigate to the main page for your device
    You will see a simple page with your DeviceId or DeviceName in the title
    And a single log on the web page's log section that will say that 'My Device server started!'

    Then you will be getting a new log added to the log section every 5 seconds
*/

#include <Kasia.h>

void setup()
{
    kasia.start("My Device", 9600);

    //or if you prefer web-only logging 
    //kasia.start("My Device");
}

void loop()
{
    //generate random number to simulate a sensor reading
    int num = random(-248, 248);;

    //log with text and values
    logInfo("Random number: ", num, " logged");
    delay(5000); 
}
