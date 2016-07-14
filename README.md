<h1> WHAT </h1>
This a program that shall be used to control multiple network cameras (SNP 6320H -- http://www.cctvcentersl.es/upload/Catalogos/SNP-6320_6320H_eng.pdf). The program shall susbcribe and publish requests and responses through UDP multicast datagrams. It is designed to run with a UI/GUI application that shall publish requests (see MORE section) defined using the lcm language (https://lcm-proj.github.io/type_specification.html) to control the network cameras, and it shall perform the appropriate http level requests to the network cameras. In the same way, it shall also translate the http responses to appropriate higher level messages (see MORE section).  As such, the program is called the camera runtime (CameraRT).

<h1> HOW </h1>

The UI shall use the lcmdotnet library to publish data to the RT. lcm (https://lcm-proj.github.io/) allows communication between processes via UDP multicast packets. The RT and UI shall agree to send and receive pre-defined types using the lcm type specification language. 

For example, one of the types defined looks like:

    struct discovery_response_t
    {
      int32_t total_cams;
      string camera_names[total_cams];
    }

An executable called lcm-gen.exe shall be used to compile the lcm type into language specific type. 
All the types that the UI and the runtime are going to use are found in the CameraRT\camera_rt_types\camera_rt_types.lcm file.

Use the lcm-gen.exe tool in the CameraRT\lcm folder to generate C# types as follows:

    lcm-gen.exe --csharp camera_rt_types.lcm

All the types will be put in a directory called ptz_camera. The directory is created in the same directory as where the file lcm-gen.exe exists. 

Follow the following steps to publish and subscribe data with lcm.
<ol>
<li> Copy all the types to a directory in your project </li>
<li> Copy CameraRT\camera_rt\ui_client_example\LCM\lcm.dll to a directory in your project and add a reference to it</li>
<li> Copy CameraRT\camera_rt\ui_client_example\RequestChannelNames.cs and CameraRT\camera_rt\ui_client_example\ResponseChannelNames.cs  to a directory in your project. These are the channel names which your lcm instance is going to use </li>
</ol>

You are now ready to use lcm. 

You will need to instantiate an instance of lcm (here as a singleton):

    _lcm = LCM.LCM.LCM.Singleton;
    
The first request is generally to discover cameras. Use the discovery_request_t type:

     var discoveryRequest = new discovery_request_t();
    _lcm.Subscribe(Channels.discovery_res_channel, new DiscoveryResponseHandler());
    _lcm.Publish(Channels.discovery_req_channel, discoveryRequest);

Notice the subscribe call before the publish call to ensure that any response is not missed.

The RT shall reply with a discovery respone which can be captured like this:

    public class DiscoveryResponseHandler : LCMSubscriber
    {
        public void MessageReceived(LCM.LCM.LCM lcm, string channel, LCMDataInputStream data_stream)
        {
            if (channel == Channels.discovery_res_channel)
            {
                var response = new discovery_response_t(data_stream);
                dynamic app = ui_client_example.App.Current;
                var ea = (EventAggregator)app.EA;
                ea.GetEvent<DiscoveryResponseReceivedEvent>().Publish(response);
            }
        }
    }
The `app.EA` variable referenced is a global event aggregator used for publishing and subscribing events.

The lcm type discovery_response_t is defined as:

    struct discovery_response_t
    {
        int32_t total_cams;
        string ip_addresses[total_cams];
    	int16_t status_code;
    	string response_message;
    }

where status_code is like an enum-like field based on the following lcm-type:

    struct status_codes_t
    {
    	const int16_t OK=1, ERR=2;
    }

The response object you will receive thus will have the:
<ul>
    <li>
    total_cams-- total number of cameras discovered
    </li>
    <li>
    ip_addresses[]-- array with the ip_addresses for the discovered cams
    </li>
    <li>
    status_code-- field with ok=1 and error=2 values
    </li>
    <li>
    response_message-- string field with details about the response (generally "OK" for good responses and error message for errors)
    </li>
</ul>

Before you can send any requests to the camera, you will need to send an `init_session_request_t` which contains login information for the camera. The runtime will send an `init_session_response_t` so that you can know whether the initialization was successful and prompt for any correction action if needed. Any session is retained indefinitely.

<h1> DEPENDENCIES </h1>
<ul>
<li>  lcm (https://lcm-proj.github.io/) -> library for internetwork request/response message passing through UDP multicast</li>
<li> cpprestsdk(https://github.com/Microsoft/cpprestsdk) -> library for handling http request/response passing to the network cameras
</ul>

<h1> MORE </h1>
All the requests supported are here for your reference:

        package ptz_camera;
        
        struct discovery_request_t
        {
        }
        
        struct status_codes_t
        {
        	const int16_t OK=1, ERR=2;
        }
        
        struct discovery_response_t
        {
            int32_t total_cams;
            string ip_addresses[total_cams];
        	int16_t status_code;
        	string response_message;
        }
        
        struct init_session_request_t
        {
        	string ip_address;
        	string username;	
        	string password;	
        }
        
        struct init_session_response_t
        {
        	string ip_address;
        	int16_t status_code;
        	string response_message;
        }
        
        struct end_session_request_t
        {
        	string ip_address;
        }
        
        struct end_session_response_t
        {
        	string ip_address;
        	string response;
        	int16_t status_code;
        	string response_message;
        }
        
        struct stream_uri_request_t
        {
        	string ip_address;
        	string profile;
        	string codec_type;
        	string resolution;
        	string frame_rate;
        	string compression_level;
        	string channel;
        }
        
        struct stream_uri_response_t
        {    
        	string ip_address;
            string uri;
        	int16_t status_code;
        	string response_message;
        }
        
        struct ptz_control_request_t
        {
        	const int8_t ABS=1, REL=2, CON=3;
        	int8_t mode;
            string ip_address;
            string pan_value;  //  0 to 360
            string tilt_value; // -15 to 195
            string zoom_value; //  1 to 32
        }
        
        struct stop_ptz_control_request_t
        {
        	string ip_address;
        	const int8_t ALL = 1, PAN = 2, TILT = 3, ZOOM = 4;
        	int8_t operation_type;
        }
        
        struct stop_ptz_control_response_t
        {
        	string ip_address;
        	int16_t status_code;
        	string response_message;
        }
        struct ptz_control_response_t
        {
        	string ip_address;
        	int16_t status_code;
        	string response_message;
        }
        
        struct position_request_t
        {
           string ip_address;        
        }
        
        struct position_response_t
        {
        	string ip_address;
        	string pan_value;
        	string tilt_value;
        	string zoom_value;
        	int16_t status_code;
        	string response_message;
        }

Please look at the CameraRT\camera_rt\ui_client_example\MainWindowViewModel.cs class for complete samples.
