/*  MsgParser
 *  Ilya Brutman
 *
 */

#ifndef MsgParser_h
#define MsgParser_h

#ifdef __cplusplus      //use extern "C" to make msgParser compatible with c++ compilers
 extern "C" {
#endif


#include <avr/pgmspace.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>




//********************** CONSTANTS **********************

static const uint8_t MSGPARSER_MAX_BUFFER_SIZE = 64;                //This is the max length of a received message
static const uint8_t MSGPARSER_DESCRIPTION_SIZE = 128;              //This is the length of a description string, that is the help msg for a command.




//************************ TYPES ************************

//define a pointer to a function that returns void and accepts no parameters
typedef void (*FuncPtr_t)(void);

// define a point to a function that returns void and accepts a pointer to a byte array and array length.
typedef void (*cmdNotFoundHandler_t)(uint8_t*, uint16_t);


typedef struct
{
    char* pCmdString;  //pointer to a null terminated char array
    char* pDescString; //Short help text about this command. Pointer to a null terminated char array
    FuncPtr_t pFunc;   //pointer to a function

}FuncEntry_t;








class MsgParser
{
public:

    MsgParser();                                    //constructor
    //~MsgParser();                                   //destructor
    void    setTable(FuncEntry_t* newFunctTable, uint8_t newFunctTableLength);
    void    useStartByteSet(bool newStatus);
    void    setEndByte(uint8_t newEndByte);
    void    setHandlerForCmdNotFound(cmdNotFoundHandler_t pNewHandlerFunc);
    void    processByte(uint8_t newByte);
    long    getLong();
    int     getInt();
    float   getFloat();
	int     numCmds();
	char*   cmdString(uint8_t cmdIndex);
	char*   cmdDesc(uint8_t cmdIndex);
    uint8_t version();



private:

    //***************************************************
    //  Private Types
    //***************************************************
    typedef enum
    {
        WAITING_FOR_START_BYTE = 0,
        READING_MESSAGE,
    } STATE_T;


    //***************************************************
    //  Private Functions
    //***************************************************
    void    clearTheBuffer();


    //***************************************************
    //  Private Variables
    //***************************************************
    FuncEntry_t m_bufStruct;

    char    buffer[MSGPARSER_MAX_BUFFER_SIZE];     //buffer that holds incoming data
    uint8_t bufferlength;                          //length of this buffer, which will get set to MAX_BUFFER_SIZE
    uint8_t bufferWriteIndex;                      //index of the first free space in the buffer
    char*   bufferReadPtr;                         //

    char    origMsg[MSGPARSER_MAX_BUFFER_SIZE];    //Before we start partitioning the received message in "buffer",
                                                   // we make a copy of it into this buffer.

    char*   msgParserCommand;                      //points to the first "token" in the receive buffer

    uint8_t msgStartByte;                          //this byte marks the start of the message
    uint8_t msgEndByte;                            //this byte marks the end of the message
    bool    useStartByte;                          //flag if the parser should use a start byte or not;

    STATE_T myState;                               //keeps track of which state we are in

    FuncEntry_t* funcTable;                        //pointer to the function lookup table in FLASH space
    uint8_t funcTableLength;                       //length of the functions table

    cmdNotFoundHandler_t pCmdNotFoundFunc;         // Function to call if we received a command that we are not going to handle.
	char   m_pDescBuf[MSGPARSER_DESCRIPTION_SIZE];  // When we returning a string from PROGMEM, we first copy it here and return pointer to this buffer.
}; //end MsgParser




#ifdef __cplusplus
 }
#endif

#endif //MsgParser_h