#include <Thread.h>
#include "HX711.h"
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>   // Hardware-specific library
#include <SoftwareSerial.h>
#include <StringSplitter.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

#include <FreeDefaultFonts.h>

//--------------------Wi-Fi--------------------
String ACCOUNT_h = "YU"; 
String PASSWORD_h = "0978703705";
String ACCOUNT_s = "Redmi"; 
String PASSWORD_s = "16881688";
String MODE = "1"; //當Server
String POST = "10001";
SoftwareSerial wifi(A13,A14); //TX:橘 , RX:黃 
//---------------------------------------------
  
//=============Thread==================
Thread wifiThread = Thread();
Thread hx711Thread = Thread();

//==============hx711=================
HX711 cell(49,47);

//===============tft===================
MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.


//------------------------------paint-----------------------------
const char bmp_header[54]=
{
     0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 
     0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 

char *name = "Please Calibrate.";  //edit name of shield
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
const int TS_LEFT=907,TS_RT=136,TS_TOP=942,TS_BOT=139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 500

int16_t BOXSIZE;  // 顏色選單大小
int16_t PENRADIUS = 6; //筆的粗細
uint16_t ID, oldcolor, currentcolor;

//顏色
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

boolean draw = false;
uint16_t lastxpos, lastypos;  //螢幕座標
//---------------------------show sd card img------------------------

#if defined(ESP32)
#define SD_CS     5
#else
#define SD_CS     10
#endif
#define NAMEMATCH ""        // "" matches any name
#define PALETTEDEPTH   0     // do not support Palette modes

bool good;
//==============btn====================
const int buttonPin1 = 35;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin
const int buttonPin2 = 33;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin
const int buttonPin3 = 31;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin
const int buttonPin4 = 39;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin
const int buttonPin5 = 41;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin
const int buttonPin6 = 43;     // the number of the pushbutton pinconst int buttonPin = 2;     // the number of the pushbutton pin

//=============sys param===================
bool flag = true;//T:btn can click F:btn cannot click
int mode = 0;//0:show 1:wifi 2:draw 3:msg
int page = 1;
int select = 0;
File root;
int pathlen;
char filenm;
String lastfilename = "";
bool wificon = false;
String data1 = "TS#1#10110#6#123456789012345678901234567890123!#123456789012345678901234567890123!#123456789012345678901234567890123!#123456789012345678901234567890123!#123456789012345678901234567890123!";

String data = "TS#1#10110#5#123456#123456#123456#123456#123456";
String subject[] = { "M", "A", "U", "C", "L", "P", "S", "R", "G", "B" };
//=========================================
//main
void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);

 setshowimg();
 String imgname = "/TShome/1.bmp";
 //getclass();
 delay(10000);
 printimg(imgname.c_str());
 Serial.println("home");
 imgname = "/TShome/12.bmp";
 printimg(imgname.c_str());
 Serial.println("home2");
 setbtn();
 setwifithread();
}

void loop() {
  // put your main code here, to run repeatedly:
    
    // checks if thread should run

  if(wificon){

    if(wifiThread.shouldRun()){
      wifiThread.run(); 
    }     
  
  }

  
  btnselect();
  
  Serial.println(millis());

  
}
//===========================================

//=================================================
//btn

