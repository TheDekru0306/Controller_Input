#include <iostream>
#include "Versions/JS_ZERO_VERSION/JS_ZERO.h"
#include "Versions/EVENT_BASED_VERSION/Event_Based.h"
#include "Versions/HidBased/HID_Based.h"
int main() {
   /* JS_ZERO js;
    js.Update();*/

   Event_Based eb;
   eb.Initialize();
   while(true){
       eb.Update();
   }

    /*HID_Based test;
    test.Intialize();
    while(true){
        test.Update();
    }*/


    return 0;
}
