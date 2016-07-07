using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class PtzControlResponseReceivedEvent : PubSubEvent<ptz_control_response_t>
    {
    }
}