//-------------------------------------------
//設置
void setbtn(){
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);
  pinMode(buttonPin5, INPUT);
  pinMode(buttonPin6, INPUT);
}
//-------------------------------------------
//判斷
void btnselect(){
  
  select=0;
  
  int btn1 = digitalRead(buttonPin1);
  int btn2 = digitalRead(buttonPin2);
  int btn3 = digitalRead(buttonPin3); 
  int btn4 = digitalRead(buttonPin4);
  int btn5 = digitalRead(buttonPin5);
  int btn6 = digitalRead(buttonPin6);

  if(btn1 == 1){select = 1;}
  else if(btn2 == 1){select = 2;}
  else if(btn3 == 1){select = 3;}
  else if(btn4 == 1){select = 4;}
  else if(btn5 == 1){select = 5;}
  else if(btn6 == 1){select = 6;}
  else{Serial.println("wait...");}
  Serial.println("select:"+String(select));


  String imgname1 = "/TShome/12.bmp";
  String imgname2 = "/wifiok/choose.bmp";
  String imgname3 = "/msg/s/0.bmp";
  
  Serial.println(mode);
  if(mode == 3){//心情留言
    
    String m3name1 = "/msg/s/1.bmp";//喜
    String m3name2 = "/msg/s/2.bmp";//怒
    String m3name3 = "/msg/s/3.bmp"; //哀
    String m3name4 = "/msg/s/4.bmp";//樂 
    String m3name5 = "/msg/s/5.bmp"; //不舒服


    tft.setRotation(1);
    Serial.println(flag);
      if(!flag){//不能按
        Serial.println("按鈕執行中");
      }else{//能按
        
        switch(select){
          
          case 1: flag=false; mode=0; page=1; printimg(imgname1.c_str()); Serial.println("show"); flag=true; break;
          case 2: flag=false; SD.remove("msgpg.txt"); writedata("msgpg.txt","1"); printimg(m3name1.c_str()); Serial.println("1"); flag=true; break;
          case 3: flag=false; SD.remove("msgpg.txt"); writedata("msgpg.txt","2"); printimg(m3name2.c_str()); Serial.println("2"); flag=true; break;
          case 4: flag=false; SD.remove("msgpg.txt"); writedata("msgpg.txt","3"); printimg(m3name3.c_str()); Serial.println("3"); flag=true; break;
          case 5: flag=false; SD.remove("msgpg.txt"); writedata("msgpg.txt","4"); printimg(m3name4.c_str()); Serial.println("4"); flag=true; break;
          case 6: flag=false; SD.remove("msgpg.txt"); writedata("msgpg.txt","5"); printimg(m3name5.c_str()); Serial.println("5"); flag=true; break;
          default:Serial.println("no");   
        
        }
      }
    
  }else if(mode == 2){//畫畫
    
    Serial.println(flag);
    
    paintdraw();
    
    if(!flag){//不能按
      Serial.println("按鈕執行中");
    }else{//能按
      switch(select){
          
          case 1: tft.setRotation(1);flag=false; mode=0; page=1; printimg(imgname1.c_str()); Serial.println("show"); flag=true; break;
          case 4: flag=false; setpaint(); Serial.println("clear"); flag=true; break;
          case 5: flag=false; setsaveimg(); delay(5000); setpaint(); Serial.println("save ok"); flag=true; break;      
          default:Serial.println("no");
             
      }

    }
    
  }else if(mode == 1){//wifi
    
    tft.setRotation(1);
    Serial.println(flag);
    if(!flag){//不能按
      Serial.println("按鈕執行中");
    }else{//能按
      switch(select){
          
          case 1: flag=false; mode=0; page=1; printimg(imgname1.c_str()); Serial.println("show"); flag=true; break;
          case 3: flag=false; last(); Serial.println("last"); flag=true; break; 
          case 2: flag=false; next(); Serial.println("next"); flag=true; break; 
          default:Serial.println("no");
             
      }
    }
  }else if(mode == 0){//顯示


    tft.setRotation(1);
    Serial.println(flag);
      if(!flag){//不能按
        Serial.println("按鈕執行中");
      }else{//能按
        switch(select){

          case 1: flag=false; mode=0; page=1; printimg(imgname1.c_str()); Serial.println("show"); flag=true; break;
          case 2: flag=false; mode=1; printimg(imgname2.c_str()); Serial.println("wifi"); flag=true; break;
          case 3: flag=false; last(); Serial.println("last"); flag=true; break;
          case 4: flag=false; mode=2; setpaint(); Serial.println("draw"); lastfilename = "draw"; flag=true; break;
          case 5: flag=false; mode=3; printimg(imgname3.c_str()); Serial.println("msg"); flag=true; break;
          case 6: flag=false; next(); Serial.println("next"); flag=true; break;      
          default:Serial.println("no");   
        
        }
      }
     
  }
 
  
//  delay(1000);
}
//------------------------------------------
//上一頁/家
void last(){

  String imgname1 = "/TShome/12.bmp";//吉祥物 
  String imgname2 = "/HW/class.bmp";//課表
  String imgname3 = "/HW/hw.bmp";//作業
  String imgname4 = "/msg/h/"+readtxt("msgpg.txt")+".bmp"; //心情留言
  String imgname5 = "/butter"+getgrade()+"/"+getlevel()+".bmp";//電子寵物 
  String imgname6 = "/ID/SAVEOK1.bmp"; //個性圖案
  
  
  
  if(mode == 0){//show
    tft.setRotation(1);
    
    page--;
    if(page < 1){page=1;Serial.println(page);return;}

    switch(page){
      case 1: printimg(imgname1.c_str()); Serial.println("img1"); break;
      case 2: getclass(); Serial.println("img2"); break;
      case 3: getHW(); Serial.println("img3"); break;
      case 4: printimg(imgname4.c_str()); Serial.println("img4"); break;
      case 5: printimg(imgname5.c_str()); Serial.println("img5"); break;
      case 6: printimg(imgname6.c_str()); Serial.println("img6"); break;
      default:Serial.println("no");
      }
  }else if(mode == 1){//hwifi
    
    Serial.println("home");
    
    if(wificon == false){
      printimg("/wifiok/H.bmp");
      wificon=true;
    }else{
      printimg("/wifiok/2HF.bmp");
      wificon=false;
    }
    
    page=1;
    printimg("/TShome/12.bmp");
    mode=0;
  }
  
}
//---------------------------------------------
//下一頁/學校
void next(){
  
  String imgname1 = "/TShome/12.bmp";//吉祥物 
  String imgname2 = "/HW/class.bmp";//課表
  String imgname3 = "/HW/hw.bmp";//作業
  String imgname4 = "/msg/h/"+readtxt("msgpg.txt")+".bmp"; //心情留言
  String imgname5 = "/butter"+getgrade()+"/"+getlevel()+".bmp";//電子寵物 
  String imgname6 = "/ID/SAVEOK1.bmp"; //個性圖案
  
  if(mode == 0){//show
    tft.setRotation(1);
    
    page++;
    if(page > 6){page=6;Serial.println(page);return;}
 
    switch(page){
      case 1: printimg(imgname1.c_str()); Serial.println("img1"); break;
      case 2: getclass(); Serial.println("img2"); break;
      case 3: getHW(); Serial.println("img3"); break;
      case 4: Serial.println("1:"+imgname4); printimg(imgname4.c_str()); Serial.println("2:"+imgname4); Serial.println("img4"); break;
      case 5: printimg(imgname5.c_str()); Serial.println("img5"); break;
      case 6: printimg(imgname6.c_str()); Serial.println("img6"); break;
      default:Serial.println("no");
      }
  }else if(mode == 1){//swifi
    Serial.println("school");
    
    if(wificon == false){
      printimg("/wifiok/S.bmp");
      wificon=true;
    }else{
      printimg("/wifiok/2SF.bmp");
      wificon=false;
    }
    
    page=1;
    printimg("/TShome/12.bmp");
    mode=0;
  }
}
//=====================================================

