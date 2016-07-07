﻿using LCM.LCM;
using ptz_camera;
using Prism.Events;
using ui_client_example.Events;

namespace ui_client_example.Subscribers
{
    public class PositionResponseHandler : LCMSubscriber
    {
        public void MessageReceived(LCM.LCM.LCM lcm, string channel, LCMDataInputStream data_stream)
        {
            if (channel == Channels.position_res_channel)
            {
                var response = new position_response_t(data_stream);
                dynamic app = ui_client_example.App.Current;
                var ea = (EventAggregator)app.EA;
                ea.GetEvent<PositionResponseReceivedEvent>().Publish(response);
            }
        }
    }
}