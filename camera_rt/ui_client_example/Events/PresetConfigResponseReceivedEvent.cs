using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class PresetConfigResponseReceivedEvent : PubSubEvent<preset_config_response_t>
    {
    }
}