//=====================================================
//wifi thread

//----------------------------------------------------
//設置
void setwifithread(){
  setupWiFi();
  wifiThread.onRun(connectinternet);
  wifiThread.setInterval(500);
  
}

//----------------------------------------------------
//連線
void connectinternet(){

  String str = "TS#1#10110#2#"+hx711()+"#4#"+readtxt("msgpg.txt");
  Serial.println(str);
  sendData(str); 
  while (wifi.available()){
    Wi_Fi();
  }  
  
}

//-------------------------------------------------------------------------------
//設定wifi
void setupWiFi(){
  Serial.begin(9600);
  wifi.begin(9600);
//
//  wifiCommand("AT+RST" , 500);
//  wifiCommand("AT+CWMODE=3" , 500);
//  wifiCommand("AT+CWJAP=\"" + ACCOUNT + "\",\"" + PASSWORD + "\"" , 5000);  
  wifiCommand("AT+CIFSR" , 500);
  wifiCommand("AT+CIPMUX=" + MODE , 500); 
  wifiCommand("AT+CIPSERVER=" + MODE + "," + POST , 500); 

  Serial.println("====================ESP8266 Wi-Fi Started====================");
}

//---------------------------------------------------------------------
//wifi指令
void wifiCommand(String command, const int timeout) {
  String response="";
  command = command + "\r\n";
  wifi.print(command); 
  long int time=millis();
  
  while ((time+timeout) > millis()) {
    while(wifi.available()) {
      char c=wifi.read(); 
      response += c;
    } 
  }
  Serial.print(F("response is ")); Serial.println(response);
} 

