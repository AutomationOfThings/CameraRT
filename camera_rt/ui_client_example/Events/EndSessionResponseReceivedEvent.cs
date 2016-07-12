using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class EndSessionResponseReceivedEvent : PubSubEvent<end_session_response_t>
    {
    }
}
