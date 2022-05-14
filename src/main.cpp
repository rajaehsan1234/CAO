#include<Arduino.h>
#include<WiFi.h>
#include "time.h"
#include <ESP_Mail_Client.h>

#define Wifi_net "AndroidAP"
#define Wifi_pass "rajaamjad"
#define Timeout 20000
// The smtp host name e.g. smtp.gmail.com for GMail 
#define SMTP_HOST "smtp.gmail.com"
// 465 or esp_mail_smtp_port_465
#define SMTP_PORT 465
/* The sign in credentials */
#define AUTHOR_EMAIL "rajaehsanriaz@gmail.com"
#define AUTHOR_PASSWORD "******"

//variable to ensure that email is send only once
bool onlyone=true;

// The SMTP Session object used for Email sending 
SMTPSession smtp;

// Callback function to get the Email sending status 
void smtpCallback(SMTP_Status status);
String time_str;
int ir=19;

//setting NTP server
const char* ntpServer = "pool.ntp.org";
//for pakistan
const long  gmtOffset_sec = 18000;   // GMT offset (seconds)
const int   daylightOffset_sec = 0; // daylight offset (seconds)


//function to send message
void sendMail()
{

   // 1 for basic level debugging
  smtp.debug(1);

  /* Set the callback function to get the sending results */
   smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;


  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "Ehsan";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Pills Reminder";
  message.addRecipient("Ehsan"  , "rajputjanjua1234@gmail.com");

  String textMsg = "Do it";
  message.text.content = textMsg.c_str();

  message.text.charSet = "us-ascii";

  //The content transfer encoding e.g.
  // enc_7bit or "7bit" (not encoded)
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /* The message priority
    esp_mail_smtp_priority_high or 1
    esp_mail_smtp_priority_normal or 3
    esp_mail_smtp_priority_low or 5
   */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  /*The Delivery Status Notifications e.g.
    esp_mail_smtp_notify_never
    esp_mail_smtp_notify_success
    esp_mail_smtp_notify_failure
    esp_mail_smtp_notify_delay
    The default value is esp_mail_smtp_notify_never
   */
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  message.addHeader(F("Message-ID: <test.send@gmail.com>"));

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}


//function to connect to Wifi
void connect_wifi(){
  Serial.print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(Wifi_net,Wifi_pass);
  //timeout to stop checking for wifi connection after 20 seconds
  unsigned long startattmpt=millis();
  //to make sure were still within our timeout we will subtract startattempttime from current uptime and makesure its less then timeout
  while(WiFi.status()!= WL_CONNECTED && millis()-startattmpt < Timeout)
  {
    Serial.print(".");
    delay(100);
  }

  if(WiFi.status()!= WL_CONNECTED)
  {
    Serial.print("Failed");
    //take any action
  }
  else
  {
    Serial.print("Connected");
    Serial.println(WiFi.localIP());
  } 

}

//function to return local date day time in pakistan
String printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "Time Error";
  }
  char output[80];
  strftime(output, 80, "%H:%M:%S", &timeinfo);
  time_str = String(output);
  return String(output);
}



void setup(){
  Serial.begin(115200);
  //setting pin 18 to used as output
  pinMode(18,OUTPUT);
  //setting 19 pin to get digital output fro IR sendor
  pinMode(ir,INPUT);
  connect_wifi();

   //init and get the time
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   printLocalTime();

}

//function to check if local time is equal to specific time which was set
bool checktime(){
  if(printLocalTime()=="12:48:57" || printLocalTime()=="12:48:58" ||printLocalTime()=="12:48:59" )
  return true;
  else return false;
}

//function to start alarm
void alarm()
{
  //sending mail when alarm is  set on
  sendMail();
  //blinking led showing alarm is on while there is LOW at IR  input pin
  while(digitalRead(ir)==LOW)
  {
    digitalWrite(18,HIGH);
    delay(1000);
    
    digitalWrite(18,LOW);
    delay(1000);

  }
  //when IR input is 1 then alarm is set off )(0)
    digitalWrite(18,LOW);
}
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP32 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");
 
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
void loop() {
  delay(1000);
      Serial.println(printLocalTime());
      //is its time for alarm then set call alarm function
  if(checktime())
  {
  alarm();
  }
}
