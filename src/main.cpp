#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <json.h>
#include <esp_camera.h>
#include <base64.h>

const char *ssid = "Galaxy S22";
const char *password = "12345678";
const char *serverAddress = "192.168.70.227";
const int serverPort = 80;
WiFiClient client;
String clientId = "shivesh.kurious@gmail.com";

void sendData(String s);
void sendData(const uint8_t *data, size_t length);
// void sendData(const String &data);
void captureAndSendImage();

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_GPIO_NUM 4

void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(9600);
  Serial.print("\nDefault ESP32 MAC Address: ");
  Serial.println(clientId);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.enableLongRange(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_HVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // config.frame_size = FRAMESIZE_QVGA;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  Serial.print("ipd");
  Serial.println(s->id.PID);
  s->set_brightness(s, 2);                 // -2 to 2
  s->set_contrast(s, 1);                   // -2 to 2
  s->set_saturation(s, 1);                 // -2 to 2
  s->set_special_effect(s, 0);             // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);                   // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);                    // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);              // 0 = disable , 1 = enable
  s->set_aec2(s, 0);                       // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);                   // -2 to 2
  s->set_aec_value(s, 300);                // 0 to 1200
  s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);                   // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
  s->set_bpc(s, 0);                        // 0 = disable , 1 = enable
  s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
  s->set_lenc(s, 1);                       // 0 = disable , 1 = enable
  s->set_hmirror(s, 1);                    // 0 = disable , 1 = enable
  s->set_vflip(s, 1);                      // 0 = disable , 1 = enable
  s->set_dcw(s, 1);                        // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);                   // 0 = disable , 1 = enable
  if (client.connect(serverAddress, serverPort))
  {
    Serial.println("Connected to WebSocket server!");

    char key[] = "dGhlIHNhbXBsZSBub25jZR=="; // Base64-encoded random key (replace with actual random key)

    String request = "GET /?clientId=" + String(clientId) + " HTTP/1.1\r\n"
                                                            "Host: " +
                     String(serverAddress) + "\r\n"
                                             "Upgrade: websocket\r\n"
                                             "Connection: Upgrade\r\n"
                                             "Sec-WebSocket-Key: " +
                     String(key) + "\r\n"
                                   "Sec-WebSocket-Version: 13\r\n"
                                   "User-Agent: ESP01\r\n"
                                   "\r\n";

    client.print(request);

    while (client.connected())
    {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
        break;
      }
    }
    sendData("hello");
    String response = client.readString();
    Serial.println("Server Response: " + response);
  }
  else
  {
    Serial.println("Connection failed");
  }
  pinMode(4, OUTPUT);
  digitalWrite(4, 1);
}

void loop()
{
  while (!client.connected())
  {
    Serial.print("s");
    delay(!00);
  }
  captureAndSendImage(); // Function to capture and send image

  // uint64_t m = millis();
  // while (millis() - m < 10)
  // {
  //   String s;
  //   while (client.available())
  //   {
  //     uint8_t buffer[256];
  //     size_t bytesRead = client.readBytes(buffer, sizeof(buffer));
  //     for (size_t i = 0; i < bytesRead; i++)
  //     {
  //       if (i > 1)
  //         s += char(buffer[i]);
  //       Serial.println(char(buffer[i]));
  //     }
  //   }
  // }
}
void sendData(String data)
{
  if (data.length() == 0)
  {
    Serial.println("No data to send");
    return;
  }

  uint8_t opcode = 0x81; // FIN + opcode for text
  uint8_t mask = 0x80;   // MASK bit
  uint64_t payloadLength = data.length();

  // Create a buffer for WebSocket frame
  size_t frameSize = 1 + 1 + (payloadLength <= 125 ? 0 : (payloadLength <= 65535 ? 2 : 8)) + 4 + payloadLength;
  uint8_t *frame = new uint8_t[frameSize];

  size_t index = 0;

  // Write the first byte (FIN + opcode)
  frame[index++] = opcode;

  // Write the second byte (MASK + payload length)
  if (payloadLength <= 125)
  {
    frame[index++] = mask | payloadLength;
  }
  else if (payloadLength <= 65535)
  {
    frame[index++] = mask | 126;
    frame[index++] = (payloadLength >> 8) & 0xFF;
    frame[index++] = payloadLength & 0xFF;
  }
  else
  {
    frame[index++] = mask | 127;
    for (int i = 7; i >= 0; i--)
    {
      frame[index++] = (payloadLength >> (8 * i)) & 0xFF;
    }
  }

  // Write the masking key
  uint8_t maskingKey[4];
  for (int i = 0; i < 4; i++)
  {
    maskingKey[i] = random(0, 256);
  }
  memcpy(frame + index, maskingKey, 4);
  index += 4;

  // Apply masking and copy payload data
  for (size_t i = 0; i < payloadLength; i++)
  {
    frame[index++] = data[i] ^ maskingKey[i % 4];
  }

  // Send the entire frame in one go
  client.write(frame, frameSize);

  // Clean up
  delete[] frame;
}

// void sendData(String data)
// {
//   // String message = s;
//   Serial.println("E");
//   uint8_t opcode = 0x81; // FIN + opcode for text
//   uint8_t mask = 0x80;   // MASK bit
//   uint64_t payloadLength = data.length();

//   client.write(opcode);
//   Serial.println("F");
//   if (payloadLength <= 125)
//   {
//     client.write(mask | payloadLength);
//   }
//   else if (payloadLength <= 65535)
//   {
//     client.write(mask | 126);
//     client.write((uint8_t)(payloadLength >> 8));
//     client.write((uint8_t)payloadLength);
//   }
//   else
//   {
//     client.write(mask | 127);
//     for (int i = 7; i >= 0; i--)
//     {
//       client.write((uint8_t)(payloadLength >> (8 * i)));
//     }
//   }
//   Serial.println("G");
//   uint8_t maskingKey[4];
//   for (int i = 0; i < 4; i++)
//   {
//     maskingKey[i] = random(0, 256);
//   }
//   Serial.println("H");
//   client.write(maskingKey, 4);
//   Serial.println("I");
//   Serial.println(payloadLength);
//   for (size_t i = 0; i < payloadLength; i++)
//   {
//     client.write(data[i] ^ maskingKey[i % 4]);
//   }

//   Serial.println("J");
// }

void captureAndSendImage()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.println("A");
  // String image = "";
  // for (uint64_t i = 0; i < fb->len; i++)
  // {
  //   image += (String)fb->buf[i];
  //   image += ' ';
  // }
  Serial.println(fb->len);
  // Serial.println(image);
  // sendImage(fb->buf, );
  String base64Image = "data:image/jpeg;base64,";
  base64Image += base64::encode(fb->buf, fb->len);
  sendData(base64Image);
  // sendData(fb->buf,fb->len);
  Serial.println("C");
  esp_camera_fb_return(fb);
}
