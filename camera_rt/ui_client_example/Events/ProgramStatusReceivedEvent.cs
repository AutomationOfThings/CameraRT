using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class ProgramStatusMessageReceivedEvent : PubSubEvent<program_status_message_t>
    {
    }
}
