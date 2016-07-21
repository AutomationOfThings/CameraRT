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
using System.Threading;
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
        public ICommand PresetSetCommand { get; set; }
        public ICommand PresetMoveCommand { get; set; }
        public ICommand WriteProgramCommand { get; set; }
        public ICommand StartProgramCommand { get; set; }
        public ICommand StopProgramCommand { get; set; }

        public string CurrentPanValue { get; set; }
        public string CurrentTiltValue { get; set; }
        public string CurrentZoomValue { get; set; }

        public string PanValue { get; set; }
        public string TiltValue { get; set; }
        public string ZoomValue { get; set; }

        public string Username { get; set; }      
        public string Password { get; set; }

        public string ProgramText { get; set; }

        public bool IsContinuousMode { get; set; }

        public string InitSessionResponse { get; set; }
        public string EndSessionResponse { get; set; }
        public string PresetSetResponse { get; set; }
        public string PresetMoveResponse { get; set; }
        public string PtzControlResponse { get; set; }

        public string StartProgramResponse { get; set; }
        public string StopProgramResponse { get; set; }
        public string OutputRequest { get; set; }
        public string ProgramLine { get; set; }

        public string PresetName { get; set; }
        public string PresetNumber { get; set; }

        public ObservableCollection<string> CameraList { get; set; }
        public string StreamUri { get; set; }
        public int SelectedCamera { get; set; }

        private LoginDialog _loginDialog;
        private WriteProgramDialog _writeProgramDialog;
        private readonly LCM.LCM.LCM _lcm;
        private CancellationTokenSource _positionPollerTokenSource = 
            new CancellationTokenSource();

        public MainWindowViewModel()
        {
            //_lcm = new LCM.LCM.LCM("udpm://239.255.76.67:7770");
            _lcm = new LCM.LCM.LCM();

            SubscribeForResponses();

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
            PresetSetCommand = new DelegateCommand(OnPresetSetCommand);
            PresetMoveCommand = new DelegateCommand(OnPresetMoveCommand);
            WriteProgramCommand = new DelegateCommand(OnWriteProgramCommand);
            StartProgramCommand = new DelegateCommand(OnStartProgramCommand);
            StopProgramCommand = new DelegateCommand(OnStopProgramCommand);

            dynamic app = ui_client_example.App.Current;
            var ea = (EventAggregator)app.EA;
            ea.GetEvent<DiscoveryResponseReceivedEvent>().Subscribe(OnDiscoveryResponseReceived);
            ea.GetEvent<StreamUriResponseReceivedEvent>().Subscribe(OnStreamUriResponseReceived);
            ea.GetEvent<PtzControlResponseReceivedEvent>().Subscribe(OnPtzControlResponseReceived);
            ea.GetEvent<PositionResponseReceivedEvent>().Subscribe(OnPositionResponseReceived);
            ea.GetEvent<StopPtzControlResponseReceivedEvent>().Subscribe(OnStopPtzControlResponseReceived);
            ea.GetEvent<InitSessionResponseReceivedEvent>().Subscribe(OnInitSessionResponseReceived);
            ea.GetEvent<EndSessionResponseReceivedEvent>().Subscribe(OnEndSessionResponseReceived);
            ea.GetEvent<PresetConfigResponseReceivedEvent>().Subscribe(OnPresetConfigResponseReceived);
            ea.GetEvent<PresetMoveResponseReceivedEvent>().Subscribe(OnPresetMoveResponseReceived);
            ea.GetEvent<StartProgramResponseReceivedEvent>().Subscribe(OnStartProgramResponseReceived);
            ea.GetEvent<OutputRequestReceivedEvent>().Subscribe(OnOutputRequestReceived);
            ea.GetEvent<StopProgramResponseReceivedEvent>().Subscribe(OnStopProgramResponseReceived);
            ea.GetEvent<ProgramStatusMessageReceivedEvent>().Subscribe(OnProgramStatusMessageReceived);
        }

        private void OnProgramStatusMessageReceived(program_status_message_t program_status_message)
        {
            ProgramLine = Convert.ToString(program_status_message.line_num);
            OnPropertyChanged("ProgramLine");
        }

        private void OnStopProgramResponseReceived(stop_program_response_t stop_program_response)
        {
            StopProgramResponse = stop_program_response.response_message;
            OnPropertyChanged("StopProgramResponse");

            if (stop_program_response.status_code == status_codes_t.ERR)
                MessageBox.Show(StopProgramResponse);
        }

        private void OnStartProgramCommand()
        {
            var start_program_request = new start_program_request_t()
            {
                program = ProgramText
            };

            _writeProgramDialog.Close();
            _lcm.Publish(RequestChannelNames.start_program_req_channel, start_program_request);
        }

        private void OnStopProgramCommand()
        {
            var stop_program_request = new stop_program_request_t();
            _lcm.Publish(RequestChannelNames.stop_program_req_channel, stop_program_request);
        }

        private void OnWriteProgramCommand()
        {
            _writeProgramDialog = new WriteProgramDialog();
            _writeProgramDialog.DataContext = this;
            _writeProgramDialog.Show();
        }

        private void OnOutputRequestReceived(output_request_t output_request)
        {
            OutputRequest = output_request.ip_address;
            OnPropertyChanged("OutputRequest");
        }

        private void OnStartProgramResponseReceived(start_program_response_t start_program_response)
        {
            StartProgramResponse = start_program_response.response_message;
            OnPropertyChanged("StartProgramResponse");

            if (start_program_response.status_code == status_codes_t.ERR)
                MessageBox.Show(StartProgramResponse);
        }

        private void SubscribeForResponses()
        {
            _lcm.Subscribe(ResponseChannelNames.discovery_res_channel, new DiscoveryResponseHandler());

            _lcm.Subscribe(ResponseChannelNames.init_session_res_channel, new InitSessionResponseHandler());
            _lcm.Subscribe(ResponseChannelNames.end_session_res_channel, new EndSessionResponseHandler());

            _lcm.Subscribe(ResponseChannelNames.position_res_channel, new PositionResponseHandler());

            _lcm.Subscribe(ResponseChannelNames.preset_config_res_channel, new PresetConfigResponseHandler());
            _lcm.Subscribe(ResponseChannelNames.preset_move_res_channel, new PresetMoveResponseHandler());

            _lcm.Subscribe(ResponseChannelNames.ptz_control_res_channel, new PtzControlResponseHandler());
            _lcm.Subscribe(ResponseChannelNames.stop_ptz_control_res_channel, new StopPtzControlResponseHandler());
      
            _lcm.Subscribe(ResponseChannelNames.stream_res_channel, new StreamUriResponseHandler());

            _lcm.Subscribe(ResponseChannelNames.start_program_res_channel, new StartProgramResponseHandler());
            _lcm.Subscribe(RequestChannelNames.output_req_channel, new OutputRequestHandler());
            _lcm.Subscribe(ResponseChannelNames.stop_program_res_channel, new StopProgramResponseHandler());
            _lcm.Subscribe(MessageChannelNames.program_status_mes_channel, new ProgramStatusMessageHandler());
        }

        private void OnPresetSetCommand()
        {
            var preset_set_request = new preset_config_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                mode = preset_config_request_t.ADD,
                preset_name = PresetName??"preset1",
                preset_number = PresetNumber??"1",

                // these values are not needed and will be removed
                pan_value = "",
                zoom_value = "",
                tilt_value = ""
            };

            _lcm.Publish(RequestChannelNames.preset_config_req_channel, preset_set_request);
        }

        private void OnPresetMoveCommand()
        {
            var preset_move_request = new preset_move_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                preset_number = PresetNumber??""
            };

            _lcm.Publish(RequestChannelNames.preset_move_req_channel, preset_move_request);
        }
        
        private void OnPresetMoveResponseReceived(preset_move_response_t move_response)
        {
            PresetMoveResponse = move_response.response_message;
            OnPropertyChanged("PresetMoveResponse");

            if (move_response.status_code == status_codes_t.ERR)
                MessageBox.Show(PresetMoveResponse);            
        }

        private void OnPresetConfigResponseReceived(preset_config_response_t config_response)
        {
            PresetSetResponse = config_response.response_message;
            OnPropertyChanged("PresetSetResponse");

            if (config_response.status_code == status_codes_t.ERR)
                MessageBox.Show(PresetSetResponse);
        }

        private void OnEndSessionResponseReceived(end_session_response_t end_session_response)
        {
           EndSessionResponse = end_session_response.response_message;
           OnPropertyChanged("EndSessionResponse");

            if (end_session_response.status_code == status_codes_t.ERR)
                MessageBox.Show(EndSessionResponse);
        }

        private void OnInitSessionResponseReceived(init_session_response_t init_session_response)
        {
            InitSessionResponse = init_session_response.response_message;
            OnPropertyChanged("InitSessionResponse");

            if (init_session_response.status_code == status_codes_t.ERR)
                MessageBox.Show(EndSessionResponse, "Try again");
        }

        private void OnStopPtzControlResponseReceived(stop_ptz_control_response_t stop_ptz_control_response)
        {
            PtzControlResponse = stop_ptz_control_response.response_message;
            OnPropertyChanged("PtzControlResponse");

            if (stop_ptz_control_response.status_code == status_codes_t.ERR)
                MessageBox.Show(stop_ptz_control_response.response_message);
        }

        private void OnStopMoveCommand()
        {
            var ptzStopControlRequest = new stop_ptz_control_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                operation_type = stop_ptz_control_request_t.ALL
            };

            _lcm.Publish(RequestChannelNames.stop_ptz_control_req_channel, ptzStopControlRequest);
         
        }

        private void OnAbsMoveCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = PanValue??"",
                tilt_value = TiltValue??"",
                zoom_value = ZoomValue??""
            };
                        
            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);
          
        }

        private void OnEndSessionCommand()
        {
            var endSessionRequest = new end_session_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera]
            };

            _positionPollerTokenSource.Cancel();            
            _lcm.Publish(RequestChannelNames.end_session_req_channel, endSessionRequest);
        }

        private void OnDiscoveryResponseReceived(discovery_response_t discovery_response)
        {
            CameraList = new ObservableCollection<string>(discovery_response.ip_addresses);
            OnPropertyChanged("CameraList");

            if (discovery_response.status_code == status_codes_t.ERR)
                MessageBox.Show(discovery_response.response_message);
        }

        private void OnPositionResponseReceived(position_response_t position_response)
        {
            CurrentPanValue = position_response.pan_value;
            CurrentTiltValue = position_response.tilt_value;
            CurrentZoomValue = position_response.zoom_value;
            OnPropertyChanged("CurrentPanValue");
            OnPropertyChanged("CurrentTiltValue");
            OnPropertyChanged("CurrentZoomValue");

            if (position_response.status_code == status_codes_t.ERR)
                MessageBox.Show(position_response.response_message);
        }       

        private void OnPtzControlResponseReceived(ptz_control_response_t ptz_control_response)
        {
            PtzControlResponse = ptz_control_response.response_message;
            OnPropertyChanged("PtzControlResponse");

            if (ptz_control_response.status_code == status_codes_t.ERR)
                MessageBox.Show(ptz_control_response.response_message);
        }

        private void OnInitSessionCommand()
        {

            _loginDialog = new LoginDialog();
            _loginDialog.DataContext = this;
            _loginDialog.Show();
        }

        private void OnLoginCommand(LoginDialog loginDialog)
        {          
            _loginDialog.Close();
            var initSessionRequest = new init_session_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ? 
                "127.0.0.1" : CameraList[SelectedCamera],
                username = Username??" ",
                password = Password??" "
            };
            
            _lcm.Publish(RequestChannelNames.init_session_req_channel, initSessionRequest);           

            PollCameraPosition(_positionPollerTokenSource.Token);
        }

        private void PollCameraPosition(CancellationToken cToken)
        {
            var positionRequest = new position_request_t()
            {
                ip_address = CameraList == null ? "127.0.0.1" : CameraList[SelectedCamera],
            };

            ActionBlock<position_request_t> pollingBlock = null;
            pollingBlock = new ActionBlock<position_request_t>(
                async x =>
                {
                    _lcm.Publish(RequestChannelNames.position_req_channel, x);
                    await Task.Delay(TimeSpan.FromSeconds(5), cToken).
                        ConfigureAwait(false);
                    pollingBlock.Post(x); //post the same request again for polling
                }, new ExecutionDataflowBlockOptions { CancellationToken = cToken });

            pollingBlock.Post(positionRequest); //seed and start the poller
        }

        private void OnPanLeftCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = "5",
                tilt_value = "",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;
          
            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);          
        }
      
        private void OnPanRightCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = "-5",
                tilt_value = "",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;
        
            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);            
        }

        private void OnTiltUpCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "-5",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;

            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);          
        }

        private void OnTiltDownCommand()
        {
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.REL,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "5",
                zoom_value = ""
            };

            if (IsContinuousMode)
                ptzControlRequest.mode = ptz_control_request_t.CON;
           
            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);
        }

        private void OnZoomInCommand()
        {
            var zoom_v = (Convert.ToDouble(CurrentZoomValue??"1") + 1).ToString();
         
            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],   
                pan_value = "",
                tilt_value = "",             
                zoom_value = zoom_v,

            };

            if (IsContinuousMode)
            {
                ptzControlRequest.mode = ptz_control_request_t.CON;
                ptzControlRequest.zoom_value = "1"; //probably means zoom speed
            }

            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);
        }

        private void OnZoomOutCommand()
        {
            var zoom_value = (Convert.ToDouble(CurrentZoomValue??"1") - 1).ToString(); 

            var ptzControlRequest = new ptz_control_request_t()
            {
                mode = ptz_control_request_t.ABS,
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                pan_value = "",
                tilt_value = "",
                zoom_value = zoom_value
            };

            if (IsContinuousMode)
            {
                ptzControlRequest.mode = ptz_control_request_t.CON;
                ptzControlRequest.zoom_value = "-1"; //probably means zoom speed
            }
            
            _lcm.Publish(RequestChannelNames.ptz_control_req_channel, ptzControlRequest);
        }


        private void OnStreamUriResponseReceived(stream_uri_response_t res)
        {
            StreamUri = res.uri;
            OnPropertyChanged("StreamUri");

            if (res.status_code == status_codes_t.ERR)
                MessageBox.Show(res.response_message);
        }      

        private void OnGetStreamUriCommand()
        {
            // IMPORTANT: SET unspecifed fields to the empty string ""
            var streamUriRequest = new stream_uri_request_t()
            {
                ip_address = CameraList == null || CameraList.Count == 0 ?
                "127.0.0.1" : CameraList[SelectedCamera],
                profile = "1",
                codec_type = "",
                resolution = "",
                frame_rate = "",
                compression_level = "",
                channel = ""
            };
            
            _lcm.Publish(RequestChannelNames.stream_req_channel, streamUriRequest);
        }

        private void OnDiscoverCommand()
        {
            var discoveryRequest = new discovery_request_t();
            _lcm.Publish(RequestChannelNames.discovery_req_channel, discoveryRequest);
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
