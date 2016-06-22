using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Input;
using Prism.Commands;
using Prism.Events;
using ui_client_example.Subscribers;
using ptz_camera;
using ui_client_example.Events;
using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

//using LCM = LCM.LCM;

namespace ui_client_example
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public ICommand DiscoverCommand { get; set; }
        public ICommand GetStreamUriCommand { get; set; }
        public ICommand PanRightCommand { get; set; }
        public ICommand PanLeftCommand { get; set; }

        private readonly LCM.LCM.LCM _lcm;
        
        public MainWindowViewModel()
        {
            DiscoverCommand = new DelegateCommand(OnDiscoverCommand);
            GetStreamUriCommand = new DelegateCommand(OnGetStreamUriCommand);
            PanRightCommand = new DelegateCommand(OnPanRightCommand);
            PanLeftCommand = new DelegateCommand(OnPanLeftCommand);

            _lcm = LCM.LCM.LCM.Singleton;

            dynamic app = ui_client_example.App.Current;
            var ea = (EventAggregator)app.EA;
            ea.GetEvent<StreamUriResponseReceived>().Subscribe(OnStreamUriResponseReceived);

        }

        private void OnPanLeftCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = "192.168.0.148",
                pan_value = 200,
                tilt_value = 0,
                zoom_value = 0
            };

            _lcm.Publish(Channels.PtzControlReqChannel, ptzControlRequest);
        }

        private void OnPanRightCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = "192.168.0.148",
                pan_value = 100,
                tilt_value = 0,
                zoom_value = 0
            };

            _lcm.Publish(Channels.PtzControlReqChannel, ptzControlRequest);
        }

        private void OnStreamUriResponseReceived(stream_uri_response_t res)
        {
            StreamUri = res.uri;
            OnPropertyChanged("StreamUri");       
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
            string ip_address = "";
            if (SelectedCamera == 0)
                ip_address = "192.168.0.148";
            if (SelectedCamera == 1)
                ip_address = "192.168.0.119";

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
            _lcm.Subscribe(Channels.StreamUriResChannel, new StreamUriResponseHandler());
        }

        private void OnDiscoverCommand()
        {
            var discoveryRequest = new discovery_request_t();
            _lcm.Publish(Channels.DiscoveryReqChannel, discoveryRequest);
        }


        public event PropertyChangedEventHandler PropertyChanged;
        
        private void OnPropertyChanged([CallerMemberName] String propertyName = "")
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
