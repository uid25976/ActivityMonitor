/** Activity monitor 
 *  Every 1s, it checks for a 4s pulse from AN3 indicating a mouvement in front of the sensor
 *  It also checks if a WEB client is present and process its request
 *  Every 60s, it renews the Ethernet connection
 *  When a WEB client is present:
 *  It respond to either a GET page or GET form, by sending a WEB page that contains the value of the mouvement counter 
 *  If a form response is recevied, it resets the counter
 *  HISTORY:
 *  - Sept 4 2016 : first draft on Arduino M0
  */
  // include for ethernet shield
#include <SPI.h>
#include <Ethernet2.h>

#include "cfg_activity.h"


// RAM data =====================================================
// Initialize the Ethernet server port 80
EthernetServer server(80);
EthernetClient client;
boolean rbi_EthernetConnected;

// @TODO: replace by read outut and rewrtie : used by LEGACY LED: 
boolean rbi_LED_L_toggle = false;

// stores the received line from the remote browser
char string_ReceivedLine[HTTP_SIZE];
// write index into the string
int rint_currentWR = 0;

// counter for tasks
int rint_call_counter= 0;

// ACTIVITY counter ==== 
volatile int analog_sensor_value;
boolean rbi_lastState = false;
boolean rbi_newState = false;
int rint_counter= 0;
// =====================

 

// prototypes ===================================================
inline void cyclic_Task_SENSOR(void);
inline void cyclic_Task_WEBserver(void);
inline void cyclic_Task_RefreshConnection(void);

void SendHTMLpage();
int clientReadBuffer(EthernetClient* Ptrclient, int rint_NbBytesToRead, char* stringPtr_ReceivedLine);


// INIT task ====================================================
void setup()
{
    // INIT IOs ----------------
    INIT_IOS();
    
    // Debug monitor
    while (!Serial);
    Serial.begin(115200);
    Serial.println("RESET, now starting...");
    
    // request ethernet connection
    rbi_EthernetConnected = false;
  
    if (Ethernet.begin(mac) != 0)
    {
        rbi_EthernetConnected = true;
        server.begin();
      
        DEBUG_PRINT("My IP is now: ");
        DEBUG_PRINTLN(Ethernet.localIP());
    } // else will try 10mn later

}






// MAIN LOOP ========================================================
void loop()
{
    cyclic_Task_SENSOR();

    cyclic_Task_WEBserver();

   if (rint_call_counter++ > 60)
   {
        rint_call_counter = 0;
        cyclic_Task_RefreshConnection();
   }
   
    // wait 1s before to process any request 
    delay(1000);
}



// CYCLIC TASKS =======================================================
/** Input filter process: must be << 17ms
 *  
 */
inline void cyclic_Task_SENSOR(void)
{
    analog_sensor_value = analogRead(IR_SENSOR);
    // detection = high state for 4s (3.5V)
    // convert to digital data
     rbi_newState = (analog_sensor_value > IR_THRESHOLD)? true:false;

    if (rbi_newState && !rbi_lastState)
    {
      // rising edsge
      rint_counter++;
       Serial.print("rint_counter ");
       Serial.println(rint_counter);
    }
    
    rbi_lastState = rbi_newState;
}


/** WEB server process
 *  It can be long as it has a lower priority than the 17ms
 *  client checked every 350ms 
 */
inline void cyclic_Task_WEBserver(void)
{
  if(rbi_EthernetConnected)
  {
       // process WEB server
       // listen for incoming clients
       client = server.available();

              if (client)
              {
                    Serial.println(">>>we have a new client");
    
                    if (client.connected())
                    {
                          Serial.print(">>>client connected: bytes to read: ");
                          // check HTTP request
                          int isHTTPrequestComplete = 0;
                          int rint_NbBytesToRead = client.available();
                          Serial.println(rint_NbBytesToRead);
                          
                          isHTTPrequestComplete = false;  
                                         
                          // reset string RAM
                         string_ReceivedLine[0] = 0;
                         rint_currentWR = 0; 
                                 
                          // while bytes to read and http request end not found
                          int rint_NbWhile = 0;
                          while ((rint_NbBytesToRead > 0) && (isHTTPrequestComplete == 0))
                          {
                              // Serial.println("reading");
                              // available bytes (HTTP request) : read them until HTTP head is complete : blank line CR/LF)
                              isHTTPrequestComplete = clientReadBuffer(&client, rint_NbBytesToRead, string_ReceivedLine);
                              Serial.println("HTTP request START =========================================");
                              Serial.print(string_ReceivedLine);
                              Serial.println("HTTP request END ===========================================");
                              rint_NbBytesToRead = client.available();               

                              rint_NbWhile++;
                          }

                          //Serial.print("Nb while: ");
                          //Serial.println(rint_NbWhile);
                          
                          
                         if ( isHTTPrequestComplete == -1)
                         {
                            Serial.println("ERROR : HTTP request exceeds buffer size : STOP");
                            client.stop();
                         } else 
                         {
                             
                              // check HTTP request: 
                             if (strstr(string_ReceivedLine,"GET /") != NULL)
                             {
                                // process web page request
                                Serial.println(">>>VALID HTTP : REQUEST ============== : display page ");
                                
                                SendHTMLpage();
                                Serial.println(">>>page sent");
                                delay(500);
                                // triggers the page display
                                client.stop();
                                
                             } else if (strstr(string_ReceivedLine,"POST /") != NULL)
                             {
                             
                                // process http form request
                                Serial.println(">>>VALID HTTP :  FORM ============== : reset counter ");
                                // here we just expected a click on the reset button
                                // we have to reset the counter in this case

                                // reset counter
                                rint_counter = 0;
                                 Serial.print("rint_counter ");
                                 Serial.println(rint_counter);
                                                            
                                SendHTMLpage();
                                delay(500);
                                // triggers the page display
                                client.stop();
    
                                
                                
                                Serial.println(">>>page sent + RAZ done");
                             } else
                             {
                                Serial.println(">>>HTTP request not complete: keywords not found"  );
                                client.stop();                      
                             }
                         }  // HTTP request process end
                         
  
                     } // end of new client conected      
              } // client found
  } // connected


} // end cyclic task





