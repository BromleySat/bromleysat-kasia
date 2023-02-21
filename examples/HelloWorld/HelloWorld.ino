#include <Kasia.h>

void setup()
{
    // Uncomment this line and set:
    // 1. Your deviceId or name
    // 2. Baud rate that is right for your device
    // 3. The SSID or Network name of your WiFi connection
    // 4. WiFi password
    // kasia.start("Hello, World!", 9600, "<WiFi-SSID>", "<WiFi-password>");

    // Once it has successfully connected. It will print something like this:
    //  Hello, World! server started!
    //  Connected got new IP: 192.168.138.96

    // Once you have connected once. can use this line to start your connection
    kasia.start("Hello, World!");

    //WiFi credentials will be persisted on the device
    //Baud rate will not really be needed because you'll see your logs on the web page
    //use the IP that you got in the first step and just navigate to it on your browser
    //eg. http://192.168.138.96
}

void loop()
{
    // not used in this example
}
