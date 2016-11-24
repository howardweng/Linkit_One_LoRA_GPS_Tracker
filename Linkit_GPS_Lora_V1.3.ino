/* Linkit One GPS + Gemtek LoRA tracker 
 *  The code is for Linkit One board + Gemtek LoRA module. The LoRA service is available in Taipei,Taiwan. It will not work if your location is not covered by Gemtek LoRA service.  
 *  If your LoRA service is not available, and you can not fully use the code, it is also a good sample code for Linkit One board to use Serial Port1 for transmit data. 
 *  Because unlike Arduino board, Linkit One board can not use USB to Serial for Serial TX. 
    
  FRRUT.COM is glad to provide this open source code and you are free to distrubute, and it also took reference from many open source projects and sample codes, 
  such as LASS  https://github.com/LinkItONEDevGroup/LASS   project, and Mediatek sample codes, and many others...    
   
  On top of it, I invested a lot of time on this code for perfection.
    
  We welcome any opportunity about web and IoT applications business opportunities in terms of training, consulting, or business projects. 
  
  Please visit FRRUT.COM for more detail and 
  ----> https://www.frrut.com  果子創意
  
 author:   Howard Weng (FRRUT.COM)  howard.weng@gmail.com  
    @section  HISTORY
    v1.0  - First release
    v1.2  - released to Github

If hardware and MOQTT broker is correctly set. you can expect to receive the data as following.  

{
  "id" : "f3bf73a8-c9aa-4e05-b392-a42c49da0315",
  "macAddr" : "0400xxxx",                 ===> this is module ID
  "data" : "118a142dfc8a6400000000",      ===> this is GPS data we need.
  "buff" : "2016-11-08T02:29:35.122Z",
  "recv" : "2016-11-08T02:29:34.000Z",
  "extra" : {
    "gwip" : "192.168.0.89",
    "gwid" : "00001c497b48dc91",
    "repeater" : "00000000ffffffff",
    "systype" : 4,
    "rssi" : -77,
    "snr" : 95
  }
}  


***** As you can see, Gemtek LoRA module does not need any credential, because authorization management is on server (broker) side. 
 */


#define LED 2      //connect LED to digital pin2

#include <LDateTime.h>
datetimeInfo t;
char bufft[256];

byte lora_trans[22];  //lora buffer for GPS

#include <LGPS.h>
gpsSentenceInfoStruct g_info;

#include <LBattery.h>
char buffb[256];

unsigned long lastSend;
char GPS_formatted[130]; 

long MemoryMillis;

long longtimewaiting; 

// 3 Axis Accerator definition

#include <Wire.h>
#include <ADXL345.h>
ADXL345 accelerometer;




// following is the main body of program 


void getGPSData(gpsSentenceInfoStruct &g_info, char* GPS_formatted)
{
  processled();
  
  LGPS.powerOn();
  boolean GPS_fix = false;

  while (!GPS_fix)
  {
    LGPS.getData(&g_info);                                      //get the data from the GPS and store it in 'g_info'
    GPS_fix = printGPGGA((char*)g_info.GPGGA,GPS_formatted);    //printGPGGA returns TRUE if the GPGGA string returned confirms a GPS fix.
   }
  LGPS.powerOff();   
}

