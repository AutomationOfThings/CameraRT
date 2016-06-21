The UI shall use the lcmdotnet library to publish data to the RT. Channel Name shall be “PTZCAMERA” for all the requests. Steps to executing sample code:


1. Reference <LCM.dll> in your project
2. Using lcm-gen.exe Compile DiscoveryRequest_t to get DiscoveryRequest_t.cs
                lcm-gen --csharp DiscoveryRequest_t.lcm


Sample C# code
LCM.LCM lcm = LCM.LCM.Singleton;
DiscoveryRequest_t discoveryRequest = new DiscoveryRequest_t ();
lcm.Publish("PTZCAMERA", temp); 


         
1. Discovery Request


        Request for discovering all network cameras


        package PtzCameraRequests;
struct DiscoveryRequest_t
{
}


1. Discovery Request Response
        
        package PtzCameraResponses
        struct DiscoveryResponse_t
{
        int32_t totalCams;
        string cameraNames[totalCams];
}


1. Stream URI Request
        struct StreamUriRequest_t
        {
                string cameraName;
        }


1. Stream URI Response
        struct StreamUriResponse_t
        {        
                string cameraName;
                string uri;
        }
1. PTZ Control Request
        struct PtzControlRequest_t
        {
                string cameraName;
                int8_t panValue; // ? -180 to 180
                int8_t tiltValue; // ? -180 to 180
                int8_t zoomValue; // ? 1 to 16
        }


        
1. Position Request
        struct PositionRequest_t
        {
                string cameraName;                
        }


1. Preset Request
        TODO
 
1. Group control Request
TODO