//----------------------------------------------------------------------
//接收
void Wi_Fi(){
  char val;
  String msg1 = "";
  int msgLen = 0;
  int block=0;
  
  while((val=wifi.read()) != '\n'){
    msg1 += val;
    delay(20);
  }

  msg1.trim(); 
  Serial.println("封包內容="+msg1);
  

  int firstSubstring = msg1.indexOf(':');
  if(firstSubstring < 0){
    return;
  }

  msg1 = msg1.substring(firstSubstring+1);
  msgLen = msg1.length(); 
  Serial.println("封包內容="+msg1);
  if(msg1.substring(0,2) == "TS"){
    StringSplitter *sysp = new StringSplitter(msg1, '#', 5);
    String cmd = sysp->getItemAtIndex(3);
    if(cmd == "5"){
        writedata("/subject.txt",msg1);//課表
    }
	else if (cmd == "6")
	{
		writedata("/HW.txt", msg1);//作業
	}
	else
	{
        writedata("/butpg.txt",msg1);//TS#0#10110#3#年級區間#第幾階段  
    }
    
  }

  


  Serial.println("Receive OK!!!\n--------------------");
}


//--------------------------------------------------------------
//傳送資料
void sendData(String str){
  Serial.println(str);
  String cipSend = "AT+CIPSEND=0,";
  cipSend += str.length()+2;
  wifiCommand(cipSend , 20);
  wifiCommand(str , 20);
}


//================================================================


//=====================================================
//hx711

//hx711
String hx711(){
long val=0;
String val2 = "";
val=cell.read();
val2=((((val+102750)/477.00f)/100)-0.1);
  
return val2;
}
//==============================================================
//SD card

//--------------------------------------------------------------
//年級
String getgrade(){
//  TS#0#10110#3#1#3

  StringSplitter *sysp = new StringSplitter(readtxt("butpg.txt"), '#', 4);
  String syss = sysp->getItemAtIndex(3);
  Serial.println(syss);
  StringSplitter *fp = new StringSplitter(syss, '#', 3);
  String grade = fp->getItemAtIndex(1);
  Serial.println(grade);
  return grade;
}

//--------------------------------------------------------------
//階級
String getlevel(){
  StringSplitter *sysp = new StringSplitter(readtxt("butpg.txt"), '#', 4);
  String syss = sysp->getItemAtIndex(3);
  Serial.println(syss);
  StringSplitter *fp = new StringSplitter(syss, '#', 3);
  String lv = fp->getItemAtIndex(2);
  Serial.println(lv);
  return lv;
}

//--------------------------------------------------------------
//讀txt
String readtxt(String txtfile){

 String rec="";
 char c; 
 File readFile;
 readFile = SD.open(txtfile.c_str());
 
 if (! readFile){Serial.println("no"); return;}
  Serial.println(readFile);
     // read from the file until there's nothing else in it:
  while (readFile.available()) {
    c=readFile.read();
    rec=rec+String(c);
  }

    // close the file:
  readFile.close();
  Serial.println(rec);
  Serial.println("ok");
  return rec;
}

