#include "FlightControl_Platform.h"
#include "LAL.h"

typedef struct App {
    u32 stuff;
} App;

global App *app = 0;
global Platform *platform = 0;


b32 App_Update(Platform *platform_)
{
    b32 app_should_quit = 0;
    platform = platform_;
    app = platform->permanent_storage;
    {
        if(platform->key_down[KEY_a])
        {
            app_should_quit = 1;
        }
    }
    
    return app_should_quit;
}
