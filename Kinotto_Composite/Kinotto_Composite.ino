//code by bitluni (send me a high five if you like the code)
#include "esp_pm.h"

#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"

#include "luni.h"
#include "font6x8.h"

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 225;
char *KINOTTO[] = {
  " FAMMI UN K______ GIGANTE\n",
  " FAMMI UN KA_____ GIGANTE\n",
  " FAMMI UN K______ GIGANTE\n",
  " FAMMI UN KI_____ GIGANTE\n",
  " FAMMI UN KIN____ GIGANTE\n",
  " FAMMI UN KINO___ GIGANTE\n",
  " FAMMI UN KINOR__ GIGANTE\n",
  " FAMMI UN KINORT_ GIGANTE\n",
  " FAMMI UN KINO___ GIGANTE\n",
  " FAMMI UN KINOT__ GIGANTE\n",
  " FAMMI UN KINOTT_ GIGANTE\n",
  " FAMMI UN KINOTTO GIGANTE\n",
  "  "
};
char* ascii_title[] = {

" \n\n\n __  _  ____  ____    ___   ______  ______   ___  \n",
" |  l/ ]l    j|    \\  /   \\ |      T|      T /   \\ \n",
" |  ' /  |  T |  _  YY    Y|      ||      |Y    Y\n",
" |    \\  |  | |  |  ||  O  |l_j  l_jl_j  l_j|  O  |\n",
" |     Y |  | |  |  ||     |  |  |    |  |  |     |\n",
" |  .  | j  l |  |  |l     !  |  |    |  |  l     !\n",
" l__j\\_j|____jl__j__j \\___/   l__j    l__j   \\___/ \n"
};

char* freakettoni[] = {
"        __ __  __          ________ __       \n",
"       |_ |__)|_  /\\ |_/|_/|_  |  | /  \\|\\ || \n",
"       |  | \\ |__/--\\| \\| \\|__ |  | \\__/| \\|| \n",
"                                        \n",

};

//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
//Composite output using the desired mode (PAL/NTSC) and twice the resolution. 
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

//image and font from the included headers created by the converter. Each iamge uses its own namespace.
Image<CompositeGraphics> luni0(luni::xres, luni::yres, luni::pixels);

//font is based on ASCII starting from char 32 (space), width end height of the monospace characters. 
//All characters are staored in an image vertically. Value 0 is background.
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>
char currentScene = 'C'; 
int paramBg    = 0; // Index 1: 0 = Black BG, 1 = White BG
int paramSize  = 6; // Index 2: 0-9 (Size multiplier)
int paramSpeed = 3; // Index 3: 0-9 (Speed multiplier)
int paramShape = 1; // Index 4: 0 = Cube, 1 = Pyramid
int paramMulti = 1; // Index 5: 1-9 (Grid Multiplier)
// Global variables for animation and serial
int t = millis();
int j = 0;
int scene = 0;
String customSerialText = "WAITING FOR SERIAL...";

void setup()
{
  // 1. Initialize Serial Communication at 115200 baud rate
  Serial.begin(115200);
  Serial.println("ESP32 Composite Video Started!");
  Serial.println("Type 'scene 0' or 'scene 1' to change scenes, or type a custom message.");

  //highest clockspeed needed
  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);
  
  //initializing DMA buffers and I2S
  composite.init();
  //initializing graphics double buffer
  graphics.init();
  //select font
  graphics.setFont(font);

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void handleSerial() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); 

    // CHANGE HERE: Now checking for exactly 6 characters (e.g., "C15304")
    if (input.length() == 6) { 
      
      // 1. Extract the Scene Letter
      currentScene = toupper(input.charAt(0));
      
      // 2. Extract the individual parameter digits
      paramBg    = input.charAt(1) - '0';
      paramSize  = input.charAt(2) - '0';
      paramSpeed = input.charAt(3) - '0';
      paramShape = input.charAt(4) - '0';
      paramMulti = input.charAt(5) - '0';
      
      if (paramMulti < 1) paramMulti = 1; // Safety check
      
      Serial.println("Command parsed:");
      Serial.println(input);
    } else {
      Serial.println("Error: Expected 6 characters (e.g., C06301)");
    }
  }
}
void draw()
{
  // 1. Determine background and foreground colors based on paramBg
  // (Assuming 54 is white and 0 is black in Bitluni's palette)
  int bgColor = (paramBg == 1) ? 54 : 0;
  int fgColor = (paramBg == 1) ? 0 : 54;

  // Clear the screen with the chosen background color
  graphics.begin(bgColor);
  
  switch(currentScene) {
    case 'A': 
      if (paramMulti == 0) {
        // A0000: Whole screen white
        graphics.fillRect(0, 0, XRES, YRES, 54); 
      } 
      else if (paramMulti == 1) {
        // A0001: Whole screen gray
        graphics.fillRect(0, 0, XRES, YRES, 20); 
      }
      break;

    // --- SCENE B: Split Screens ---
    case 'B':
      if (paramMulti == 1) {
        // B0001: Top half black (0), Bottom half white (54)
        graphics.fillRect(0, YRES/2, XRES, YRES/2, 54);
      }
      break;

    // --- SCENE C: 3D WIREFRAMES ---
    case 'C': {
      static float angle = 0; 
      float speed = paramSpeed * 0.005; 
      angle += speed; 
      int actualSize = 20 + (paramSize * 10); 

      // 1. LOOKUP ARRAYS FOR BASE GRID
      // Index matches paramMulti (0 to 9). 
      // M=1(1x1), M=2(2x1), M=3(2x1), M=4(2x2), M=5(2x2), M=6(2x2), M=7(3x2), M=8(3x2), M=9(3x3)
      int gridCols[] = {0, 1, 2, 2, 2, 2, 3, 3, 3, 3};
      int gridRows[] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 3};
      
      int cols = gridCols[paramMulti];
      int rows = gridRows[paramMulti];
      
      // Calculate how many go in the grid, and how many overlap in the center
      int baseItems = cols * rows;
      int remainder = paramMulti - baseItems;

      int cellWidth = XRES / cols;
      int cellHeight = YRES / rows;

      // 2. DRAW THE BASE GRID (The normal layout)
      for (int i = 0; i < baseItems; i++) {
        int col = i % cols;
        int row = i / cols;
        
        int cx = (col * cellWidth) + (cellWidth / 2);
        int cy = (row * cellHeight) + (cellHeight / 2);

        switch(paramShape) {
          case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
        }
      }

      // 3. DRAW THE OVERLAPPING REMAINDER (In the center)
      for (int i = 0; i < remainder; i++) {
        int cx = XRES / 2;
        int cy = YRES / 2;
        
        // If there are exactly 2 remainder items, spread them out!
        if (remainder == 2) {
           // Calculate the offset: Half the shape's size PLUS 20 extra pixels of padding
           int spreadDistance = (actualSize / 2) + 30; 
           
           // Apply the negative offset to the first one, and positive to the second
           cx += (i == 0) ? -spreadDistance : spreadDistance;
        }

        switch(paramShape) {
          case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
        }
      }
      break;
    }
    default:
      graphics.setCursor(10, 10);
      graphics.setTextColor(fgColor, bgColor);
      graphics.print((char*)"UNKNOWN COMMAND");
      break;
  }
  
  graphics.end();
}
void loop()
{
  // Check for serial input every loop
  handleSerial();
  // Draw the graphics
  draw();
}