#pragma once
#include "Arduino.h"
using mcpwm_unit_t=int;using mcpwm_timer_t=int;using mcpwm_operator_t=int;using mcpwm_io_signals_t=int;
const int MCPWM_UNIT_0=0,MCPWM_UNIT_1=1,MCPWM_OPR_A=0,MCPWM_OPR_B=1;
const int MCPWM0A=0,MCPWM0B=1,MCPWM1A=2,MCPWM1B=3,MCPWM2A=4,MCPWM2B=5;
const int MCPWM_UP_COUNTER=0,MCPWM_DUTY_MODE_0=0;
struct mcpwm_config_t{double frequency;float cmpr_a,cmpr_b;int counter_mode,duty_mode;};
static std::map<int,std::pair<int,int>> routes;
static float duties[2][3][2]={};static int initCount[2][3]={};
inline void mcpwm_init(int u,int t,mcpwm_config_t* c){++initCount[u][t];duties[u][t][0]=c->cmpr_a;duties[u][t][1]=c->cmpr_b;}
inline void mcpwm_set_duty(int u,int t,int op,float d){assert(u>=0&&u<2&&t>=0&&t<3&&op>=0&&op<2);duties[u][t][op]=d;}
inline void mcpwm_set_duty_type(int,int,int,int){}
inline void mcpwm_gpio_init(int u,int sig,int pin){routes[pin]={u,sig};}
