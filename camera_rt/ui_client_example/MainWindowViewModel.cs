using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Windows.Documents;
using System.Windows.Input;
using Prism.Commands;
using PtzCamera;

//using LCM = LCM.LCM;

namespace ui_client_example
{
    public class MainWindowViewModel
    {
        public ICommand DiscoverCommand { get; set; }
        public ICommand GetStreamUriCommand { get; set; }
        private readonly LCM.LCM.LCM _lcm;
        private string[] _channels = 
        {
            "DISCOVERYREQ", "STREAMURIREQ", "PTZCONTROLREQ", "POSITIONREQ"
        };
        public MainWindowViewModel()
        {
            DiscoverCommand = new DelegateCommand(OnDiscoverCommand);
            GetStreamUriCommand = new DelegateCommand(OnGetStreamUriCommand);
            _lcm = LCM.LCM.LCM.Singleton;
        }

        public ObservableCollection<string> CameraList
        {
            get
            {
                var list = new List<string>()
                {
                    "Camera1",
                    "Camera2"
                };

                return new ObservableCollection<string>(list);

            } 
            set { }
        }

        public string StreamUri { get; set; }

        public int SelectedCamera { get; set; }

        private void OnGetStreamUriCommand()
        {
            var streamUriRequest = new StreamUriRequest_t()
            {
                cameraName = SelectedCamera.ToString()
            };
            _lcm.Publish(_channels[1], streamUriRequest);
        }

        private void OnDiscoverCommand()
        {
            var discoveryRequest = new DiscoveryRequest_t();
            _lcm.Publish(_channels[0], discoveryRequest);
        }

    }
}
