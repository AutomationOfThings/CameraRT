using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ui_client_example
{
    public class RequestChannelNames
    {
        public const string discovery_req_channel = "DISCOVERYREQ";       
        public const string init_session_req_channel = "INITSESSIONREQ";      
        public const string end_session_req_channel = "ENDSESSIONREQ";     
        public  const string stream_req_channel = "STREAMURIREQ";       
        public const string ptz_control_req_channel = "PTZCONTROLREQ";
        public const string position_req_channel = "POSITIONREQ";
        public const string stop_ptz_control_req_channel = "STOPPTZCONTROLREQ";      
        public const string preset_config_req_channel = "PRESETCONFREQ";
        public const string preset_move_req_channel = "PRESETMOVEREQ";
        public const string start_program_req_channel = "STARTPROGRAMREQ";
        public const string stop_program_req_channel = "STOPPROGRAMREQ";
        public const string output_req_channel = "OUTPUTREQ";
    }
}
