using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class PositionResponseReceivedEvent : PubSubEvent<position_response_t>
    {
    }
}
