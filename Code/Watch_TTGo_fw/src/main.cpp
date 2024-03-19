#include "config.h"


// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;
AXP20X_Class *power; 

// Watch objects
TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
bool irq = false;


uint32_t last = 0;
uint32_t updateTimeout = 0;
uint16_t id = 0;



volatile uint8_t state;
volatile bool irqBMA = false;
volatile bool irqButton = false;
int batterylevel = 0;
bool sessionStored = false;
bool sessionSent = false;

char buf[128];



void initAccelerometer()
{
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_400HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;
    sensor->accelConfig(cfg);
}



void enableAccelerometer()
{
    sensor->enableAccel();
    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, [] {
        // Set interrupt to set irq value to 1
        irq = 1;
    }, RISING); //It must be a rising edge

    sensor->enableFeature(BMA423_STEP_CNTR, true);   // Enable BMA423 step count feature
    sensor->resetStepCounter(); // reset steps
    sensor->enableStepCountInterrupt();  // Turn on step interrupt
}




void initHikeWatch()
{
    // LittleFS
    if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
    // Stepcounter
    // Configure IMU
    // Enable BMA423 step count feature
    // Reset steps
    // Turn on step interrupt
    //Initializing accelerometer
    initAccelerometer();
    enableAccelerometer();
    // Side button
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irqButton = true;
    }, FALLING);
    //!Clear IRQ unprocessed first
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    watch->power->clearIRQ();
    watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
    return;



    
}

void sendDataBT(fs::FS &fs, const char * path)
{
    /* Sends data via SerialBT */
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }
    Serial.println("- read from file:");
    while(file.available()){
        SerialBT.write(file.read());
    }
    file.close();
}

void sendSessionBT()
{
    // Read session and send it via SerialBT
    // Sending session id
    sendDataBT(LITTLEFS, "/id.txt");
    SerialBT.write(';');
    // Sending steps
    sendDataBT(LITTLEFS, "/steps.txt");
    SerialBT.write(';');
    // Sending distance
    sendDataBT(LITTLEFS, "/distance.txt");
    SerialBT.write(';');
    // Send connection termination char
    SerialBT.write('\n');
    
    
}


void saveIdToFile(uint16_t id)
{
    char buffer[10];
    itoa(id, buffer, 10);
    writeFile(LITTLEFS, "/id.txt", buffer);
}

void saveStepsToFile(uint32_t step_count)
{
    char buffer[10];
    itoa(step_count, buffer, 10);
    writeFile(LITTLEFS, "/steps.txt", buffer);
}

void saveDistanceToFile(double distance)
{
    char buffer[10];
    //itoa(distance, buffer, 10);
    sprintf(buffer,"%f", distance);
    writeFile(LITTLEFS, "/distance.txt", buffer);
}

void deleteSession()
{
    deleteFile(LITTLEFS, "/id.txt");
    deleteFile(LITTLEFS, "/distance.txt");
    deleteFile(LITTLEFS, "/steps.txt");
    deleteFile(LITTLEFS, "/coord.txt");
}




void setup()
{   
    
    Serial.begin(115200);

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    //Receive objects for easy writing
    tft = watch->tft;
    sensor = watch->bma;
    
    initHikeWatch();
    SerialBT.begin("Hiking Watch");

    watch->tft->fillScreen(TFT_BLACK);
    watch->tft->setTextFont(4);
    watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);


    watch->tft->drawString("HikeIt",  80, 60, 4);
    watch->tft->drawString("System starting", 30, 115);
      
    delay(5000);
    state = 1;
    
}




