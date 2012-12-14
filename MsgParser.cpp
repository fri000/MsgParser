/*  MsgParser
 *  Ilya Brutman
 *
 */

#include <avr/pgmspace.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "MsgParser.h"



//===============================================================================
// Constructor
//===============================================================================
MsgParser::MsgParser()
{
    bufferlength     = MSGPARSER_MAX_BUFFER_SIZE;
    bufferWriteIndex = 0;
    bufferReadPtr    = buffer;
    msgParserCommand = NULL;
    msgStartByte     = '/';
    msgEndByte       = '\r';    //by default, use the carriage return as the end byte
    useStartByte     = false;
    myState          = WAITING_FOR_START_BYTE; //This just needs to be some valid value, for now.
    funcTable        = NULL;
    funcTableLength  = 0;
    pCmdNotFoundFunc = NULL;

    // Call our useStartByteSet function to set myState to the correct state.
    // Our state will be based on whether we are using the start byte or not.
    useStartByteSet(useStartByte);
      
}// end constructor






//================================================================================
// FUNCTION:    void setTable(...)
//
// DESCRIPTION: sets a new function table to use
//
// INPUT:       *newFunctTable      - pointer to the function table to use
//              newFunctTableLength - number of entries in that table
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::setTable(FuncEntry_t* newFunctTable, uint8_t newFunctTableLength)
{
    if(newFunctTable != NULL)
    {
        funcTable = newFunctTable;
        funcTableLength = newFunctTableLength;
    }
}// end setTable






//================================================================================
// FUNCTION:    void useStartByteSet(...)
//
// DESCRIPTION: Sets the option whether the parser should use a start byte or not.
//
// INPUT:       newStatus -  true if the parser should use a start byte
//                        - false if the parser should not.
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::useStartByteSet(bool newStatus)
{
    useStartByte = newStatus;

    //if we are currently waiting for the start byte and the new option is to
    // not wait for the start byte
    if( (myState == WAITING_FOR_START_BYTE) &&
        (useStartByte == false)
      )
    {
        //then we have to stop waiting for it. We do this by updating our state.
        myState = READING_MESSAGE;
    }
    else
    {
        //otherwise, do nothing more
    }
}// end useStartByteSet




//================================================================================
// FUNCTION:    void setHandlerForCmdNotFound(...)
//
// DESCRIPTION: Sets a new call handler function for when a command is not found.
//
// INPUT:       pNewHandlerFunc - new call back function. This is the function
//                  to call when we receive a command that we don't process.
//
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::setHandlerForCmdNotFound(cmdNotFoundHandler_t pNewHandlerFunc)
{
    pCmdNotFoundFunc = pNewHandlerFunc;
}//end setHandlerForCmdNotFound




//================================================================================
// FUNCTION:    void processByte(...)
//
// DESCRIPTION: Process a new byte. If this new byte completes a message then the function
//              will be called that correspondes with the received string, if such function exists.
//
// INPUT:       newByte - the byte to process
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::processByte(uint8_t newByte)
{    
    switch (myState)
    {
    case WAITING_FOR_START_BYTE:

        if( newByte == msgStartByte )
        {
            myState = READING_MESSAGE;
        }
        else
        {
            //ignore all other bytes
        }
        break;


    case READING_MESSAGE:
    
        if( newByte == msgEndByte )
        {
            //we just got the end byte and the message is now fully received. Time to parse it.
            if(useStartByte == true)
                myState = WAITING_FOR_START_BYTE;

            //***process message***
            bool msgFound = false;
            char *ptr = bufferReadPtr;                      // make ptr point to start of Buffer.
            char *rest;                                     // to point to the rest of the string after token extraction.


            //We are about to edit our receive buffer, so make a copy of it first.
            memcpy(origMsg, buffer, bufferWriteIndex);


            msgParserCommand = strtok_r(ptr, " ", &rest);   //extract the command from the received message   [side note: this replaces the first space with a null]
            bufferReadPtr = rest;                           // rest contains the leftover part..assign it to ptr...and start tokenizing again.


            //search through the lookup table and try to find the string
            uint8_t i = 0;                                  //index variable
            while( (msgFound == false) && (i < funcTableLength) )
            {
                memcpy_P( &bufStruct, &(funcTable[i]), sizeof(bufStruct) );     //pull a function table entry out of flash and into ram.

                if ( strcmp_P( msgParserCommand, bufStruct.myCmd ) == 0)
                {
                    msgFound = true;
                }
                else
                {
                    ++i;
                }
            }


            //at this point we either found a matching string in the table or reached the end of it
            if(i < funcTableLength) //check if the message was found in the table
            {
                //we have a match, call the corresponding function
                (*bufStruct.myFunction)();
            }
            else
            {
                //The received string was not found in our function table.
                // Do we have a handler function that we can call when we have a command not found?
                if(pCmdNotFoundFunc != NULL)
                {
                    // Yes we do. Call that function and pass it the string that we didn't handle.
                    (*pCmdNotFoundFunc)((uint8_t*)origMsg, bufferWriteIndex);
                }
                else
                {
                    // We don't have a handler function defined.
                    // So we don't call any function. We just clear our receive buffer and start
                    // waiting for the next message.
                }
            }

            clearTheBuffer();

        }
        else
        {
            //the new byte is not the end byte
            //therefor this is not the end of the message. add the received byte to the buffer
            buffer[bufferWriteIndex] = newByte;
            bufferWriteIndex++;

            //check if we have reached the end of the buffer without receiving an end byte
            if(bufferWriteIndex == bufferlength)
            {
                //yes we have, our buffer is full.
                clearTheBuffer();

                if(useStartByte == true)
                {
                    myState = WAITING_FOR_START_BYTE;
                }
                else
                {
                    myState = READING_MESSAGE;
                }
            }
            else
            {
                //we still have space in the buffer for at least 1 more byte
                //do nothing
            }
        }

        break;

    default:
        //should never ever get here.
        break;

    }

}// end processByte




