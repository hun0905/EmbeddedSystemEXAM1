#include "mbed.h"
#include "uLCD_4DGL.h"
#include<iostream>
using namespace std::chrono;
const double pi = 3.141592653589793238462;
const double amplitude = 0.5f;
const double offset = 65535 ;

// The sinewave is created on this pin
// Adjust analog output pin name to your board spec.
AnalogOut aout(PA_5);
AnalogIn ain(A5);
uLCD_4DGL uLCD(D1, D0, D2);//Problem point. connect ulcd 
InterruptIn button1(PC_5);//up
InterruptIn button2(PC_4);//down
InterruptIn button3(PC_3);//reset
DigitalIn mypin(USER_BUTTON);//out data

EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
EventQueue queue3(32 * EVENTS_EVENT_SIZE);
EventQueue queue4(32 * EVENTS_EVENT_SIZE);
EventQueue queue5(32 * EVENTS_EVENT_SIZE);
Thread t1,t2,t3,t4,t5;

int j = 3;
float ADCdata[5000];
int t = 0;
int Time[4] = {10,20,40,80};//cycle time
int TimeNow = Time[j];//cycle time now
int RiseTime = TimeNow;
int FallTime = TimeNow;
void Display(int &i)
{
    uLCD.cls();
    uLCD.text_width(2); //4X size text
    uLCD.text_height(2);
    uLCD.locate(4,4);
    if(i == 0)
        uLCD.printf("%s","1/8");
    else if(i == 1)
        uLCD.printf("%s","1/4");
    else if(i ==2)
        uLCD.printf("%s","1/2");
    else
        uLCD.printf("%s","1"); 

}
void Sampling()
{
    while(1)
    {
        ADCdata[t] = ain;
        t++;
        if(t>1000)
        {
            for(int i = 0 ; i < 1000 ; i++)
            {
                std::cout<<ADCdata[i]<<"\r\n";
            }
            t=0;
        }
        ThisThread::sleep_for(1ms);
    }
}
void Generate()
{
    uint16_t sample = 0;
    Display(j);
    while (1) {
        for (int i = 0; i < TimeNow*2; i++) {
            if(i < TimeNow)   
                sample = (uint16_t)(offset*i/RiseTime);
            else if(i > TimeNow )
                sample = (uint16_t)(offset*(2*TimeNow-i)/FallTime);
            else if(i == TimeNow){
                if(j == 3)
                    ThisThread::sleep_for(79ms);
                else if(j == 2)
                    ThisThread::sleep_for(159ms);
                else if(j == 1)
                    ThisThread::sleep_for(199ms);
                else
                    ThisThread::sleep_for(219ms);
            }
            aout.write_u16(sample);
            ThisThread::sleep_for(1ms);
        }    
   }
}
void ISR1() //up
{
    j = (j == 3)?3:j+1;
    TimeNow = Time[j];
    RiseTime = TimeNow;
    FallTime = TimeNow;
    Display(j);
    ThisThread::sleep_for(100ms);
}
void ISR2()
{
    j = (j == 0)?0:j-1;
    TimeNow = Time[j];
    RiseTime = TimeNow;
    FallTime = TimeNow;
    Display(j);
    ThisThread::sleep_for(100ms);
}
void ISR3()//select
{
    //j = 2;
    Display(j);
    TimeNow = Time[j];
    RiseTime = TimeNow;
    FallTime = TimeNow;
    t4.start(callback(&queue4, &EventQueue::dispatch_forever));
    queue4.call(Generate);
    t5.start(callback(&queue5, &EventQueue::dispatch_forever));
    queue5.call(Sampling);
    ThisThread::sleep_for(100ms);
}
int main()
{
    /*int tmp;
    uint16_t sample = 0;
    int count = 0;*/
    Display(j);
    t1.start(callback(&queue1, &EventQueue::dispatch_forever));
    t2.start(callback(&queue2, &EventQueue::dispatch_forever));
    t3.start(callback(&queue3, &EventQueue::dispatch_forever));
    button1.rise(queue1.event(ISR1));
    button2.rise(queue2.event(ISR2));
    button3.rise(queue3.event(ISR3));
}