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
const int YRES = 200;
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
" |  ' /  |  T |  _  YY     Y|      ||      |Y     Y\n",
" |    \\  |  | |  |  ||  O  |l_j  l_jl_j  l_j|  O  |\n",
" |     Y |  | |  |  ||     |  |  |    |  |  |     |\n",
" |  .  | j  l |  |  |l     !  |  |    |  |  l     !\n",
" l__j\\_j|____jl__j__j \\___/   l__j    l__j   \\___/ \n"
};

char* freakettoni[] = {
"        __ __  __           ________ __       \n",
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

void setup()
{
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
  //rendering the actual graphics in the main loop is done on the second core by default

}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}


int t = millis();
int j = 0;
int scene = 0;

void draw()
{
  //clearing background and starting to draw
  graphics.begin(0);
  //drawing an image
  // luni0.draw(graphics, 200, 10);

  // //drawing a frame
  // graphics.fillRect(27, 18, 160, 30, 10);
  // graphics.rect(27, 18, 160, 30, 20);

  //setting text color, transparent background
  graphics.setTextColor(100);
  //text starting position
  graphics.setCursor(1, 1);
  //printing some lines of text
  // graphics.print("hello!");
  // graphics.print(" free memory: ");
  // graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  // graphics.print("\nrendered frame: ");
  // static int frame = 0;
  // graphics.print(frame, 10, 4); //base 10, 6 characters 
  // graphics.print("\n        in hex: ");
  // graphics.print(frame, 16, 4);
  // frame++;

  // //drawing some lines
  // for(int i = 0; i <= 100; i++)
  // {
  //   graphics.line(50, i + 60, 50 + i, 160, i / 2);
  //   graphics.line(150, 160 - i, 50 + i, 60, i / 2);
  // }
  // if(millis()%(200*12*2)<= 200*12){
  if(scene == 0){

    if(j<13){
      if ((millis()-t)>=250){
        t = millis();
        j++;
      }
      for (int i = 0; i<j; i++){
        graphics.printBig(KINOTTO[i]);
      }
    }

    if(j>=13){
      j = 0;
      scene = 1;
    }
  }

  if(scene == 1){
    for(int i= 0; i<7; i++){
      graphics.print(ascii_title[i]);
    }
    graphics.setCursor(10, 45);
    graphics.printBig("       IL BAR DEI\n   ");
    graphics.setCursor(10, 105);
        for(int i= 0; i<3; i++){
    
      graphics.print(freakettoni[i]);
    }
    if(millis()-t >= 3000){
      t = millis();
      scene = 0;
    }

  }
  // //draw single pixel
  // graphics.dot(20, 190, 10);
  
  //finished drawing, swap back and front buffer to display it
  graphics.end();
}

void loop()
{
  draw();
}
