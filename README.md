The UI shall use the lcmdotnet library to publish data to the RT. lcm (https://lcm-proj.github.io/) allows communication between processes via UDP multicast packets. The RT and UI agree to send and receive pre-defined types using the lcm type specification language. For example one of the types defined looks like:

    struct discovery_response_t
    {
      int32_t total_cams;
      string camera_names[total_cams];
    }

An executable called lcm-gen.exe is then used to compile the lcm type into language specific type. 


All the types that the UI and the runtime are going to use are found in the CameraRT\camera_rt_types\camera_rt_types.lcm file.

Use the lcm-gen.exe tool in the CameraRT\lcm folder to generate C# types as follows:

    lcm-gen.exe --csharp camera_rt_types.lcm

All the types will be put in a directory called ptz_camera. The directory is created in the same directory as where the file lcm-gen.exe exists.

Follow the following steps to publish and subscribe data with lcm.
<ol>
<li> Copy all the types to a directory in your project </li>
<li> Copy CameraRT\camera_rt\ui_client_example\LCM\lcm.dll to a directory in your project and add a reference to <lcm.dll> </li>
<li> Copy CameraRT\camera_rt\ui_client_example\Channels.cs to a directory in your project </li>
</ol>

You are now ready to use lcm. Look at the CameraRT\camera_rt\ui_client_example\MainWindowViewModel.cs class for complete samples.

Publish sample code:

    // IMPORTANT: SET unspecifed fields to the empty string ""
    var streamUriRequest = new stream_uri_request_t()
    {                
        ip_address = ip_address,
        profile = "1",
        codec_type = "",
        resolution = "",
        frame_rate = "",
        compression_level = "",
        channel = ""
    };
    _lcm.Publish(Channels.StreamUriReqChannel, streamUriRequest);

Subscribe sample code:

    public class StreamUriResponseHandler : LCMSubscriber
    {
        public void MessageReceived(LCM.LCM.LCM lcm, string channel, LCMDataInputStream data_stream)
        {
            if (channel == Channels.StreamUriResChannel)
            {
                stream_uri_response_t response = new stream_uri_response_t(data_stream);
                dynamic app = ui_client_example.App.Current;
                var ea = (EventAggregator)app.EA;
                ea.GetEvent<StreamUriResponseReceived>().Publish(response);
            }
        }
    }
    _lcm.Subscribe(Channels.StreamUriResChannel, new StreamUriResponseHandler());



