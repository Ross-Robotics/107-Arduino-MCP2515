/**
 * @brief   This example enables the loopback mode to test the transmission and reception of CAN frames via MCP2515 without any physical bus connection.
 * @author  Alexander Entinger, MSc / LXRobotics GmbH
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <SPI.h>
#undef max
#undef min

#include <ArduinoMCP2515.h>

#include <algorithm>

/**************************************************************************************
 * FUNCTION DECLARATION
 **************************************************************************************/

void mcp2515_spi_select();
void mcp2515_spi_deselect();
uint8_t spi_transfer(uint8_t const data);
void mcp2515_onReceiveBufferFull(uint32_t const id, uint8_t const * data, uint8_t const len);
void mcp2515_OnTransmitBufferEmpty(ArduinoMCP2515 * this_ptr);

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

typedef struct
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
} sCanTestFrame;

/**************************************************************************************
 * GLOBAL CONSTANTS
 **************************************************************************************/

static sCanTestFrame const test_frame_1 = { 0x00000001, {0}, 0 };                                              /* Minimum (no) payload */
static sCanTestFrame const test_frame_2 = { 0x00000002, {0xCA, 0xFE, 0xCA, 0xFE, 0, 0, 0, 0}, 4 };             /* Between minimum and maximum payload */
static sCanTestFrame const test_frame_3 = { 0x00000003, {0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE}, 8 }; /* Maximum payload */
static sCanTestFrame const test_frame_4 = { 0x40000004, {0}, 0 };                                              /* RTR frame */
static sCanTestFrame const test_frame_5 = { 0x000007FF, {0}, 0 };                                              /* Highest standard 11 bit CAN address */
static sCanTestFrame const test_frame_6 = { 0x80000800, {0}, 0 };                                              /* Lowest extended 29 bit CAN address */
static sCanTestFrame const test_frame_7 = { 0x9FFFFFFF, {0}, 0 };                                              /* Highest extended 29 bit CAN address */

static std::array<sCanTestFrame, 7> const CAN_TEST_FRAME_ARRAY =
{
  test_frame_1,
  test_frame_2,
  test_frame_3,
  test_frame_4,
  test_frame_5,
  test_frame_6,
  test_frame_7
};

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

ArduinoMCP2515 mcp2515(mcp2515_spi_select,
                       mcp2515_spi_deselect,
                       spi_transfer,
                       mcp2515_onReceiveBufferFull,
                       mcp2515_OnTransmitBufferEmpty);

/**************************************************************************************
 * CALLBACK FUNCTIONS
 **************************************************************************************/

void onMCP2515ExternalEvent()
{
  mcp2515.onExternalEventHandler();
}

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(9600);
  while(!Serial) { }

  /* Setup SPI access */
  SPI.begin();
  pinMode(MKRCAN_MCP2515_CS_PIN, OUTPUT);
  digitalWrite(MKRCAN_MCP2515_CS_PIN, HIGH);

  /* Attach interrupt handler to register MCP2515 signaled by taking INT low */
  pinMode(MKRCAN_MCP2515_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_MCP2515_INT_PIN), onMCP2515ExternalEvent, FALLING);

  mcp2515.begin();
  mcp2515.setBitRate(CanBitRate::BR_250kBPS);
  mcp2515.setLoopbackMode();

  std::for_each(CAN_TEST_FRAME_ARRAY.cbegin(),
                CAN_TEST_FRAME_ARRAY.cend(),
                [](sCanTestFrame const frame)
                {
                  if(!mcp2515.transmit(frame.id, frame.data, frame.len)) {
                    Serial.println("ERROR TX");
                  }
                  delay(10);
                });
}

void loop()
{

}

/**************************************************************************************
 * FUNCTION DEFINITION
 **************************************************************************************/

void mcp2515_spi_select()
{
  digitalWrite(MKRCAN_MCP2515_CS_PIN, LOW);
}

void mcp2515_spi_deselect()
{
  digitalWrite(MKRCAN_MCP2515_CS_PIN, HIGH);
}

uint8_t spi_transfer(uint8_t const data)
{
  return SPI.transfer(data);
}

void mcp2515_onReceiveBufferFull(uint32_t const id, uint8_t const * data, uint8_t const len)
{
  Serial.print("ID");
  if(id & MCP2515::CAN_EFF_BITMASK) Serial.print("(EXT)");
  if(id & MCP2515::CAN_RTR_BITMASK) Serial.print("(RTR)");
  Serial.print(" ");
  Serial.print(id, HEX);
  
  Serial.print(" DATA[");
  Serial.print(len);
  Serial.print("] ");
  std::for_each(data,
                data+len,
                [](uint8_t const elem) {
                  Serial.print(elem, HEX);
                  Serial.print(" ");
                });
  Serial.println();
}

void mcp2515_OnTransmitBufferEmpty(ArduinoMCP2515 * this_ptr)
{
  /* Serial.println("Transmit Buffer empty"); */
}
