#pragma config(Motor,  motorA,          x_axis,        tmotorEV3_Large, PIDControl, reversed, encoder)
#pragma config(Motor,  motorB,          y_axis,        tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          z_axis,        tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD,          extruderButton, tmotorEV3_Medium, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// This forces the debugstream to be opened
#pragma DebuggerWindows("debugStream");
// Comment out the line below if you want the motors to run
#define DISABLE_MOTORS

const char *fileName = "gcode.txt"; // name of the file it'll be reading

// You need some kind of value here that will never be used in your g-code
const float noParam = -255;

typedef enum tCmdType
{
	G1_NONE,
	G1_START,
	G1_X,
	G1_Y,
	G1_Z,
	G1_E,
	G1_F,
} tCmdType;


#ifndef DISABLE_MOTORS
void waitForMotors();
#endif

float calcDeltaDistance(float &currentPosition, float newPosition);
float calcMotorDegrees(float travelDistance, long gearSize);
void moveMotorAxis(tMotor axis, float degrees);

tCmdType processesCommand(char *buff, int buffLen, float &cmdVal);
bool readNextCommand(char *cmd, int cmdLen, float &x, float &y, float &z, float &e, float &f);
void executeCommand(string gcmd, float x, float y, float z, float e, float f);
long readLine(long fd, char *buffer, long buffLen);

//------------------------------------------------------------------------------------

//this is where you specify your build area

float xAxisPosition = 128;

float yAxisPosition = 128;

float zAxisPosition = 120;

//this is where you specify gear sized for each axis

long gearSizeX = 8;

long gearSizeY = 8;

long gearSizeZ = 8;

//------------------------------------------------------------------------------------

task main()
{
	// Clear all text from the debugstream window
	clearDebugStream();

	//sets LED to flash to show that the printer is printing
	setLEDColor(ledRed);

	float x, y, z, e, f = 0.0;
	long fd = 0;
	char buffer[128];
	long lineLength = 0;

	string gcmd = "G1"; // always a g1

	fd = fileOpenRead(fileName); // fileName = gcode.rtf

	if (fd < 0) // if file is not found/cannot open
	{
		writeDebugStreamLine("Could not open %s", fileName);
		return;
	}
	while (true)
	{
		lineLength = readLine(fd, buffer, 128);
		if (lineLength > 0)
		{
			readNextCommand(buffer, lineLength, x, y, z, e, f); // do these functions
			executeCommand(gcmd, x, y, z, e, f);
		}
		else
			// we're done, no lines left to read from the file
		return;

		// Wipe the buffer by setting its contents to 0
		memset(buffer, 0, sizeof(buffer));

		//LED turns green to show that the print is done
		setLEDColor(ledGreen);

	}
}

//-------------------------------------------------------------------------------------
#ifndef DISABLE_MOTORS
void waitForMotors(){
	while(getMotorRunning(x_axis) || getMotorRunning(y_axis) || getMotorRunning(z_axis)){
		sleep(1);
	}
}
#endif

//void homeAllAxes(){
//	while(getTouchValue(xAxisLimit)== 0)
//		moveMotorTarget(x_axis, 1, 30); //incomplete
//	return;
//}

//------------------------------------------------------------------------------------

// Calculate the distance (delta) from the current position to the new one
// and update the current position
float calcDeltaDistance(float &currentPosition, float newPosition){

	writeDebugStreamLine("calcDeltaDistance(%f, %f)", currentPosition, newPosition);

	float deltaPosition = (currentPosition - newPosition) * -1;
	writeDebugStreamLine("deltaPosition: %f", deltaPosition);
	currentPosition = newPosition;
	writeDebugStreamLine("Updated currentPosition: %f", currentPosition);
	return deltaPosition;
}

// Calculate the degrees the motor has to turn, using provided gear size
float calcMotorDegrees(float travelDistance, long gearSize)
{
	writeDebugStreamLine("calcMotorDegrees(%f, %f)", travelDistance, gearSize);
	return travelDistance * gearSize;
}

// Wrapper to move the motor, provides additional debugging feedback
void moveMotorAxis(tMotor axis, float degrees)
{
	writeDebugStreamLine("moveMotorAxis: motor: %d, degrees: %f", axis, degrees);
#ifndef DISABLE_MOTORS
	moveMotorTarget(axis, round(degrees), 50);
#endif
	return;
}


//------------------------------------------------------------------------------------

