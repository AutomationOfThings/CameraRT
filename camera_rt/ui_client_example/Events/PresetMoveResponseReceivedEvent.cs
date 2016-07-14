using Prism.Events;
using ptz_camera;

namespace ui_client_example.Events
{
    class PresetMoveResponseReceivedEvent : PubSubEvent<preset_move_response_t>
    {
    }
}
