using System.Runtime.InteropServices;
using System.Windows.Input;
using Prism.Commands;
using PtzCamera;
using XNSWindowctrlAxLib;

//using LCM = LCM.LCM;

namespace ui_client_example
{
    public class MainWindowViewModel
    {
        public ICommand DiscoverCommand { get; set; }

        public MainWindowViewModel()
        {
            DiscoverCommand = new DelegateCommand(OnDiscoverCommand);
        }

        private void OnDiscoverCommand()
        {
            TestAssembly();
            var lcm = LCM.LCM.LCM.Singleton;

            var discoverRequest = new DiscoveryRequest_t();
            lcm.Publish("PTZCAMERA", discoverRequest);
        }

        private void TestAssembly()
        {
            //var types =  typeof(AxXnsSdkDevice).Assembly.GetExportedTypes();
            //var attributes = typeof(AxXnsSdkDevice).Assembly.GetCustomAttributes(typeof(ComVisibleAttribute), false);
            var types =  typeof(XNSWindowctrlAx).Assembly.GetExportedTypes();
            var attributes = typeof(AxXNSSDKWINDOWLib.AxXnsSdkWindow).Assembly.GetCustomAttributes(typeof(ComVisibleAttribute), false);
        }
    }
}
