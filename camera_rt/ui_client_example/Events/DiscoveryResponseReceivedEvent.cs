using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class DiscoveryResponseReceivedEvent : PubSubEvent<discovery_response_t>
    {
    }
}