//================================================================================
// FUNCTION:    void setEndByte(...)
//
// DESCRIPTION: Sets a new byte to use as the end.
//
// INPUT:       newEndByte - the new byte to use
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::setEndByte(uint8_t newEndByte)
{
    msgEndByte = newEndByte;
}// end setEndByte




//================================================================================
// FUNCTION:    void getLong(...)
//
// DESCRIPTION: Parsers the received string for a number and returns it as a long
//
// INPUT:       Nothing
//
// RETURNS:     The number as a long.
//              Or 0, if no number is found in the remainder of the string
//================================================================================
long MsgParser::getLong()
{
    char *numPtr;
    char *ptr = bufferReadPtr;                      // make ptr point to start of Buffer.
    char *rest;                                     // to point to the rest of the string after token extraction.

    numPtr = strtok_r(ptr, " ", &rest);            //extract the number from the received message   [side note: this replaces the first space with a null]
    bufferReadPtr = rest;                          // rest contains the leftover part..assign it to ptr...and start tokenizing again.

    return( atol(numPtr) );

}//end getLong




//================================================================================
// FUNCTION:    void getInt(...)
//
// DESCRIPTION: Parsers the received string for a number and returns it as a int
//
// INPUT:       Nothing
//
// RETURNS:     The number as a int.
//              Or 0, if no number is found in the remainder of the string
//================================================================================
int MsgParser::getInt()
{
    char *numPtr;
    char *ptr = bufferReadPtr;                     // make ptr point to start of Buffer.
    char *rest;                                    // to point to the rest of the string after token extraction.

    numPtr = strtok_r(ptr, " ", &rest);            //extract the number from the received message   [side note: this replaces the first space with a null]
    bufferReadPtr = rest;                          // rest contains the leftover part..assign it to ptr...and start tokenizing again.

    return( atoi(numPtr) );

}//end getInt





//================================================================================
// FUNCTION:    void getFloat(...)
//
// DESCRIPTION: Parsers the received string for a number and returns it as a float
//
// INPUT:       Nothing
//
//
// RETURNS:     The number as a float.
//              Or 0, if no number is found in the remainder of the string
//================================================================================
float MsgParser::getFloat()
{
    char *numPtr;
    char *ptr = bufferReadPtr;                     // make ptr point to start of Buffer.
    char *rest;                                    // to point to the rest of the string after token extraction.

    numPtr = strtok_r(ptr, " ", &rest);            //extract the number from the received message   [side note: this replaces the first space with a null]
    bufferReadPtr = rest;                          // rest contains the leftover part..assign it to ptr...and start tokenizing again.

    return( atof(numPtr) );

}//end getFloat













//================================================================================
// FUNCTION:    void clearTheBuffer(...)
//
// DESCRIPTION: empties the receive buffer
//
// INPUT:       Nothing
//
//
// RETURNS:     Nothing
//================================================================================
void MsgParser::clearTheBuffer()
{
    bufferWriteIndex = 0;                     //clear the buffer by setting the buffer write index to the beginning
    bufferReadPtr = buffer;                   //move the read pointer back to the start of the buffer
    memset(buffer, NULL, bufferlength);       //overwrite the whole buffer with nulls

}//end clearTheBuffer








//================================================================================
// FUNCTION:    void version(...)
//
// DESCRIPTION: Returns the version number of the MsgParser class
//
// INPUT:       None
//
// RETURNS:     The version number of the MsgParser class
//================================================================================
uint8_t MsgParser::version()
{
    return  0x04;              //this is our version number
}//end version