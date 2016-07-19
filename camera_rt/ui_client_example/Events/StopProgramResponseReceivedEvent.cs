using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class StopProgramResponseReceivedEvent : PubSubEvent<stop_program_response_t>
    {
    }
}
