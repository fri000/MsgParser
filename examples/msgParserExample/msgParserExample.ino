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
const char s0[] PROGMEM = "?";		const char help0[] PROGMEM = "Prints this help menu.";
const char s1[] PROGMEM = "hi";		const char help1[] PROGMEM = "Prints a hello message.";
const char s2[] PROGMEM = "bye";	const char help2[] PROGMEM = "Prints a goodbye message.";
const char s3[] PROGMEM = "echo";	const char help3[] PROGMEM = "[a number]. Prints back the number.";
const char s4[] PROGMEM = "add";	const char help4[] PROGMEM = "[x] [y]. Prints results of x + y.";


//This is our look up table. It says which function to call when a particular string is received
const FuncEntry_t functionTable[] PROGMEM = {
//  String, help, Function
    {s0, help0,   printHelp       },
    {s1, help1,   sayHello        },
    {s2, help2,   sayBye          },
    {s3, help3,   echo            },
    {s4, help4,   addTwoNumbers   }
    };


//this is the compile time calculation of the length of our look up table.
int funcTableLength = (sizeof functionTable / sizeof functionTable[0]);     //number of elements in the function table

MsgParser myParser;     //this creates our parser




void setup()
{
    Serial.begin(115200);
    Serial.println("Ready for action");
    myParser.setTable(functionTable, funcTableLength);      //tell the parser to use our lookup table
    myParser.setHandlerForCmdNotFound(commandNotFound);     //Tell the parser which function to call when it gets a command it doesn't handle
	printHelp();
}// end setup





void loop()
{
    //if we received any bytes from the serial port, pass them to the parser
    while ( Serial.available() )  myParser.processByte(Serial.read () );

}//end loop








void printHelp()
{
	//For each command that the parser knows about...
	for( int i = 0; i < myParser.numCmds(); i++)
	{
		Serial.print(myParser.cmdString(i)); //print the command name
		Serial.print(" - ");
		Serial.print(myParser.cmdDesc(i)); //print the command description
		Serial.println("");
	}
}



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



//This function is called when the msgParser gets a command that it didn't handle.
void commandNotFound(uint8_t* pCmd, uint16_t length)
{
    Serial.print("Command not found: ");
    Serial.write(pCmd, length); //print out what command was not found
    Serial.println();           //print out a new line
}
