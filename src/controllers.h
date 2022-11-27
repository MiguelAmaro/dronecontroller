#ifndef CONTROLLERS_H
#define CONTROLLERS_H

typedef enum controller_kind controller_kind;
enum controller_kind
{
  // NOTE(MIGUEL): Wtf are these constants??? 
  ControllerKind_Stick    = 0xb10a044f,
  ControllerKind_Throttle = 0xb687044f
};
typedef struct controller controller;
struct controller
{
  b32 Connected;
  controller_kind Kind;
};

b32   ControllerInit(controller *Controller, controller_kind Kind, os_window Window);
void  ControllerPoll(controller_kind Kind, platform *Platform);


#endif //CONTROLLERS_H
