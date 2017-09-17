
enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

enum _actionState {
  AS_IDLE,
  AS_WAITING_FOR_RESPONSE
};

byte actionState = AS_IDLE;
unsigned long lastActionTime = 0;

byte parseState = PS_DETECT_MSG_TYPE;
char buffer[20];
byte pos = 0;

int httpResult = 0;
int contentLength = 0;

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void parseATText(byte b) {

#ifdef DEBUG_SERIAL
  Serial.write(b);
#endif

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) ) 
    resetBuffer(); // just to be safe
  
    #ifdef DEBUG_SERIAL
   // Detailed debugging
   /*Serial.println();
   Serial.print("state = ");
   Serial.println(parseStat);
   Serial.print("b = ");
   Serial.println(b);
   Serial.print("pos = ");
   Serial.println(pos);
   Serial.print("buffer = ");
   Serial.println(buffer);*/
   #endif
   

  switch (parseState) {
  case PS_DETECT_MSG_TYPE: 
    {
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          parseState = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( b == ':' ) {
#ifdef DEBUG_SERIAL
          Serial.print("Checking message type: ");
          Serial.println(buffer);
#endif

          if ( strcmp(buffer, "+HTTPACTION:") == 1 ) {
#ifdef DEBUG_SERIAL
            Serial.println("Received HTTPACTION");
#endif
            parseState = PS_HTTPACTION_TYPE;
          }
          else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
#ifdef DEBUG_SERIAL
            Serial.println("Received HTTPREAD"); 
#endif           
            parseState = PS_HTTPREAD_LENGTH;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
#ifdef DEBUG_SERIAL
        Serial.print("Ignoring echo: ");
        Serial.println(buffer);
#endif
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_TYPE:
    {
      if ( b == ',' ) {
#ifdef DEBUG_SERIAL
        Serial.print("HTTPACTION type is ");
        Serial.println(buffer);
#endif
        parseState = PS_HTTPACTION_RESULT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_RESULT:
    {
      if ( b == ',' ) {
#ifdef DEBUG_SERIAL
        Serial.print("HTTPACTION result is ");
        Serial.println(buffer);
#endif
        httpResult = atoi(buffer);
        parseState = PS_HTTPACTION_LENGTH;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_LENGTH:
    {
      if ( b == '\n' ) {
#ifdef DEBUG_SERIAL
        Serial.print("HTTPACTION length is ");
        Serial.println(buffer);
#endif

        contentLength = atoi(buffer);
        // now request content
        if ( contentLength > 0 ) {
          GSM_PORT.print("AT+HTTPREAD=0,");
          GSM_PORT.println(buffer);
        }
        else
          actionState = AS_IDLE;

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_LENGTH:
    {
      if ( b == '\n' ) {
        contentLength = atoi(buffer);
#ifdef DEBUG_SERIAL
        Serial.print("HTTPREAD length is ");
        Serial.println(contentLength);

        Serial.print("HTTPREAD content: ");
#endif

        parseState = PS_HTTPREAD_CONTENT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_CONTENT:
    {
      // for this demo I'm just showing the content bytes in the serial monitor
#ifdef DEBUG_SERIAL
      Serial.write(b);
#endif

      contentLength--;

      if ( contentLength <= 0 ) {

        // all content bytes have now been read

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();

#ifdef DEBUG_SERIAL
        Serial.print("\n\n\n\n");
#endif

        actionState = AS_IDLE;
      }
    }
    break;
  }
}

void sendGSM(const char* msg, int waitMs = 500) {
  GSM_PORT.println(msg);
  while(GSM_PORT.available()) {
    parseATText(GSM_PORT.read());
  }
  delay(waitMs);
}
