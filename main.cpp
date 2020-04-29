#include "mbed.h"
#include "math.h"
#include "mbed_events.h"
Ticker time_up;
//DigitalOut led1(LED1);
DigitalOut led2(LED2);
InterruptIn btn(SW2);
EventQueue printfQueue;
EventQueue eventQueue;
void acc();
void blink_led2() {
  // this runs in the normal priority thread
  led2 = !led2;
}

void print_toggle_led() {
  // this runs in the lower priority thread
  printf("Toggle LED!\r\n");
}

void btn_fall_irq() {
  //led1 = !led1;
   //printf("0");
  time_up.attach( &blink_led2, 0.2 );
  // defer the printf call to the low priority thread
  printfQueue.call(&acc);
  //printf("1");
  //acc();
  //printf("2");
}

#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void acc(){
   pc.baud(115200);
   uint8_t who_am_i, data[2], res[6];
   int16_t acc16;
   float t[3];
   // Enable the FXOS8700Q
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);
   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);
   //pc.printf("Here is %x\r\n", who_am_i);
   float time=0.0;
   float disX=0.0;//dispalcement in X axis
   float disY=0.0;//displacement in Y axis
   float displacement=0.0;//total displacement in XY-plane
   while (true) {
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[0] = ((float)acc16) / 4096.0f;
      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[1] = ((float)acc16) / 4096.0f;
      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[2] = ((float)acc16) / 4096.0f;
      
         
         printf("%1.4f\r\n",t[0]);
         printf("%1.4f\r\n",t[1]);
         printf("%1.4f\r\n",t[2]);
         
         float tilt;//if >5cm, means true and tilt=1 
         disX=disX+9.8*0.5*t[0]*0.1*0.1;//x=x_0+9.8*0.5*a_x*t^2;
         disY=disY+9.8*0.5*t[1]*0.1*0.1;//y=y_0+9.8*0.5*a_y*t^2;
         //displacement=0.5*a*time*time;
         displacement=sqrt(disX*disX+disY*disY);
         if(displacement>5){
            tilt=1;//displacement > 5
         }
         else{
            tilt=0;//displacement <=5
         }
      printf("%1.2f\r\n",tilt);      
      //printf("%1.2f\n",time);
      wait(0.1);
      time=time+0.1;
      if(time>10.0)
      break;
      
      
   }   
}
int main() {
  // low priority thread for calling printf()
  Thread printfThread(osPriorityLow);
  printfThread.start(callback(&printfQueue, &EventQueue::dispatch_forever));
  // normal priority thread for other events
  Thread eventThread(osPriorityNormal);
  eventThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));
  // call blink_led2 every second, automatically defering to the eventThread
  //Ticker ledTicker;
  //ledTicker.attach(eventQueue.event(&blink_led2), 1.0f);
  // button fall still runs in the ISR
   led2 =1;
  btn.fall(&btn_fall_irq);
 
  while (1) {wait(1);}
}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}