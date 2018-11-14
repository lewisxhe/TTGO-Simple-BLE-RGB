#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID                "0000cbbb-0000-1000-8000-00805f9b34fb"
#define COLOUR_CHARACTERISTIC_UUID  "0000cbb1-0000-1000-8000-00805f9b34fb"
#define MODE_CHARACTERISTIC_UUID    "0000cbb2-0000-1000-8000-00805f9b34fb"
#define WS2812B_OUTPUT_PIN          13 
#define NUMPIXELS                   120

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, WS2812B_OUTPUT_PIN, NEO_GRB + NEO_KHZ800);

BLEServer *pServer = NULL;
BLECharacteristic *pModeCharacteristic = NULL;
BLECharacteristic *pColourCharacteristic = NULL;
uint8_t mode = 0;

void _set_colour(uint32_t colour);

class MyServerCallbacks : public BLEServerCallbacks
{
    // This function is called when connecting
    void onConnect(BLEServer *pServer){};
    // This function is called when disconnected
    void onDisconnect(BLEServer *pServer) {}
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    // This function is called when there is a write event
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (pCharacteristic->getHandle() == pColourCharacteristic->getHandle())
        {
            mode = 255;
            if (value.length() == 4)
            {
                // Serial.printf("New value: R:%d G:%d B:%d L:%d\n", value[0], value[1], value[2], value[3]);
                strip.setBrightness(value[3]);
                // GRB
                // _set_colour(strip.Color(value[1], value[0], value[2]));
                // RGB
                _set_colour(strip.Color( value[0],value[1], value[2]));
            }
        }
        if (pCharacteristic->getHandle() == pModeCharacteristic->getHandle())
        {
            mode = value[0];
        }
    }
};

void ble_start(void)
{
    BLEDevice::init("TTGO-RGB");

    // Create one server
    pServer = BLEDevice::createServer();

    // Create Bluetooth event callback
    pServer->setCallbacks(new MyServerCallbacks());

    // Create ble server by this uuid
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a feature and give it read and write permissions
    pColourCharacteristic = pService->createCharacteristic(
        COLOUR_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    pModeCharacteristic = pService->createCharacteristic(
        MODE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    // Create Characteristic event callback
    pColourCharacteristic->setCallbacks(new MyCallbacks());
    pModeCharacteristic->setCallbacks(new MyCallbacks());
    // Start ble
    pService->start();

    // Turn on the radio
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void _set_colour(uint32_t colour)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, colour);
        strip.show();
    }
}

void setup(void)
{
    Serial.begin(115200);
    ble_start();
    strip.setBrightness(50);
    strip.begin();
    strip.show();
}

void TTGO_RGB_breathe(void)
{
    for (int j = 0; j < 256; j++)
    {
        for (uint16_t i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, strip.Color(0, 0, 0, strip.gamma8(j)));
        }
        strip.show();
    }
    delay(2000);
    for (int j = 255; j >= 0; j--)
    {

        for (uint16_t i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, strip.Color(0, 0, 0, strip.gamma8(j)));
        }
        strip.show();
    }
    delay(2000);
}

uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void TTGO_RGB_rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)
    {
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void TTGO_RGB_rainbowCycle(uint8_t wait)
{
    uint16_t i, j;
    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

//Theatre-style crawling lights with rainbow effect
void TTGO_RGB_theaterChaseRainbow(uint8_t wait)
{
    for (int j = 0; j < 256; j++)
    { // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, Wheel((i + j) % 255)); //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, 0); //turn every third pixel off
            }
        }
    }
}
//Theatre-style crawling lights.
void TTGO_RGB_theaterChase(uint32_t c, uint8_t wait)
{
    for (int j = 0; j < 10; j++)
    { //do 10 cycles of chasing
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, c); //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, 0); //turn every third pixel off
            }
        }
    }
}

void loop(void)
{
    switch (mode)
    {
    case 0x00:
        TTGO_RGB_breathe();
        break;
    case 0x01:
        TTGO_RGB_theaterChaseRainbow(20);
        break;
    case 0x02:
        TTGO_RGB_rainbow(20);
        break;
    case 0x03:
        TTGO_RGB_rainbowCycle(20);
        break;
    case 0x04:
        TTGO_RGB_theaterChase(strip.Color(127, 0, 0), 50);
        break;
    default:
        break;
    }
}