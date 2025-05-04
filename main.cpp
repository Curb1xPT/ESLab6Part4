#include "mbed.h"
#include "LCDi2c.h"

LCDi2c lcd(LCD20x4);

char str[32];

AnalogIn potentiometer(A0);
AnalogIn lm35(A1);
AnalogIn mq2(A3);
PwmOut buzzer(D9);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

float lm35Reading = 0.0;
float lm35TempC = 0.0;

float analogReadingScaledWithTheLM35Formula(float analogReading);

float potentiometerReading = 0.00f;

bool overTempDetect = false;
bool overTempDetectState = false;
bool gasDetectState = false;

DigitalOut keypadRow[4] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn keypadCol[4] = {PB_12, PB_13, PB_15, PC_6};

char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

void inputsInit() {
    for(int i = 0; i < 4; i++) {
        keypadCol[i].mode(PullUp);
    }
}

void outputsInit() {
    buzzer = 0;
}

char matrixKeypadScan() {
    for(int row = 0; row < 4; row++) {
        for(int i = 0; i < 4; i++) {
            keypadRow[i] = 1;
        }
        keypadRow[row] = 0;

        for(int col = 0; col < 4; col++) {
            if(keypadCol[col] == 0) {
                return matrixKeypadIndexToCharArray[row * 4 + col];
            }
        }
    }
    return '\0';
}

char matrixKeypadUpdate() {
    static char lastKey = '\0';
    static Timer debounceTimer;
    static bool debounceTimerStarted = false;
    char currentKey = matrixKeypadScan();

    if(currentKey != '\0' && lastKey == '\0' && !debounceTimerStarted) {
        debounceTimer.reset();
        debounceTimer.start();
        debounceTimerStarted = true;
    }

    if(debounceTimerStarted && debounceTimer.elapsed_time().count() > 20000) {
        debounceTimer.stop();
        debounceTimerStarted = false;
        lastKey = currentKey;
        if(currentKey != '\0') {
            return currentKey;
        }
    }

    if(currentKey == '\0') {
        lastKey = '\0';
    }

    return '\0';
}

int main() {

    lcd.cls();
    lcd.display(CURSOR_OFF);
    lcd.display(BLINK_OFF);

    inputsInit();
    outputsInit();

    uartUsb.write("\nSystem is on.\r\n", 16);

    while (true) {

        float potentiometerReading = potentiometer.read();

        lm35Reading = lm35.read();
        lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);

        float gasLevel = mq2.read();

        if (lm35TempC <= 30.0 && gasLevel > 0.8f) {
            lcd.cls();
            str[0] = '\0';
            lcd.locate(0, 0);
            sprintf(str, "Alarm is off");
            lcd.printf("%s \n", str);
            ThisThread::sleep_for(1000ms);
            lcd.locate(0, 0);
            str[0] = '\0';
        } else if (lm35TempC > 30.0 && gasLevel > 0.8f) {
            str[0] = '\0';
            lcd.locate(0, 0);
            sprintf(str, "Alarm is on");
            lcd.printf("%s \n", str);
            lcd.locate(0, 1);
            sprintf(str, "Warning:");
            lcd.printf("%s \n", str);
            lcd.locate(0, 2);
            sprintf(str, "Temp. Over 30 C");
            lcd.printf("%s \n", str);
            lcd.locate(13, 2);
            lcd.printf("%c", (char)223);
            ThisThread::sleep_for(1000ms);
            lcd.locate(0, 0);
            str[0] = '\0';
        } else if (lm35TempC <= 30.0 && gasLevel <= 0.8f) {
            str[0] = '\0';
            lcd.locate(0, 0);
            sprintf(str, "Alarm is on");
            lcd.printf("%s \n", str);
            lcd.locate(0, 1);
            sprintf(str, "Warning:");
            lcd.printf("%s \n", str);
            lcd.locate(0, 2);
            sprintf(str, "Gas Detected");
            lcd.printf("%s \n", str);
            ThisThread::sleep_for(1000ms);
            lcd.locate(0, 0);
            str[0] = '\0';
        } else if (lm35TempC > 30.0 && gasLevel <= 0.8f) {
            str[0] = '\0';
            lcd.locate(0, 0);
            sprintf(str, "Alarm is on");
            lcd.printf("%s \n", str);
            lcd.locate(0, 1);
            sprintf(str, "Warning:");
            lcd.printf("%s \n", str);
            lcd.locate(0, 2);
            sprintf(str, "Temp. Over 30 C");
            lcd.printf("%s \n", str);
            lcd.locate(13, 2);
            lcd.printf("%c", (char)223);
            lcd.locate(0, 3);
            sprintf(str, "Gas Detected");
            lcd.printf("%s", str);
            ThisThread::sleep_for(1000ms);
            lcd.locate(0, 0);
            str[0] = '\0';
            lcd.cls();
        }
        
        ThisThread::sleep_for(1ms);
    }
}


float analogReadingScaledWithTheLM35Formula(float analogReading)
{
    return analogReading * 330.0;
}