//--------------------------------------------------------------
//寫資料到txt
void writedata(String filen,String data){
    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  SD.remove(filen.c_str());
File myFile = SD.open(filen.c_str(), FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.print(data);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
//====================================================

//=================================================
//截圖

//-----------------------------------------------
//loadScreendata
void GrabImage(char* str)
{
  Serial.println("d");
 File outFile;
 byte VH,VL;
 int i,j = 0;
 Serial.println("d1");
 //Create the File
 outFile = SD.open(str,FILE_WRITE);
 if (! outFile) return;
Serial.println("d2");
 //Write the BMP header
 for( i = 0; i < 54; i++)
 {
   char ch = bmp_header[i];
   outFile.write((uint8_t*)&ch,1);
 }
 
  
 

Serial.println("d3");
 for(i = 0; i < 240; i++){
   for(j = 0; j < 320; j++){
     unsigned short rgb = tft.readPixel(i,j);
     VH = (rgb & 0xFF00) >> 8; //Mask the High Byte
     VL = rgb & 0x00FF; //Mask the Low Byte
     //RGB565 to RGB555 Conversion
     VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
     VH = VH >> 1;
     //Write image data to file, low byte first
     outFile.write(VL); 
     outFile.write(VH);
     tft.drawPixel(i,j,BLACK);
 }
 }
 Serial.println("d5");
 //Close the file  
 outFile.close(); 
 Serial.println("ok");
 return;
}


//-----------------------------------------------
//save image

void setsaveimg(){
Serial.println("go1");
 Serial.println("go2");
 char str[] = "/ID/saveok1.BMP";
 SD.remove(str);
 Serial.println("go3");
 BOXSIZE = tft.width() / 6;
 tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
  delay(500);

 
 GrabImage(str);
}
//================================================

//================================================
//畫畫

//------------------------------------------------
//setpaint
void setpaint(){
      //初始化
    uint16_t tmp;

    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    Serial.begin(9600);
    
    tft.setRotation(0);
    tft.fillScreen(WHITE);


    BOXSIZE = tft.width() / 6;

    //顯示顏色選單
    tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
    tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
    tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
    tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
    tft.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
    tft.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);

    tft.drawRect(0, 0, BOXSIZE, BOXSIZE, BLACK);
    currentcolor = RED;
    delay(1000);

}

//--------------------------------------------
//paint
void paintdraw(){
      uint16_t xpos, ypos;  //螢幕座標
    tp = ts.getPoint();   //tp.x, tp.y 是ＡＤＣ值

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {

        xpos = map(tp.y,TS_BOT, TS_TOP, 0, tft.width());
        ypos = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());


        //偵測我們選的顏色區域
        if (ypos < BOXSIZE) {//在選擇的區域畫白色選擇匡
            oldcolor = currentcolor;

            if (xpos < BOXSIZE) {
                currentcolor = RED;
                tft.drawRect(0, 0, BOXSIZE, BOXSIZE, BLACK);
            } else if (xpos < BOXSIZE * 2) {
                currentcolor = YELLOW;
                tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, BLACK);
            } else if (xpos < BOXSIZE * 3) {
                currentcolor = GREEN;
                tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, BLACK);
            } else if (xpos < BOXSIZE * 4) {
                currentcolor = CYAN;
                tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, BLACK);
            } else if (xpos < BOXSIZE * 5) {
                currentcolor = BLUE;
                tft.drawRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLACK);
            } else if (xpos < BOXSIZE * 6) {
                currentcolor = MAGENTA;
                tft.drawRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, BLACK);
            }

            if (oldcolor != currentcolor) { //清除舊顏色的白匡
                if (oldcolor == RED) tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
                if (oldcolor == YELLOW) tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
                if (oldcolor == GREEN) tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
                if (oldcolor == CYAN) tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
                if (oldcolor == BLUE) tft.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
                if (oldcolor == MAGENTA) tft.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);
            }
        }
        
        // 判斷是不是在畫圖區
        if (((ypos - PENRADIUS) > BOXSIZE) && ((ypos + PENRADIUS) < tft.height())) {
          
            tft.fillCircle(xpos, ypos, PENRADIUS, currentcolor);

        }
        

    }
}

//=========================================================

//=========================================================
//tft

//-------------------------------------------------------
//set SD card