void loop()
{
  
    switch (state)
    {
    case 1:
    {
        /* Initial stage */
        //Basic interface
        int per = watch->power->getBattPercentage();
        watch->tft->fillScreen(TFT_BLACK);
        watch->tft->setTextFont(4);
        watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
        if(sessionStored and not sessionSent)
            watch->tft->drawString("D",  210, 0, 4);
        tft->setCursor(0,0);
        tft->print("Battery:");
        tft->print(watch->power->getBattPercentage());
        tft->print("%");
        watch->tft->drawString("To start session",  30, 80, 4);
        watch->tft->drawString("Press the button", 28, 135);
              


        bool exitSync = false;
        
        //Bluetooth discovery
        while (1)
        {
            
            /* Bluetooth sync */

            if (SerialBT.available())
            {
                char incomingChar = SerialBT.read();
                if (incomingChar == 'c' and sessionStored and not sessionSent)
                {
                    
                    watch->tft->fillScreen(TFT_BLACK);
                    watch->tft->setTextFont(4);
                    watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
                    watch->tft->drawString("Synchronizing",  30, 80, 4);
                          
                    bool exitSync = false;
                    delay(2000);
                    sendSessionBT();
                    sessionSent = true;
                    char incomingChar = SerialBT.read();
                    
                }

                if (sessionSent && sessionStored) {
                    // Update timeout before blocking while

                    updateTimeout = 0;
                    last = millis();
                    while(1)
                    {
                        updateTimeout = millis();

                        if (SerialBT.available())
                            incomingChar = SerialBT.read();

                        if (incomingChar == 'r')
                        {
                            Serial.println("Got an R");
                            // Delete session
                            deleteSession();
                            sessionStored = false;
                            sessionSent = false;
                            incomingChar = 'q';
                            exitSync = true;
                            sessionSent = true;

                            watch->tft->fillScreen(TFT_BLACK);
                            watch->tft->setTextFont(4);
                            watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
                            watch->tft->drawString("Synchronization",  30, 80, 4);
                            watch->tft->drawString("completed", 55, 135);
                                 
                            delay(2000);
                            break;
                        }
                        
                        else if ((updateTimeout- last >2000))
                        {
                            Serial.println("Waiting for timeout to expire");
                            updateTimeout = millis();
                            sessionSent = false;
                            exitSync = true;

                            break;
                        }
                    }
                }
            }
            if (exitSync)
            {
                delay(1000);
                watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
                tft->setCursor(0,0);
                tft->print("Battery:");
                tft->print(watch->power->getBattPercentage());
                tft->print("%");
                watch->tft->drawString("To start session",  30, 80, 4);
                watch->tft->drawString("Press the button", 28, 135);
                    
                if(sessionStored and not sessionSent)
                    watch->tft->drawString("D",  210, 0, 4);
                exitSync = false;
            }

            /*      IRQ     */
            if (irqButton) {
                irqButton = false;
                watch->power->readIRQ();
                if (state == 1)
                {
                    state = 2;
                }
                watch->power->clearIRQ();
            }


            
            if (state == 2) {
                if (sessionStored)
                {
                    watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
                    watch->tft->drawString("Overwriting",  55, 100, 4);
                    watch->tft->drawString("session", 70, 130);
                     
                    delay(2000);
                }
                break;
            }
        }
        break;
    }
    case 2:
    {
        /* Hiking session initalisation */
        bool  rlst;
        do {
            // Read the BMA423 interrupt status,
            // need to wait for it to return to true before continuing
            rlst =  sensor->readInterrupt();
        } while (!rlst);
        id = id+1;

        state = 3;
        break;
    }
    case 3:
    {
        /* Hiking session ongoing */
    
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);

        watch->tft->drawString("Starting hike", 45, 100);
        delay(1000);
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
      
        watch->tft->drawString("3", 120, 100);
        delay(1000);
          
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
 
        watch->tft->drawString("2", 120, 100);
        delay(1000);
    
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
          
        watch->tft->drawString("1", 120, 100);
        delay(1000);
              
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
        
        watch->tft->drawString("GO!", 100, 100);
        delay(1000);
            
        watch->tft->fillRect(0, 0, 240, 240, TFT_GREEN);
        
        delay(1000);
        
        
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
         
        SerialBT.write(SerialBT.read());

        int sec = 0;
        int outputsec = 0;
        int hour = 0;
        int min = 0;
        int step = 0;
        double distance = 0;
        int batterylevel = 0;
        last = millis();


        while (!irqButton) {
            tft->setCursor(0,0);
            tft->print("Battery:");
            tft->print(watch->power->getBattPercentage());
            tft->print("%");
            step = sensor->getCounter();
            clock_t tStart=clock();
            sec = (millis()-last)/1000;
            hour = sec / 3600;
            min = (sec / 60) % 60;
            outputsec = sec % 60; 


            watch->tft->setCursor(10, 70);
            watch->tft->print("Steps: ");
            tft->print(step);
            distance = step*0.000762;
            watch->tft->setCursor(10, 120);
            watch->tft->print("Dist: ");
            tft->print(distance);
            watch->tft->print(" km");
            
    
            watch->tft->setCursor(10, 170);
            watch->tft->print("Time: ");
            tft->print(hour);
            tft->print("h ");
            tft->print(min);
            tft->print("min ");
            tft->print(outputsec);
            tft->print("s");
            updateTimeout = 0;

            delay(10);
        }

        sensor->resetStepCounter();
        irqButton = false;
        watch->power->readIRQ();
        watch->power->clearIRQ();
        
        state = 4;
        saveStepsToFile(step);
        saveDistanceToFile(distance);
        saveIdToFile(id);
        break;
        //reset step-counter
    }
    case 4:
    {
        //Save hiking session data



        sessionStored = true;
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
 
        watch->tft->drawString("Storing data", 45, 100);
        delay(1000);           
        watch->tft->fillRect(0, 0, 240, 240, TFT_BLACK);
 
        watch->tft->drawString("Session stored", 30, 100);
        delay(1500);
        state = 1;  
        break;
    }
    default:
        // Restart watch
        ESP.restart();
        break;
    }
}