// We're passed a single command, like "G1" or "X12.456"
// We need to split it up and pick the value type (X, or Y, etc) and float value out of it.
tCmdType processesCommand(char *buff, int buffLen, float &cmdVal)
{
	// This is the default value
	cmdVal = noParam; // cmdVal = -255

	// Anything less than 2 characters is bogus
	if (buffLen < 2)
		return G1_NONE;

	writeDebugStreamLine("processesCommand: buff: %s", buff);

	switch (buff[0])
	{
	case 'G':
		return G1_START;

	case 'X':
		sscanf(buff, "X%f", &cmdVal); return G1_X;

	case 'Y':
		sscanf(buff, "Y%f", &cmdVal); return G1_Y;

	case 'Z':
		sscanf(buff, "Z%f", &cmdVal); return G1_Z;

	case 'E':
		sscanf(buff, "E%f", &cmdVal); return G1_E;

	case 'F':
		sscanf(buff, "F%f", &cmdVal); return G1_F;

	default: return G1_NONE;
	}
}


// Read and parse the next line from file and retrieve the various parameters, if present
bool readNextCommand(char *cmd, int cmdLen, float &x, float &y, float &z, float &e, float &f)
{
	char currCmdBuff[16];
	tCmdType currCmd = G1_NONE;
	int currCmdBuffIndex = 0;
	float currCmdVal = 0;

	x = y = z = e = f = noParam;
	writeDebugStreamLine("\n----------    NEXT COMMAND   -------");
	writeDebugStreamLine("Processing: %s", cmd);

	// Clear the currCmdBuff
	memset(currCmdBuff, 0, sizeof(currCmdBuff));

	for (int i = 0; i < cmdLen; i++)
	{
		currCmdBuff[currCmdBuffIndex] = cmd[i];
		// We process a command whenever we see a space or the end of the string, which is always a 0 (NULL)
		if ((currCmdBuff[currCmdBuffIndex] == ' ') || (currCmdBuff[currCmdBuffIndex] == 0))
		{
			currCmd = processesCommand(currCmdBuff, currCmdBuffIndex + 1, currCmdVal);
			// writeDebugStreamLine("currCmd: %d, currCmdVal: %f", currCmd, currCmdVal);
			switch (currCmd)
			{
			case G1_NONE: break;
			case G1_START: break;
			case G1_X: x = currCmdVal; break;
			case G1_Y: y = currCmdVal; break;
			case G1_Z: z = currCmdVal; break;
			case G1_E: e = currCmdVal; break;
			case G1_F: f = currCmdVal; break;

			}
			// Clear the currCmdBuff
			memset(currCmdBuff, 0, sizeof(currCmdBuff));

			// Reset the index
			currCmdBuffIndex = 0;
		}
		else
		{
			// Move to the next buffer entry
			currCmdBuffIndex++;
		}
	}

	return true;
}

// Use parameters gathered from command to move the motors, extrude, that sort of thing
void executeCommand(string gcmd, float x, float y, float z, float e, float f)
{
	float motorDegrees; 	// Amount the motor has to move
	float deltaPosition;	// The difference between the current position and the one we want to move to

	// execute functions inside this algorithm
	if (strcmp(gcmd, "G1") == 0 ){

		if(x != noParam){
			writeDebugStreamLine("\n----------    X AXIS   -------------");
			deltaPosition = calcDeltaDistance(xAxisPosition, x);
			motorDegrees = calcMotorDegrees(deltaPosition, gearSizeX);
			moveMotorAxis(x_axis, motorDegrees);
		}

		if(y != noParam){
			writeDebugStreamLine("\n----------    Y AXIS   -------------");
			deltaPosition = calcDeltaDistance(yAxisPosition, y);
			motorDegrees = calcMotorDegrees(deltaPosition, gearSizeY);
			moveMotorAxis(y_axis, motorDegrees);
		}

		if(z != noParam){
			writeDebugStreamLine("\n----------    Z AXIS   -------------");
			deltaPosition = calcDeltaDistance(zAxisPosition, z);
			motorDegrees = calcMotorDegrees(deltaPosition, gearSizeZ);
			moveMotorAxis(z_axis, motorDegrees);
		}

#ifndef DISABLE_MOTORS
		waitForMotors();
#endif

	}

	//this is where to add else if statements for new g commands
	else
	{
		//error
		displayCenteredBigTextLine(1 , "error! :: gcmd value is unknown!");
	}
}

// Read the file, one line at a time
long readLine(long fd, char *buffer, long buffLen)
{
	long index = 0;
	char c;

	// Read the file one character at a time until there's nothing left
	// or we're at the end of the buffer
	while (fileReadData(fd, &c, 1) && (index < (buffLen - 1)))
	{
		//writeDebugStreamLine("c: %c (0x%02X)", c, c);
		switch (c)
		{
			// If the line ends in a newline character, that tells us we're at the end of that line
			// terminate the string with a \0 (a NULL)
		case '\r': break;
		case '\n': buffer[index] = 0; return index;
			// It's something other than a newline, so add it to the buffer and let's continue
		default: buffer[index] = c; index++;
		}
	}
	// Make sure the buffer is NULL terminated
	buffer[index] = 0;
	return index;  // number of characters in the line
}