void setshowimg(){
  
    uint16_t ID;
    Serial.begin(9600);
    Serial.print("Show BMP files on TFT with ID:0x");
    ID = tft.readID();
    Serial.println(ID, HEX);
    if (ID == 0x0D3D3) ID = 0x9481;
    tft.begin(ID);
    tft.setRotation(1);
    tft.fillScreen(BLACK);
    tft.setTextColor(0xFFFF, 0x0000);
    bool good = SD.begin(SD_CS);
    if (!good) {
        Serial.print(F("cannot start SD"));
        while (1);
    }

}
//-------------------------------------------------------
//printimg
void printimg(char namebuf[]){
  Serial.println(namebuf);
    uint8_t ret;
    uint32_t start;
    int cmp = strcmp (namebuf,lastfilename.c_str());
    Serial.println(String(cmp));
    
        if (cmp != 0 && strstr(namebuf, ".bmp") != NULL && strstr(namebuf, NAMEMATCH) != NULL) {
            Serial.println(namebuf);
            Serial.print(F(" - "));
            tft.fillScreen(0);
            start = millis();
            ret = showBMP(namebuf, 0, 0);
            lastfilename = namebuf;
            switch (ret) {
                case 0:
                    Serial.print(millis() - start);
                    Serial.println(F("ms"));
                    delay(3000);
                    break;
                case 1:
                    Serial.println(F("bad position"));
                    break;
                case 2:
                    Serial.println(F("bad BMP ID"));
                    break;
                case 3:
                    Serial.println(F("wrong number of planes"));
                    break;
                case 4:
                    Serial.println(F("unsupported BMP format"));
                    break;
                case 5:
                    Serial.println(F("unsupported palette"));
                    break;
                default:
                    Serial.println(F("unknown"));
                    break;
            }
        }
}

//----------------------------------------------------------
//read img

#define BMPIMAGEOFFSET 54

#define BUFFPIXEL      20

uint16_t read16(File& f) {
    uint16_t result;         // read little-endian
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;      //

    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg)
{
	int16_t x1, y1;
	uint16_t wid, ht;
	//.drawFastHLine(0, y, tft.width(), WHITE);
	tft.setFont(f);
	tft.setCursor(x, y);
	tft.setTextColor(BLACK);
	tft.setTextSize(sz);
	tft.print(msg);
}

#pragma region 課表

//--------------------------------------------------------------
//���Ҫ�
///param=dataŪsubject.txt���
///
void getclass() {

	String bg = "/TShome/5566.bmp";
	printimg(bg.c_str());

	StringSplitter *sysp = new StringSplitter(readtxt("subject.txt"), '#', 5);
	String week = sysp->getItemAtIndex(4);

	StringSplitter *sysweek = new StringSplitter(week, '#', 6);
	String day1 = sysweek->getItemAtIndex(0);
	String day2 = sysweek->getItemAtIndex(1);
	String day3 = sysweek->getItemAtIndex(2);
	String day4 = sysweek->getItemAtIndex(3);
	String day5 = sysweek->getItemAtIndex(4);

	setclasstitle();

	getdayclass(day1, 1);
	getdayclass(day2, 2);
	getdayclass(day3, 3);
	getdayclass(day4, 4);
	getdayclass(day5, 5);

}

//--------------------------------------------------------------
//���Ҫ���D
///param=dataŪ�Ҫ�TXT���

void setclasstitle() {
	showmsgXY(80 + 25 * 1, 0, 3, NULL, "1");
	showmsgXY(80 + 25 * 2, 0, 3, NULL, "2");
	showmsgXY(80 + 25 * 3, 0, 3, NULL, "3");
	showmsgXY(80 + 25 * 4, 0, 3, NULL, "4");
	showmsgXY(80 + 25 * 5, 0, 3, NULL, "5");
}

//--------------------------------------------------------------
//���C�Ѫ��Ҫ�
///param=data

