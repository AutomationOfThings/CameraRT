using LCM.LCM;
using ptz_camera;
using Prism.Events;
using ui_client_example.Events;

namespace ui_client_example.Subscribers
{
    public class OutputRequestHandler : LCMSubscriber
    {
        public void MessageReceived(LCM.LCM.LCM lcm, string channel, LCMDataInputStream data_stream)
        {
            if (channel == RequestChannelNames.output_req_channel)
            {
                var response = new output_request_t(data_stream);
                dynamic app = ui_client_example.App.Current;
                var ea = (EventAggregator)app.EA;
                ea.GetEvent<OutputRequestReceivedEvent>().Publish(response);
            }
        }
    }
}