/** task to maintain Ethernet connection
 * lowest priority: connection renewed every 10mn
 */
inline void cyclic_Task_RefreshConnection(void)
{

      // request ethernet connection
      if (rbi_EthernetConnected)
      { 
            
            // Ethernet connected : can expire 30mn later: so request to maintain
            byte rub_return = Ethernet.maintain();
    
            if ((rub_return == 3) || (rub_return == 1))
            {
              rbi_EthernetConnected = false;
              Serial.println(">>>Ethernet connection lease lost !");
            } else
            {
              Serial.print(">>>Ethernet connection lease credit renewed with code :");
              Serial.println(rub_return);
            }
            
      } else
      {
            // last connection attempt failed : try again
            if (Ethernet.begin(mac) != 0)
            {
                rbi_EthernetConnected = true;
                server.begin();
              
                Serial.print(">>>New connection follow-up : My IP is now: ");
                Serial.println(Ethernet.localIP());
            } // else will try 10mn later
      }

}





/** read part of the received HTTP request
 *  @return is HHTP request complete : false if not complete, true otherwise (HTTP ends with blank line with CR/LF)
 */
int clientReadBuffer(EthernetClient* Ptrclient, int rint_NbBytesToRead, char* stringPtr_ReceivedLine)
{
    boolean rbi_answer = false;
    

    // read buffer
    //Serial.println("dump buffer content ============================== ");
    char c = Ptrclient->read();
    //rint_NbBytesToRead--;
    boolean rbi_bufferOverFlow = false;
    
    while ((rint_NbBytesToRead != 0) && (rbi_bufferOverFlow == false))
    {
              // echo character to console
             /* switch (c)
              {
                default:
                    Serial.write(c);
                    break;

                case ('\r'):
                   Serial.write("-CR-\n");
                   break;
              
                case ('\n'):
                   Serial.write("-LF-\n");
                   break;  
              } */
             
              
              rint_NbBytesToRead--;

              if (rint_currentWR >= (HTTP_SIZE-2))
              {
                  rbi_bufferOverFlow = true;
                  Serial.println("ERROR >>>>  BUFFER overflow ============== ");
              } else
              {
                  // append to line
                  stringPtr_ReceivedLine[rint_currentWR] = c;
                  rint_currentWR++;
              }
              c =Ptrclient->read();         
    }
    //stringPtr_ReceivedLine[rint_currentWR] = c;
    
    // terminate string
    stringPtr_ReceivedLine[rint_currentWR] = 0;
    auto string_size = strlen(stringPtr_ReceivedLine);
       
    // An HTTP request end with CR LF CR LF
    // Serial.println(stringPtr_ReceivedLine);
    if (strstr(stringPtr_ReceivedLine, "\r\n\r\n") != NULL)
    {
         
         rbi_answer = 1;
         
    } else
    {
        Serial.println("ERROR : HTTP client request NOT finished");
        if (rbi_bufferOverFlow)
        {
            rbi_answer =-1;
        } else
        {
          rbi_answer = 0;
        }
    }

    return(rbi_answer);
}



void SendHTMLpage(void)
{
      // send back WEB page
      // 1) send a standard http response header
      HTTP_TARGET.println(http_header);
      // VERY IMPORTANT: end of HTTP section is maked by CR LF
      HTTP_TARGET.println();
              
      // 2) send HTML page
      // beginning 
      HTTP_TARGET.println(html_page_content_begin);
 
      // counter
      HTTP_TARGET.print("<h2 style=\"font-size:500\%;\">");
      HTTP_TARGET.print(rint_counter);   
      HTTP_TARGET.println("</h2>");
      
      // end
      HTTP_TARGET.println(html_page_content_end);
      HTTP_TARGET.println();
}


