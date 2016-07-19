using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class StartProgramResponseReceivedEvent : PubSubEvent<start_program_response_t>
    {
    }
}
