using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.Data.Xml.Dom;
using Windows.Devices.Haptics;
using Windows.Devices.Power;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Notifications;

namespace Luminal
{
    static class Bridge
    {
        static string Dir => Path.Combine(Engine.Root, "root", "var", "lib", "luminal", "req");

        public static void Start(CoreDispatcher ui)
        {
            Task.Run(async () =>
            {
                for (;;)
                {
                    try
                    {
                        if (Directory.Exists(Dir))
                            foreach (var req in Directory.GetFiles(Dir, "*.req"))
                                await Handle(req, ui);
                    }
                    catch (Exception) { }
                    await Task.Delay(250);
                }
            });
        }

        static async Task Handle(string req, CoreDispatcher ui)
        {
            string text;
            try { text = File.ReadAllText(req); File.Delete(req); }
            catch (Exception) { return; }
            int nl = text.IndexOf('\n');
            string cmd = nl < 0 ? text : text.Substring(0, nl), arg = nl < 0 ? "" : text.Substring(nl + 1);
            string reply;
            try { reply = "ok\n" + await Run(cmd, arg, ui); }
            catch (Exception e) { reply = "error\n" + e.Message + "\n"; }
            string res = Path.ChangeExtension(req, ".res"), tmp = res + ".tmp";
            File.WriteAllText(tmp, reply, new UTF8Encoding(false));
            File.Move(tmp, res);
        }

        static async Task<string> OnUi(CoreDispatcher ui, Func<Task<string>> f)
        {
            var done = new TaskCompletionSource<string>();
            await ui.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                try { done.SetResult(await f()); }
                catch (Exception e) { done.SetException(e); }
            });
            return await done.Task;
        }

        static async Task<string> Run(string cmd, string arg, CoreDispatcher ui)
        {
            switch (cmd)
            {
                case "open":
                    return await OnUi(ui, async () =>
                    {
                        if (!Uri.TryCreate(arg.Trim(), UriKind.Absolute, out var uri)) throw new Exception("that isn't a full link, like https://example.com");
                        if (!await Launcher.LaunchUriAsync(uri)) throw new Exception("no app opened it");
                        return "";
                    });
                case "copy":
                    return await OnUi(ui, () =>
                    {
                        var data = new DataPackage();
                        data.SetText(arg);
                        Clipboard.SetContent(data);
                        Clipboard.Flush();
                        return Task.FromResult("");
                    });
                case "paste":
                    return await OnUi(ui, async () =>
                    {
                        var view = Clipboard.GetContent();
                        if (!view.Contains(StandardDataFormats.Text)) return "";
                        return await view.GetTextAsync();
                    });
                case "notify":
                {
                    int nl = arg.IndexOf('\n');
                    string title = nl < 0 ? arg : arg.Substring(0, nl), body = nl < 0 ? "" : arg.Substring(nl + 1);
                    var xml = ToastNotificationManager.GetTemplateContent(ToastTemplateType.ToastText02);
                    var lines = xml.GetElementsByTagName("text");
                    lines[0].AppendChild(xml.CreateTextNode(title));
                    lines[1].AppendChild(xml.CreateTextNode(body));
                    ToastNotificationManager.CreateToastNotifier().Show(new ToastNotification(xml));
                    return "";
                }
                case "battery":
                {
                    var r = Battery.AggregateBattery.GetReport();
                    if (r.FullChargeCapacityInMilliwattHours == null || r.RemainingCapacityInMilliwattHours == null) return "no battery\n";
                    int pct = (int)Math.Round(100.0 * r.RemainingCapacityInMilliwattHours.Value / r.FullChargeCapacityInMilliwattHours.Value);
                    return pct + "% " + r.Status.ToString().ToLowerInvariant() + "\n";
                }
                case "vibrate":
                {
                    if (!int.TryParse(arg.Trim(), out int ms) || ms < 1) ms = 300;
                    if (await VibrationDevice.RequestAccessAsync() != VibrationAccessStatus.Allowed) throw new Exception("vibration isn't allowed");
                    var dev = await VibrationDevice.GetDefaultAsync();
                    if (dev == null) throw new Exception("no vibration motor found");
                    var c = dev.SimpleHapticsController;
                    SimpleHapticsControllerFeedback fb = null;
                    foreach (var f in c.SupportedFeedback)
                        if (f.Waveform == KnownSimpleHapticsControllerWaveforms.BuzzContinuous) fb = f;
                    if (fb == null) throw new Exception("the motor can't buzz on demand");
                    c.SendHapticFeedbackForDuration(fb, 1.0, TimeSpan.FromMilliseconds(Math.Min(ms, 5000)));
                    return "";
                }
            }
            throw new Exception("unknown command " + cmd);
        }
    }
}
