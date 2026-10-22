#include "Arduino.h"
#include "driver/mcpwm.h"
#define TAG PWM_TAG
#include "ESP32PWM.cpp"
#undef TAG
#include "ESP32Servo.cpp"
#include <set>
int main(){
#ifdef CONFIG_IDF_TARGET_ESP32C3
    const int count=4, firstPin=1;
#else
    const int count=8, firstPin=12;
#endif
    Servo motors[count];
    for(int i=0;i<count;++i){
        motors[i].setPeriodHertz(50);motors[i].attach(i+firstPin,500,2500);
        assert(motors[i].attached());
        motors[i].write(90);
#ifdef CONFIG_IDF_TARGET_ESP32S3
        assert(motors[i].readTimerWidth()==14);
        std::set<std::pair<int,int>> unique;
        for(int j=0;j<=i;++j){
            auto r=routes.at(j+firstPin);unique.insert(r);
            double us=duties[r.first][r.second/2][r.second%2]*200;
            assert(std::abs(us-1500)<1.3); // Attaching a neighbour must preserve this.
        }
        assert(unique.size()==(size_t)i+1);
#endif
    }
#ifdef CONFIG_IDF_TARGET_ESP32S3
    for(int u=0;u<2;++u)for(int t=0;t<3;++t)assert(initCount[u][t]<=1);
    for(int i=0;i<count;++i){
        auto r=routes.at(i+firstPin);int value=500+i*250;motors[i].writeMicroseconds(value);
        assert(std::abs(duties[r.first][r.second/2][r.second%2]*200-value)<1.3);
        for(int j=0;j<i;++j){auto q=routes.at(j+firstPin);assert(std::abs(duties[q.first][q.second/2][q.second%2]*200-(500+j*250))<1.3);}
    }
    // Release and reuse a slot beside an active motor without resetting it.
    auto neighbour=routes.at(13);float saved=duties[neighbour.first][neighbour.second/2][neighbour.second%2];
    motors[0].release();motors[0].detach();motors[0].attach(12,500,2500);motors[0].write(90);
    assert(duties[neighbour.first][neighbour.second/2][neighbour.second%2]==saved);
    // Release a whole timer, then reinitialize it on reuse.
    motors[0].detach();motors[1].detach();motors[0].attach(12,500,2500);motors[0].write(90);
    assert(initCount[0][0]==2);
    puts("PASS: eight independent S3 outputs, shared-timer attach, 1500us neutral, independent commands, detach/reattach");
#else
    assert(routes.empty());assert(led.size()==count);
    puts("PASS: ESP32 LEDC path remains selected");
#endif
}
