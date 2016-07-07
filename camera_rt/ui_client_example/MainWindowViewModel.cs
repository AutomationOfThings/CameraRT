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
using System.Windows.Controls;
using System.Threading.Tasks.Dataflow;
using System.Threading.Tasks;
//using LCM = LCM.LCM;

namespace ui_client_example
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public ICommand DiscoverCommand { get; set; }
        public ICommand InitSessionCommand { get; set; }
        public ICommand GetStreamUriCommand { get; set; }
        public ICommand PanRightCommand { get; set; }
        public ICommand PanLeftCommand { get; set; }
        public ICommand TiltUpCommand { get; set; }
        public ICommand TiltDownCommand { get; set; }
        public ICommand ZoomInCommand { get; set; }
        public ICommand ZoomOutCommand { get; set; }
        public ICommand LoginCommand { get; set; }
        public string Username { get; set; }      
        public string Password { get; set; }

        private LoginDialog _loginDialog; 
        private readonly LCM.LCM.LCM _lcm;
       

        public MainWindowViewModel()
        {
            DiscoverCommand = new DelegateCommand(OnDiscoverCommand);
            InitSessionCommand = new DelegateCommand(OnInitSessionCommand);
            GetStreamUriCommand = new DelegateCommand(OnGetStreamUriCommand);
            PanRightCommand = new DelegateCommand(OnPanRightCommand);
            PanLeftCommand = new DelegateCommand(OnPanLeftCommand);
            TiltUpCommand = new DelegateCommand(OnTiltUpCommand);
            TiltDownCommand = new DelegateCommand(OnTiltDownCommand);
            ZoomInCommand = new DelegateCommand(OnZoomCommand);
            ZoomOutCommand = new DelegateCommand(OnZoomCommand);
            LoginCommand = new DelegateCommand<LoginDialog>(OnLoginCommand);

            _lcm = LCM.LCM.LCM.Singleton;

            dynamic app = ui_client_example.App.Current;
            var ea = (EventAggregator)app.EA;
            ea.GetEvent<DiscoveryResponseReceivedEvent>().Subscribe(OnDiscoveryResponseReceived);
            ea.GetEvent<StreamUriResponseReceived>().Subscribe(OnStreamUriResponseReceived);
            ea.GetEvent<PtzControlResponseReceivedEvent>().Subscribe(OnPtzControlResponseReceived);
            ea.GetEvent<PositionResponseReceivedEvent>().Subscribe(OnPositionResponseReceived);
        }

        private void OnDiscoveryResponseReceived(discovery_response_t discovery_response)
        {
            CameraList = new ObservableCollection<string>(discovery_response.camera_names);
            OnPropertyChanged("CameraList");
        }

        private void OnPositionResponseReceived(position_response_t position_response)
        {
            PanValue = position_response.pan_value;
            TiltValue = position_response.tilt_value;
            ZoomValue = position_response.zoom_value;
            OnPropertyChanged("PanValue");
            OnPropertyChanged("TiltValue");
            OnPropertyChanged("ZoomValue");
        }

        public string PanValue { get; set; }
        public string TiltValue { get; set; }
        public string ZoomValue { get; set; }

        private void OnPtzControlResponseReceived(ptz_control_response_t ptz_control_response)
        {
            PtzControlResponse = ptz_control_response.response_message;
            OnPropertyChanged("PtzControlResponse");
        }

        private void OnInitSessionCommand()
        {

            _loginDialog = new LoginDialog();
            _loginDialog.DataContext = this;
            _loginDialog.Show();
        }

        public void OnLoginCommand(LoginDialog loginDialog)
        {          
            _loginDialog.Close();
            var initSessionRequest = new init_session_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                username = Username,
                password = Password
            };

            _lcm.Publish(Channels.init_session_req_channel, initSessionRequest);

            PollCameraPosition();
        }

        private void PollCameraPosition()
        {
            var positionRequest = new position_request_t()
            {
                ip_address = CameraList[SelectedCamera],
            };

            ActionBlock<position_request_t> pollingBlock = null;
            pollingBlock = new ActionBlock<position_request_t>(
                async x =>
                {
                    _lcm.Publish(Channels.position_req_channel, x);
                    await Task.Delay(TimeSpan.FromSeconds(1)).
                        ConfigureAwait(false);
                    pollingBlock.Post(x); //post the same request again for polling
                });

            _lcm.Subscribe(Channels.position_res_channel, new PositionResponseHandler());
            pollingBlock.Post(positionRequest); //seed and start the poller
            
        }

        private void OnPanLeftCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                pan_value = 10,
                tilt_value = 0,
                zoom_value = 0
            };

            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
        }
      
        private void OnPanRightCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                pan_value = -10,
                tilt_value = 0,
                zoom_value = 0
            };

            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
        }

        private void OnTiltUpCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                pan_value = 0,
                tilt_value = 10,
                zoom_value = 0
            };

            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
        }

        private void OnTiltDownCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                pan_value = 0,
                tilt_value = -10,
                zoom_value = 0
            };

            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
        }

        private void OnZoomCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                pan_value = 0,
                tilt_value = 0,
                zoom_value = 5
            };

            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
        }


        private void OnStreamUriResponseReceived(stream_uri_response_t res)
        {
            StreamUri = res.uri;
            OnPropertyChanged("StreamUri");
        }


        public ObservableCollection<string> CameraList {get; set;}

        public string PtzControlResponse { get; set;}

        public string StreamUri { get; set; }

        public int SelectedCamera { get; set; }

        private void OnGetStreamUriCommand()
        {
           
            var ip_address = CameraList[SelectedCamera];       

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
            _lcm.Publish(Channels.stream_req_channel, streamUriRequest);
            _lcm.Subscribe(Channels.stream_res_channel, new StreamUriResponseHandler());
        }

        private void OnDiscoverCommand()
        {
            var discoveryRequest = new discovery_request_t();
            _lcm.Publish(Channels.discovery_req_channel, discoveryRequest);
            _lcm.Subscribe(Channels.discovery_res_channel, new DiscoveryResponseHandler());
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