boolean printGPGGA(char* str, char* GPS_formatted)
{
  char SMScontent[160];
  char latitude[20];
  char lat_direction[1];
  char longitude[20];
  char lon_direction[1];
  char buf[20];
  char time[30];
  const char* p = str;
  p = nextToken(p, 0); // GGA
  p = nextToken(p, time); // Time
  p = nextToken(p, latitude); // Latitude
  p = nextToken(p, lat_direction); // N or S?
  p = nextToken(p, longitude); // Longitude
  p = nextToken(p, lon_direction); // E or W?
  p = nextToken(p, buf); // fix quality
  if (buf[0] == '1')
  {
    // GPS fix
    p = nextToken(p, buf); // number of satellites
   Serial.print("GPS is fixed:");
   Serial.print(buf); 
   Serial.println();  
   Serial.print(atoi(buf));                  // Printing Satellite number!! 
   Serial.println("found!");
  
    strcpy(SMScontent, "GPS fixed, satellites found: ");
   strcat(SMScontent, buf);
    
    const int coord_size = 8;
    char lat_fixed[coord_size],lon_fixed[coord_size];
    convertCoords(latitude,longitude,lat_fixed, lon_fixed,coord_size);
    

    Serial.println(lat_fixed);                  // Latitude data will be used for LoRA. 
    Serial.println(lon_fixed);                  // Longitude data will be used for LoRA.

float GPS_LAT_f = (float)atof(lat_fixed);
float GPS_LON_f = (float)atof(lon_fixed);
   
  GPS_LAT_f += 90;                            //Plus 90 on Latitude, in case it is negative. Need to deduct from NODE-Red to return to correct data.   
  GPS_LON_f += 180;                           //Plus 180, on Longitude in case it is negative. Need to deduct from NODE-Red to return to correct data.
  
  unsigned long GPS_LAT_i = GPS_LAT_f*10000;   //Times 10000 to get integer, need to devide 10000 at NODE-RED to return to correct data.  
  unsigned long GPS_LON_i = GPS_LON_f*10000;   //Times 10000 to get integer, need to devide 10000 at NODE-RED to return to correct data. 

//Seperate GPS_LAT_i into 3 * 8 bits(HEX) + battery level

lora_trans[12] = (GPS_LAT_i >> 16); 
lora_trans[13] = (GPS_LAT_i >> 8); 
lora_trans[14] = (GPS_LAT_i);
lora_trans[15] = (GPS_LON_i >> 16); 
lora_trans[16] = (GPS_LON_i >> 8);
lora_trans[17] = (GPS_LON_i);
lora_trans[18] = (LBattery.level());


 
Serial.println("++++++++++   Push to cloud  ++++++++++++++");   //Debug purpose. 
pushDataCloud();   //    Debug purpose.


return true;    
}

if ((millis()-MemoryMillis)>80000)      // In case battery runs out under no GPS signal enviornment. Timer set 80 seconds of maximum time to search GPS signal. And it will stop for next trigger to search GPS again. 
    {

      nosignalled();
            
      Serial.println(" Only Connecting 60 seconds and wait for next time ");   //    Debug purpose.
      Serial.println(MemoryMillis);  //    Debug purpose.
      Serial.println(millis());       //    Debug purpose.
      MemoryMillis=millis();           

      
       Serial.println("++++++++++   Push to cloud  ++++++++++++++");      //    Debug purpose.
       
       pushDataCloud();   // this command will push data to MQTT broker, please see PushDataCloud command for detail. 


  return true;
 }

       
  else
  {

    Serial.println("GPS is not fixed yet. waiting.......");       //    Debug purpose.

//  When GPS is searching, LED flashs as following. 

  digitalWrite(LED, HIGH);   // set the LED on
  delay(200);
  digitalWrite(LED, LOW);   // set the LED off
  delay(100);
  digitalWrite(LED, HIGH);   // set the LED on
  delay(1000);
  digitalWrite(LED, LOW);   // set the LED off
  delay(100);
    
    return false;    
  }     
   delay(100); //orignal 1000
}



void pushDataCloud(){

char buff[150];  
 sprintf(buff, "AT+DTX=22,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", lora_trans[12], lora_trans[13], \
 lora_trans[14], lora_trans[15], lora_trans[16], lora_trans[17], lora_trans[18], lora_trans[2], lora_trans[2],  \
 lora_trans[2], lora_trans[2]); 
  
  Serial.println(buff);  //For debug
  Serial.println("******************   PUSH Data now   ***************");
    
  Serial1.println(buff);  //  Sending Data to LoRA MQTT broker

  Serial.println("********  Light LED one more time *********");
  
  ledlight();  //LED flashes as defined when transfer successfully. See behavior of LED flashs detail in ledlight()

}


//Whole set of GPS conversion codes... 

void convertCoords(const char* latitude, const char* longitude, char* lat_return, char* lon_return, int buff_length)
{
  char lat_deg[3];
  strncpy(lat_deg,latitude,2);      //extract the first 2 chars to get the latitudinal degrees
  lat_deg[2] = 0;                   //null terminate
  char lon_deg[4];
  strncpy(lon_deg,longitude,3);      //extract first 3 chars to get the longitudinal degrees
  lon_deg[3] = 0;                    //null terminate
  int lat_deg_int = arrayToInt(lat_deg);    //convert to integer from char array
  int lon_deg_int = arrayToInt(lon_deg);
  float latitude_float = arrayToFloat(latitude);      //convert the entire degrees-mins-secs coordinates into a float - this is for easier manipulation later
  float longitude_float = arrayToFloat(longitude);
  latitude_float = latitude_float - (lat_deg_int*100);      //remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
  longitude_float = longitude_float - (lon_deg_int*100);
   latitude_float /=60;                                    //convert minutes-seconds to decimal
  longitude_float/=60;
  latitude_float += lat_deg_int;                          //add back on the degrees part, so it is decimal degrees
  longitude_float+= lon_deg_int;
   snprintf(lat_return,buff_length,"%2.3f",latitude_float);    //format the coordinates nicey - no more than 3 decimal places
  snprintf(lon_return,buff_length,"%3.3f",longitude_float);
}


int arrayToInt(const char* char_array)   //data NOT used in this case, reserved for expension. 
{
  int temp;
  sscanf(char_array,"%d",&temp);
  return temp;
}


float arrayToFloat(const char* char_array)  //data NOT used in this case, reserved for expension.
{
  float temp;
  sscanf(char_array, "%f", &temp);
  return temp;
}



const char *nextToken(const char* src, char* buf)
{
  int i = 0;
  while (src[i] != 0 && src[i] != ',')
  i++;
  if (buf)
  {
    strncpy(buf, src, i);
    buf[i] = 0;
  }
  if (src[i])
  i++;
  return src + i;
}






