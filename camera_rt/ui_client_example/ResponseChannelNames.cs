using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ui_client_example
{
    public class ResponseChannelNames
    {       
        public const string discovery_res_channel = "DISCOVERYRES";
        public const string init_session_res_channel = "INITSESSIONRES";
        public const string end_session_res_channel = "ENDSESSIONRES";
        public const string stream_res_channel = "STREAMURIRES";
        public const string ptz_control_res_channel = "PTZCONTROLRES";      
        public const string position_res_channel = "POSITIONRES";    
        public const string stop_ptz_control_res_channel = "STOPPTZCONTROLRES";  
        public const string preset_config_res_channel = "PRESETCONFRES";   
        public const string preset_move_res_channel = "PRESETMOVERES";
    }
}
