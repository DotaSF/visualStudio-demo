#include <PS2X_lib.h>  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        13  //14    
#define PS2_CMD        11  //15
#define PS2_SEL        10  //16
#define PS2_CLK        12  //17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
#define pressures   true
//#define pressures   false
#define rumble      true
//#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//现在，库不支持热可插拔控制器，这意味着您必须在连接控制器后始终重新启动arduino，或者在连接控制器后再次调用配置_gamepad（pins）。

int error = 0;
byte type = 0;
byte vibrate = 0;

// Reset func 
void (* resetFunc) (void) = 0;

void setup(){
 
  Serial.begin(115200);
  
  delay(500);  //在配置前增加延迟，使无线PS2模块有一段时间可以启动
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0){
    Serial.print("找到控制器，配置成功！");
    Serial.print("pressures = ");
	if (pressures)
	  Serial.println("true ");
	else
	  Serial.println("false");
	Serial.print("rumble = ");
	if (rumble)
	  Serial.println("true)");
	else
	  Serial.println("false");
    Serial.println("试一下所有的按钮，x会震动控制器，当你用力按的更快；");
    Serial.println("持有l1或r1将打印出模拟棒值。");
    Serial.println("注意：去www.billporter.info更新和报告错误。");
  }  
  else if(error == 1)
    Serial.println("找不到控制器，请检查线路，请参阅。访问www.billporter.info以获取故障排除提示");
   
  else if(error == 2)
    Serial.println("找到控制器，但不接受命令。请阅读我的txt以启用调试。访问www.billporter.info以获取故障排除提示");

  else if(error == 3)
    Serial.println("控制器拒绝进入压力模式，可能不支持。");
  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.println("找到未知控制器类型");
      break;
    case 1:
      Serial.println("DualShock Controller found ");
      break;
    case 2:
      Serial.println("GuitarHero Controller found ");
      break;
	case 3:
      Serial.println("Wireless Sony DualShock Controller found ");
      break;
   }
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
   if(Serial.available())
   Serial.println("0000");
  if(error == 1){ //skip loop if no controller found
    resetFunc();
  }
  
  if(type == 2){ //Guitar Hero Controller
    ps2x.read_gamepad();          //read controller 
   
    if(ps2x.ButtonPressed(GREEN_FRET))
      Serial.println("Green Fret Pressed");
    if(ps2x.ButtonPressed(RED_FRET))
      Serial.println("Red Fret Pressed");
    if(ps2x.ButtonPressed(YELLOW_FRET))
      Serial.println("Yellow Fret Pressed");
    if(ps2x.ButtonPressed(BLUE_FRET))
      Serial.println("Blue Fret Pressed");
    if(ps2x.ButtonPressed(ORANGE_FRET))
      Serial.println("Orange Fret Pressed"); 

    if(ps2x.ButtonPressed(STAR_POWER))
      Serial.println("Star Power Command");
    
    if(ps2x.Button(UP_STRUM))          //只要按下按钮就会是真的
      Serial.println("Up Strum");
    if(ps2x.Button(DOWN_STRUM))
      Serial.println("DOWN Strum");
 
    if(ps2x.Button(PSB_START))         //只要按下按钮就会是真的
      Serial.println("Start is being held");
    if(ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");
    
    if(ps2x.Button(ORANGE_FRET)) {     // 如果为真，则打印棒值
      Serial.print("Wammy Bar Position:");
      Serial.println(ps2x.Analog(WHAMMY_BAR), DEC); 
    } 
  }
  else { //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //读取控制器并设置大电机以“振动”速度旋转

 
    
  if(ps2x.Button(PSB_START))//start         //will be TRUE as long as button is pressed
    Serial.println("Start is being held");
  if(ps2x.Button(PSB_SELECT))//select
    Serial.println("Select is being held");      
  if(ps2x.Button(PSB_L3))//左边摇杆摁下
    Serial.println("L3 pressed");
    if(ps2x.Button(PSB_R3))//右边摇杆摁下
    Serial.println("R3 pressed");
    if(ps2x.Button(PSB_L1))//左上1
    {
      
    }
    if(ps2x.Button(PSB_R1))//左上2
    {
      
    }
    
    if(ps2x.Button(PSB_L2))//左上2
      {
        Serial.write(0xFF); 
        Serial.write(0x00); 
        Serial.write(0x06); 
        Serial.write(0x00); 
        Serial.write(0xFF); 
        Serial.write(0xFF+0x00+0x06+0x00+0xFF); 
      }
    if(ps2x.Button(PSB_R2))//右上2
      {
        Serial.write(0xFF); 
        Serial.write(0x11); 
        Serial.write(0x05); 
        Serial.write(0x11); 
        Serial.write(0xFF); 
        Serial.write(0xFF+0x11+0x05+0x11+0xFF); 
      }
    if(ps2x.ButtonPressed(PSB_TRIANGLE))//三角
      {
        Serial.write(0xFF); 
        Serial.write(0x55); 
        Serial.write(0x01); 
        Serial.write(0x55); 
        Serial.write(0xFF); 
        Serial.write(0xFF+0x55+0x01+0x55+0xFF); 
      }
    if(ps2x.Button(PSB_PAD_UP))//上
    {      //will be TRUE as long as button is pressed
      Serial.print("Up held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_UP), DEC);
    }
    if(ps2x.Button(PSB_PAD_RIGHT))//右
    {
      Serial.print("Right held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_RIGHT), DEC);
    }
    if(ps2x.Button(PSB_PAD_LEFT))//左
    {
      Serial.print("LEFT held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_LEFT), DEC);
    }
    if(ps2x.Button(PSB_PAD_DOWN))//下
    {
      Serial.print("DOWN held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
    }   

    vibrate = ps2x.Analog(PSAB_CROSS);  //这将根据你按下蓝色按钮的力度来设定大的电机振动速度

    if(ps2x.Button(PSB_CIRCLE))               //圆
      {
        Serial.write(0xFF);
        Serial.write(0x44);
        Serial.write(0x02);
        Serial.write(0x44);
        Serial.write(0xFF);
        Serial.write(0xFF+0x44+0x002+0x44+0xFF);
      }
    if(ps2x.Button(PSB_CROSS))               //×
      {Serial.write(0xFF);
      Serial.write(0x33);
      Serial.write(0x03);
      Serial.write(0x33);
      Serial.write(0xFF);
      Serial.write(0xFF+0x33+0x03+0x33+0xFF);}
    if(ps2x.Button(PSB_SQUARE))              //方块
      {Serial.write(0xFF);
//      Serial.write(0x22);
//      Serial.write(0x04);
//      Serial.write(0x22);
//      Serial.write(0xFF);
      //Serial.write(0xFF+0x22+0x04+0x22+0xFF);  
      }
//    if(ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) { //print stick values if either is TRUE
//      Serial.print("Stick Values:");
//      Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
//      Serial.print(",");
//      Serial.print(ps2x.Analog(PSS_LX), DEC); 
//      Serial.print(",");
//      Serial.print(ps2x.Analog(PSS_RY), DEC); 
//      Serial.print(",");
//      Serial.println(ps2x.Analog(PSS_RX), DEC); }
  }
  delay(50);  
}
