/*
    This is a relative simple example of the use of the Kasia framework

    It connects to WiFi and starts a WebServer at port 80
    Once you navigate to the main page for your device
    You will see a simple page with your DeviceId or DeviceName in the title
    And a single log on the web page's log section that will say that 'Temperature Sensors server started!'

    You will also see a data Chart that will show the temperature readings
*/

#include <Kasia.h>

int temperatureA = 0;
int temperatureB = 0;

void setup()
{
    //Bind or Link your data variables so that their data can be read
    //this will create a data Chart and will display these data points
    kasia.bindData("TemperatureA", &temperatureA);
    kasia.bindData("TemperatureB", &temperatureB);

    // Once you have connected once. can use this line to start your connection
    kasia.start("Temperature Sensors", 9600, "<WiFi-SSID>", "<WiFi-password>");

    // Once it has successfully connected. It will print something like this:
    // Temperature Sensors server started!
    // Connected got new IP: 192.168.138.96 (your IP is likely to be different)

    // use the IP that you see in the logs and navigate to it on your browser
    // eg. http://192.168.138.96
}

void loop()
{
    // reading sensor data
    temperatureA = random(-248, 248);
    temperatureB = random(-248, 248);
}
