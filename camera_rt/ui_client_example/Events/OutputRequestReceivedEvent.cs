using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class OutputRequestReceivedEvent : PubSubEvent<output_request_t>
    {
    }
}
