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
using System.Threading.Tasks.Dataflow;
using System.Threading.Tasks;
using System.Windows;
//using LCM = LCM.LCM;

namespace ui_client_example
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public ICommand DiscoverCommand { get; set; }
        public ICommand InitSessionCommand { get; set; }
        public ICommand EndSessionCommand { get; set; }
        public ICommand GetStreamUriCommand { get; set; }
        public ICommand PanRightCommand { get; set; }
        public ICommand PanLeftCommand { get; set; }
        public ICommand TiltUpCommand { get; set; }
        public ICommand TiltDownCommand { get; set; }
        public ICommand ZoomInCommand { get; set; }
        public ICommand ZoomOutCommand { get; set; }
        public ICommand LoginCommand { get; set; }
        public ICommand AbsMoveCommand { get; set; }
        public ICommand StopMoveCommand { get; set; }
        public string Username { get; set; }      
        public string Password { get; set; }
        public bool IsContinuousMode { get; set; }
        public string InitSessionResponse { get; set; }
        public string EndSessionResponse { get; set; }

        private LoginDialog _loginDialog; 
        private readonly LCM.LCM.LCM _lcm;
       

        public MainWindowViewModel()
        {
            DiscoverCommand = new DelegateCommand(OnDiscoverCommand);
            InitSessionCommand = new DelegateCommand(OnInitSessionCommand);
            EndSessionCommand = new DelegateCommand(OnEndSessionCommand);
            GetStreamUriCommand = new DelegateCommand(OnGetStreamUriCommand);
            PanRightCommand = new DelegateCommand(OnPanRightCommand);
            PanLeftCommand = new DelegateCommand(OnPanLeftCommand);
            TiltUpCommand = new DelegateCommand(OnTiltUpCommand);
            TiltDownCommand = new DelegateCommand(OnTiltDownCommand);
            ZoomInCommand = new DelegateCommand(OnZoomInCommand);
            ZoomOutCommand = new DelegateCommand(OnZoomOutCommand);
            LoginCommand = new DelegateCommand<LoginDialog>(OnLoginCommand);
            AbsMoveCommand = new DelegateCommand(OnAbsMoveCommand);
            StopMoveCommand = new DelegateCommand(OnStopMoveCommand);

            _lcm = LCM.LCM.LCM.Singleton;

            dynamic app = ui_client_example.App.Current;
            var ea = (EventAggregator)app.EA;
            ea.GetEvent<DiscoveryResponseReceivedEvent>().Subscribe(OnDiscoveryResponseReceived);
            ea.GetEvent<StreamUriResponseReceivedEvent>().Subscribe(OnStreamUriResponseReceived);
            ea.GetEvent<PtzControlResponseReceivedEvent>().Subscribe(OnPtzControlResponseReceived);
            ea.GetEvent<PositionResponseReceivedEvent>().Subscribe(OnPositionResponseReceived);
            ea.GetEvent<StopPtzControlResponseReceivedEvent>().Subscribe(OnStopPtzControlResponseReceived);
            ea.GetEvent<InitSessionResponseReceivedEvent>().Subscribe(OnInitSessionResponseReceived);
            ea.GetEvent<EndSessionResponseReceivedEvent>().Subscribe(OnEndSessionResponseReceived);
        }

        private void OnEndSessionResponseReceived(end_session_response_t end_session_response)
        {
           EndSessionResponse = end_session_response.response_message;
           OnPropertyChanged("EndSessionResponse");
        }

        private void OnInitSessionResponseReceived(init_session_response_t init_session_response)
        {
            InitSessionResponse = init_session_response.response_message;
            OnPropertyChanged("InitSessionResponse");
        }

        private void OnStopPtzControlResponseReceived(stop_ptz_control_response_t stop_ptz_control_response)
        {
            if (stop_ptz_control_response.status_code == status_codes_t.ERR)
                MessageBox.Show(stop_ptz_control_response.response_message);

            PtzControlResponse = stop_ptz_control_response.response_message;
            OnPropertyChanged("PtzControlResponse");
        }

        private void OnStopMoveCommand()
        {
            var ptzStopControlRequest = new stop_ptz_control_request_t()
            {
                ip_address = CameraList[SelectedCamera],
                operation_type = stop_ptz_control_request_t.ALL
            };

            _lcm.Publish(Channels.stop_ptz_control_req_channel, ptzStopControlRequest);
            _lcm.Subscribe(Channels.stop_ptz_control_res_channel, new StopPtzControlResponseHandler());
        }

        private void OnAbsMoveCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList[SelectedCamera],
                pan_value = PanValue??"",
                tilt_value = TiltValue??"",
                zoom_value = ZoomValue??""
            };

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
          
        }

        private void OnEndSessionCommand()
        {
            var endSessionRequest = new end_session_request_t()
            {
                ip_address = CameraList[SelectedCamera]
            };

            _lcm.Subscribe(Channels.end_session_res_channel, new EndSessionResponseHandler());
            _lcm.Publish(Channels.end_session_req_channel, endSessionRequest);
           
        }

        private void OnDiscoveryResponseReceived(discovery_response_t discovery_response)
        {
            CameraList = new ObservableCollection<string>(discovery_response.ip_addresses);
            OnPropertyChanged("CameraList");
        }

        private void OnPositionResponseReceived(position_response_t position_response)
        {
            CurrentPanValue = position_response.pan_value;
            CurrentTiltValue = position_response.tilt_value;
            CurrentZoomValue = position_response.zoom_value;
            OnPropertyChanged("CurrentPanValue");
            OnPropertyChanged("CurrentTiltValue");
            OnPropertyChanged("CurrentZoomValue");
        }

        public string CurrentPanValue { get; set; }
        public string CurrentTiltValue { get; set; }
        public string CurrentZoomValue { get; set; }

        public string PanValue { get; set; }
        public string TiltValue { get; set; }
        public string ZoomValue { get; set; }

        private void OnPtzControlResponseReceived(ptz_control_response_t ptz_control_response)
        {
            if (ptz_control_response.status_code == status_codes_t.ERR)
                MessageBox.Show(ptz_control_response.response_message);

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
                ip_address = CameraList == null || CameraList.Count == 0 ? 
                "127.0.0.1" : CameraList[SelectedCamera],
                username = Username??" ",
                password = Password??" "
            };

            _lcm.Subscribe(Channels.init_session_res_channel, new InitSessionResponseHandler());
            _lcm.Publish(Channels.init_session_req_channel, initSessionRequest);           

            PollCameraPosition();
        }

        private void PollCameraPosition()
        {
            var positionRequest = new position_request_t()
            {
                ip_address = CameraList == null ? "127.0.0.1" : CameraList[SelectedCamera],
            };

            ActionBlock<position_request_t> pollingBlock = null;
            pollingBlock = new ActionBlock<position_request_t>(
                async x =>
                {
                    _lcm.Publish(Channels.position_req_channel, x);
                    await Task.Delay(TimeSpan.FromSeconds(5)).
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
                mode = ptz_control_request_t.REL,
                ip_address = CameraList[SelectedCamera],
                pan_value = "5",
                tilt_value = "",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);          
        }
      
        private void OnPanRightCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList[SelectedCamera],
                pan_value = "-5",
                tilt_value = "",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);            
        }

        private void OnTiltUpCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "-5",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);          
        }

        private void OnTiltDownCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "5",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
          
        }

        private void OnZoomInCommand()
        {
            var zoom_v = (Convert.ToDouble(CurrentZoomValue) + 1).ToString();
         
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList[SelectedCamera],   
                pan_value = "",
                tilt_value = "",             
                zoom_value = zoom_v,

            };

            if (IsContinuousMode)
            {
                ptzControlRequest.mode = ptz_control_request_t.CON;
                ptzControlRequest.zoom_value = "1"; //probably means zoom speed
            }

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
           
        }

        private void OnZoomOutCommand()
        {
            var zoom_value = (Convert.ToDouble(CurrentZoomValue) - 1).ToString(); 

            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "",
                zoom_value = zoom_value
            };

            if (IsContinuousMode)
            {
                ptzControlRequest.mode = ptz_control_request_t.CON;
                ptzControlRequest.zoom_value = "-1"; //probably means zoom speed
            }

            _lcm.Subscribe(Channels.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Publish(Channels.ptz_control_req_channel, ptzControlRequest);
          
        }


        private void OnStreamUriResponseReceived(stream_uri_response_t res)
        {
            if (res.status_code == status_codes_t.ERR)
                MessageBox.Show(res.response_message);

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

            _lcm.Subscribe(Channels.stream_res_channel, new StreamUriResponseHandler());
            _lcm.Publish(Channels.stream_req_channel, streamUriRequest);
           
        }

        private void OnDiscoverCommand()
        {
            var discoveryRequest = new discovery_request_t();

            _lcm.Subscribe(Channels.discovery_res_channel, new DiscoveryResponseHandler());
            _lcm.Publish(Channels.discovery_req_channel, discoveryRequest);
           
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