//Few LED flashing settings

void ledlight(){    
  Serial.println("SENDING DATA LED");   
  digitalWrite(LED, HIGH);   // set the LED on
    delay(200);
  digitalWrite(LED, LOW);   // set the LED off
    delay(200);
  digitalWrite(LED, HIGH);   // set the LED on
    delay(200);
  digitalWrite(LED, LOW);   // set the LED off
    delay(200);
  digitalWrite(LED, HIGH);   // set the LED on
  delay(3000);               // for 500ms
  digitalWrite(LED, LOW);   // set the LED off
}


void triggerled(){   

   Serial.println("Trigger LED"); 
  digitalWrite(LED, HIGH);   // set the LED on
    delay(100);
  digitalWrite(LED, LOW);   // set the LED off
    delay(100);
  digitalWrite(LED, HIGH);   // set the LED on
    delay(100);
  digitalWrite(LED, LOW);   // set the LED off 
      delay(100);
    digitalWrite(LED, HIGH);   // set the LED on
    delay(100);
  digitalWrite(LED, LOW);   // set the LED off
    delay(100);
  digitalWrite(LED, HIGH);   // set the LED on
    delay(100);
  digitalWrite(LED, LOW);   // set the LED off   
}


void nosignalled(){    
  Serial.println("No Signal LED");  
  digitalWrite(LED, HIGH);   // set the LED on
    delay(2000);
  digitalWrite(LED, LOW);   // set the LED off
    delay(1000);
  digitalWrite(LED, HIGH);   // set the LED on
    delay(2000);
  digitalWrite(LED, LOW);   // set the LED off 
   delay(1000);  digitalWrite(LED, HIGH);   // set the LED on
  digitalWrite(LED, LOW);   // set the LED off
}


void processled(){      

  Serial.println("Processled");
  digitalWrite(LED, HIGH);   // set the LED on
    delay(50);
  digitalWrite(LED, LOW);   // set the LED off
    delay(50);
  digitalWrite(LED, HIGH);   // set the LED on
    delay(50);
  digitalWrite(LED, LOW);   // set the LED off   
}









void setup()
{

   
  //Serial.begin( 115200 );
   Serial1.begin(9600); //this open the serial port 1 for Lora module. 

   lastSend = 0;
   MemoryMillis = 0;



// Light LED Settings initial off

  pinMode(LED, OUTPUT);  
  digitalWrite(LED, LOW);   // set the LED off  



// 3 Axis Settings

 Serial.println("Initialize ADXL345");
  if (!accelerometer.begin())
  {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    delay(500);
  }

  accelerometer.setActivityThreshold(5.0);    // Recommended 2 g
  accelerometer.setInactivityThreshold(5.0);  // Recommended 2 g

  accelerometer.setTimeInactivity(180);         // Recommended 5 s
  accelerometer.setActivityXYZ(1);         // Check activity on X,Y,Z-Axis  // Set activity detection only on X,Y,Z-Axis
  accelerometer.setInactivityXYZ(1);       // Check inactivity on X,Y,Z-Axis    // Set inactivity detection only on X,Y,Z-Axis
  accelerometer.useInterrupt(ADXL345_INT1);    // Select INT 1 for get activities

    
}



void loop()
{

// Read values for activities
delay(50);  
Vector norm = accelerometer.readNormalize();
// Read activities
Activites activ = accelerometer.readActivites();



// There are two ways to trigger the GPS. One is triggered by G Sensor, when the device is moving, this sensor will be activated, and I only want one location in 60seconds. 
// When the device is not in motion, say, at night. It is not necessary to send data all the time, because the battery power is precious.So I would let only the device only report once per 1 hour(3600000 seconds). 

  if (activ.isActivity)
  {
    if( millis()-lastSend > 60000 ) { //  First Trigger is to trigger GPS report by G-Sensor motion. for power saving purpose (also MQTT broker limitation), I would allow only one update within 60 second to prevent frequent unintended movements. 
    lastSend = millis();     

    Serial.println(" ");
    Serial.println(" ");
    Serial.println("++++++    ACTION   +++++++++");
    
    triggerled();
    
   //   PrintTime();

     getGPSData(g_info,GPS_formatted);   //Get GPS location data and send data to MQTT broker.
         
    }
  }


if ((millis()-longtimewaiting)>3600000)   // Second trigger timing is to report GPS data regularly in 60 mins (1 hour). 
    {
      Serial.println(longtimewaiting);
      Serial.println(millis());
      longtimewaiting=millis();
     
    Serial.println(" ");
   Serial.println(" ");
   Serial.println("********  NO ACTION REPORTING LOCATION *********");
   Serial.println("********  Automatically report every 1 hour  *********");

       triggerled();
          
       getGPSData(g_info,GPS_formatted);      //Get GPS location data and send data to MQTT broker.
       
   }  
}