void getdayclass(String day, int x) {
  
	String sb1 = day.substring(0, 1);
	showmsgXY(80 + 25 * x, 34 * 1, 3, NULL, subject[sb1.toInt()].c_str());
	String sb2 = day.substring(1, 2);
	showmsgXY(80 + 25 * x, 34 * 2, 3, NULL, subject[sb2.toInt()].c_str());
	String sb3 = day.substring(2, 3);
	showmsgXY(80 + 25 * x, 34 * 3, 3, NULL, subject[sb3.toInt()].c_str());
	String sb4 = day.substring(3, 4);
	showmsgXY(80 + 25 * x, 34 * 4, 3, NULL, subject[sb4.toInt()].c_str());
	String sb5 = day.substring(4, 5);
	showmsgXY(80 + 25 * x, 34 * 5, 3, NULL, subject[sb5.toInt()].c_str());
	String sb6 = day.substring(5, 6);
	showmsgXY(80 + 25 * x, 34 * 6, 3, NULL, subject[sb6.toInt()].c_str());
}
#pragma endregion
#pragma region �p��ï
//--------------------------------------------------------------
//�p��ï
///param=dataŪsubject.txt���
///
void getHW() {
	Serial.println(data1);
  String bg = "/TShome/6666.bmp";
  printimg(bg.c_str());
	StringSplitter *sysp = new StringSplitter(readtxt("HW.txt"), '#', 5);
	String dataHW = sysp->getItemAtIndex(4);
	//showmsgXY(0, 200, 3, NULL, dataHW.c_str());

	StringSplitter *sysHW = new StringSplitter(dataHW, '#', 5);

	String dataHW1 = sysHW->getItemAtIndex(0);
	String dataHW2 = sysHW->getItemAtIndex(1);
	String dataHW3 = sysHW->getItemAtIndex(2);
	String dataHW4 = sysHW->getItemAtIndex(3);
	String dataHW5 = sysHW->getItemAtIndex(4);
	/*
		showmsgXY(80, 100, 2, NULL, dataHW1.c_str());*/

	int count = 0;

	if (dataHW1.length() > 17) {
		showmsgXY(0, 22 * count, 3, NULL, dataHW1.substring(0, 17).c_str());
		count++;
		showmsgXY(0, 22 * count, 3, NULL, dataHW1.substring(17, 34).c_str());
		count++;
	}
	else if (dataHW1 != "*" && dataHW1.length() > 0 && dataHW1.length() < 17)
	{
		showmsgXY(0, 22 * count, 3, NULL, dataHW1.c_str());
		count++;
	}

	if (dataHW2.length() > 17) {
		showmsgXY(0, 22 * count, 3, NULL, dataHW2.substring(0, 17).c_str());
		count++;
		showmsgXY(0, 22 * count, 3, NULL, dataHW2.substring(17, 34).c_str());
		count++;
	}
	else if (dataHW2 != "*" && dataHW2.length() > 0 && dataHW2.length() < 17)
	{
		showmsgXY(0, 22 * count, 3, NULL, dataHW2.c_str());
		count++;
	}

	if (dataHW3.length() > 17) {
		showmsgXY(0, 22 * count, 3, NULL, dataHW3.substring(0, 17).c_str());
		count++;
		showmsgXY(0, 22 * count, 3, NULL, dataHW3.substring(17, 34).c_str());
		count++;
	}
	else if (dataHW3 != "*" && dataHW3.length() > 0 && dataHW3.length() < 17)
	{
		showmsgXY(0, 22 * count, 3, NULL, dataHW3.c_str());
		count++;
	}

	if (dataHW4.length() > 17) {
		showmsgXY(0, 22 * count, 3, NULL, dataHW4.substring(0, 17).c_str());
		count++;
		showmsgXY(0, 22 * count, 3, NULL, dataHW4.substring(17, 34).c_str());
		count++;
	}
	else if (dataHW4 != "*" && dataHW4.length() > 0 && dataHW4.length() < 17)
	{
		showmsgXY(0, 22 * count, 3, NULL, dataHW4.c_str());
		count++;
	}

	if (dataHW5.length() > 17) {
		showmsgXY(0, 22 * count, 3, NULL, dataHW5.substring(0, 17).c_str());
		count++;
		showmsgXY(0, 22 * count, 3, NULL, dataHW5.substring(17, 34).c_str());
		count++;
	}
	else if (dataHW5 != "*" && dataHW5.length() > 0 && dataHW5.length() < 17)
	{
		showmsgXY(0, 22 * count, 3, NULL, dataHW5.c_str());
		count++;
	}
}

#pragma endregion
