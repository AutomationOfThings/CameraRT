using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class InitSessionResponseReceivedEvent : PubSubEvent<init_session_response_t>
    {
    }
}
