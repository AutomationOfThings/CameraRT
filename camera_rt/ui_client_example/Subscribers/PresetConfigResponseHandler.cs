﻿using LCM.LCM;
using ptz_camera;
using Prism.Events;
using ui_client_example.Events;

namespace ui_client_example.Subscribers
{
    public class PresetConfigResponseHandler : LCMSubscriber
    {
        public void MessageReceived(LCM.LCM.LCM lcm, string channel, LCMDataInputStream data_stream)
        {
            if (channel == ResponseChannelNames.preset_config_res_channel)
            {
                var response = new preset_config_response_t(data_stream);
                dynamic app = ui_client_example.App.Current;
                var ea = (EventAggregator)app.EA;
                ea.GetEvent<PresetConfigResponseReceivedEvent>().Publish(response);
            }
        }
    }
}
