using Windows.ApplicationModel.Activation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace Luminal
{
    sealed partial class App : Application
    {
        public App()
        {
            InitializeComponent();
            UnhandledException += (s, e) =>
            {
                try
                {
                    System.IO.File.AppendAllText(System.IO.Path.Combine(Windows.Storage.ApplicationData.Current.LocalFolder.Path, "crash.log"),
                        System.DateTime.Now + " " + e.Exception + "\r\n");
                }
                catch (System.Exception) { }
            };
        }

        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            var frame = Window.Current.Content as Frame;
            if (frame == null)
            {
                frame = new Frame();
                Window.Current.Content = frame;
            }
            if (frame.Content == null) frame.Navigate(typeof(MainPage), e.Arguments);
            Window.Current.Activate();
        }
    }
}
