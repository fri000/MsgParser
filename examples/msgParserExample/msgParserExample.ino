/*  MsgParser example
 *  Ilya Brutman
 *
 *  Run this code and try typing in the following things:
 *  (Make sure your arduino serial window is set to send a carriage return after each message)
 *
 *   "hi"             response will be "Hello there."
 *   "bye"            response will be "So long, thanks for all the fish."
 *   "echo 1234"      response will be "You passed in the number 1234"
 *   "add 50 42"      response will be "50 + 42 = 92"
 *
 */

#include <avr/pgmspace.h>
#include <MsgParser.h>


//list of strings. You can have as many as you want.
//each string is saved in flash memory only and does not take up any ram.
char s0[] PROGMEM = "hi";
char s1[] PROGMEM = "bye";
char s2[] PROGMEM = "echo";
char s3[] PROGMEM = "add";


//This is our look up table. It says which function to call when a particular string is received
FuncEntry_t functionTable[] PROGMEM = {
//   String     Function
    {s0,        sayHello        },
    {s1,        sayBye          },
    {s2,        echo            },
    {s3,        addTwoNumbers   }
    };


//this is the compile time calculation of the length of our look up table.
int funcTableLength = (sizeof functionTable / sizeof functionTable[0]);     //number of elements in the function table

MsgParser myParser;     //this creates our parser




void setup()
{
    Serial.begin(9600);
    Serial.println("Ready for action");
    myParser.setTable(functionTable, funcTableLength);      //tell the parser to use our lookup table
    myParser.setHandlerForCmdNotFound(commandNotFound);     //Tell the parser which function to call when it gets a command it doesn't handle
}// end setup





void loop()
{
    //if we received any bytes from the serial port, pass them to the parser
    while ( Serial.available() )  myParser.processByte(Serial.read () );

}//end loop












void sayHello()
{
    Serial.println("Hello there.");

}



void sayBye()
{
    Serial.println("So long, thanks for all the fish.");
}



void echo()
{
    int aNumber;
    aNumber = myParser.getInt();

    Serial.print("You passed in the number ");
    Serial.println (aNumber);
}





void addTwoNumbers()
{
    int x,y,result;
    x = myParser.getInt();      //get the first number passed in
    y = myParser.getInt();      //get the second number passed in

    result = x + y;             //do the addition

    Serial.print(x);            //print out the result
    Serial.print(" + ");
    Serial.print(y);
    Serial.print(" = ");
    Serial.println (result);
}



//This function is called when the msgParser gets a command that it didnt handle.
void commandNotFound(uint8_t* pCmd, uint16_t length)
{
    Serial.print("Command not found: ");
    Serial.write(pCmd, length); //print out what command was not found
    Serial.println();           //print out a new line
}