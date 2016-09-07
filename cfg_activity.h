/** general setup
 *  TARGET board is ARDUINO M0 PRO
*/
#ifndef ACTIVITY_H
#define ACTIVITY_H

// DEBUG ==============================================================
#define DEBUG_PRINT(arg) Serial.print(arg)
#define DEBUG_PRINTLN(arg) Serial.println(arg)
//#define DEBUG_PRINT(arg)
//#define DEBUG_PRINTLN(arg)


// RTOS ===================================================================
#define TICK_10MN 35400
#define TICK_50MS 7
#define TICK_350MS 21
#define TICK_1S 60
#define TICK_4S 240
#define TICK_10S 600
#define TICK_17MS 1




// IOs ================================================================
#define LEGACY_LED (13)

// all extended IOs as outputs
// 53 is output for ethernet shield
// legacy PWM as outputs
// extended COM as outputs
#define INIT_IOS() {\
pinMode(LEGACY_LED, OUTPUT);\
for(int indx= 1; indx<= 10; indx++) {pinMode(indx, OUTPUT);};\
}

#define TOGGLE_LEGACY_LED() {if (rbi_LED_L_toggle) {rbi_LED_L_toggle = false; digitalWrite(LEGACY_LED,LOW);}\
 else {rbi_LED_L_toggle = true; digitalWrite(LEGACY_LED,HIGH);}}
#define LEGACY_LED_ON() digitalWrite(LEGACY_LED,HIGH)
#define LEGACY_LED_OFF() digitalWrite(LEGACY_LED,LOW)

// IR sensor on analog port :
#define IR_SENSOR (3)
#define IR_THRESHOLD 300



// HTTP - HTML ========================================================
// buffer size to receive the whole HTTP request
#define HTTP_SIZE 500

// switch answer either to console or ethernet
//#define HTTP_TARGET Serial
#define HTTP_TARGET client


// Ethernet shield MAC address
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x53, 0xF0};



  
// HTTP header ==============================================
// We ask the remote device device to renew its requests every 20s
const char http_header[] = "HTTP/1.1 200 OK\n"
"Content-Type: text/html\n"
"Connection: close\n"
"Refresh: 20\n";    



 
 // WEB page ================================================
 const char html_page_content_begin[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
 "<\?xml version=\"1.0\" encoding=\"iso-8859-1\"\?>\n"
"<html>\n"
  "<head>\n"
    "<meta content=\"text/html; charset=iso-8859-1\" http-equiv=\"content-type\">\n"
    "<meta name=\"viewport\" content=\"width=device-width , maximum-scale=1.0\">\n"
    "<title>ARDUINO comptage</title>\n"
    "<style type=\"text/css\">\n" 
        "header { width:100\%; height:8\%;  padding:0em; color:DimGrey; background-color:orange; clear:both; text-align:center;}\n" 
      	"section {width:100\%; height:100\%; padding:1em; color:orange; background-color:DimGrey; clear:both; text-align:center;}\n"
        "footer {position:absolute; bottom:0;  width:100\%; height:20\%; padding:0em; color:DimGrey; background-color:orange; clear:both; text-align:center;}\n"
      	"button {background-color:DimGrey; color:orange; padding:10px 25px; text-align:center; display:inline-block; font-size:24px;}\n"
    "</style>\n"
  "</head>\n"
  "<body style=\"background-color: DimGrey;\">\n"
    "<header>\n"
      "<h2>MOTION COUNTER</h2>\n"
    "</header>\n"
    "<section>\n";
 


const char html_page_content_end[] = "</section>\n"
    "<footer>\n"
     "<form method=\"post\"> <button type=\"submit\">RESET counter</button> </form>\n"
    "</footer>\n"
  "</body>\n"
"</html>\n";


#endif
