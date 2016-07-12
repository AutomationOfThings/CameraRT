using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class StopPtzControlResponseReceivedEvent : PubSubEvent<stop_ptz_control_response_t>
    {
    }
}
