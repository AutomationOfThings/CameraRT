
using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class StreamUriResponseReceived : PubSubEvent<stream_uri_response_t>
    {
    }
}
