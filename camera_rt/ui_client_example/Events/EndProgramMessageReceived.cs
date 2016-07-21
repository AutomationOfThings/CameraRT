using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class EndProgramMessageReceived : PubSubEvent<end_program_message_t>
    {
    }
}
