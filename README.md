### HackNight BB&N (24 Hr Hackathon)
### Project with Crllect

Crllect’s Part:
https://github.com/crllect/hacknight/tree/main: Electron JS App + Open AI integration
App serves as interface for timer for time inputs and side quest revival
App asks what are some of the users interest through textbox input
Textbox input for hours, minutes, seconds
Stores interests and sends hours, minutes, and seconds in format “xx:xx:xx” to Adafruit IO API
When “000000” is sent to adafruit io by board app registers the timer being either ended or finishing
The app generates a side quest for the user to do for an (user) imputed time for how long they want to take a break for:
Used OpenAi tokens to generate:
const prompt = `Generate ONE sidequest based on these interests and time given, this SOLE sidequest will be AT MOST 1-3 sentences based on these interests. For example, if I input: Interests= { "I like video games and playing music" }, TimeGiven= { "5 minutes" }. The output will be "Play Friday Night Funkin for 5 minutes". If TimeGiven is not displayed or defined, default to a time given that would be acceptable for any generated sidequest. Now that you understand, here is the actual case: Interests= { ${interests} }, TimeGiven= { ${timeGiven} }`;

My Part:
https://github.com/AdaoH3/HacKnightProj: Hardware and API Implementation
Setup button interfaces
Connects to the WiFi and connects to Adafruit IO for time input
Loops until connected
If WiFi gets disconnected at any time board goes back into searching mode
Draws a WiFi Sprite when connected
Draws an idle triangle until API sends 
int x1 = cx + 50 * cos(radAngle);
int y1 = cy + 50 * sin(radAngle);
int x2 = cx + 50 * cos(radAngle + 2 * PI / 3);
int y2 = cy + 50 * sin(radAngle + 2 * PI / 3);
int x3 = cx + 50 * cos(radAngle + 4 * PI / 3);
int y3 = cy + 50 * sin(radAngle + 4 * PI / 3);
Gets all vertices of a triangle for x and y and map then to cos (for x) and sin (for y)
Use function to draw triangle based on those coords with in a for loop to vary radAngle (used with delay)
When ESP32 receives time from API:
Gets the current time in format: xx:xx:xx Hours x Minutes x Seconds (for pomodoro timer)
Clears out screen and starts timer
Uses the hours, minutes, and seconds remaining with a delay of 1 second 
Uses a millis function to check for last button press within 1 second
This is for pause and end functionalities with the two different buttons on the esp32-s2 board
When the timer ends
Sends  "000000" to the Adafruit IO API indicating timer ended to app
Goes into idle triangle state again
