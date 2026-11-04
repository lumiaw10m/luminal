using System;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.Text;
using Microsoft.Graphics.Canvas.UI;
using Microsoft.Graphics.Canvas.UI.Xaml;
using Windows.ApplicationModel.DataTransfer;
using Windows.ApplicationModel.ExtendedExecution;
using Windows.Foundation;
using Windows.Media.Core;
using Windows.Media.Playback;
using Windows.Storage;
using Windows.System;
using Windows.UI;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Luminal
{
    public sealed partial class MainPage : Page
    {
        const string FontPath = "ms-appx:///Assets/DejaVuSansMono.ttf#DejaVu Sans Mono";
        const string BoldFontPath = "ms-appx:///Assets/DejaVuSansMono-Bold.ttf#DejaVu Sans Mono";
        static uint DefaultFg = 0xFFD0D0D0, DefaultBg = 0xFF000000;
        static readonly string[] SchemeNames = { "Dark", "Light", "Green" };
        static readonly uint[] SchemeFg = { 0xFFD0D0D0, 0xFF202020, 0xFF0DBD8B };
        static readonly uint[] SchemeBg = { 0xFF000000, 0xFFF4F4F4, 0xFF000000 };
        static readonly Color Accent = Color.FromArgb(255, 0x0D, 0xBD, 0x8B);
        const string Sentinel = " ";

        sealed class Session
        {
            public int Id;
            public Vt Vt;
            public int ScrollOffset;
            public string Title;
            public long Output;
            public long OutputAtResize;
        }

        const int MaxTabs = 6;
        readonly System.Collections.Generic.List<Session> _sessions = new System.Collections.Generic.List<Session>();
        Session _active;
        Vt _vt = new Vt(24, 60);
        bool _readerStarted;
        float _fontSize = 9.5f;
        float _cellW = 7, _cellH = 14;
        CanvasTextFormat _format, _boldFormat;
        int _scrollOffset;
        double _scrollAccum, _scaleAccum = 1;
        bool _ctrl, _alt, _editing, _sessionRunning, _started, _exited;
        DispatcherTimer _frame, _resizeTimer;
        int _resizeRepeats;
        bool _selecting, _selDragging, _selMoved, _hasSel;
        int _selStartLine, _selStartCol, _selEndLine, _selEndCol;
        ExtendedExecutionSession _extended;
        string _launchArgs;
        Windows.System.Display.DisplayRequest _displayRequest;
        DispatcherTimer _saverTimer;
        bool _saver;
        float _dotX, _dotY;
        int _backgroundId;
        readonly Random _rnd = new Random();

        readonly StringBuilder _testLog = new StringBuilder();

        public MainPage()
        {
            InitializeComponent();

            _fontSize = (float)(ApplicationData.Current.LocalSettings.Values["fontSize"] as double? ?? 9.5);
            ApplyScheme(ApplicationData.Current.LocalSettings.Values["scheme"] as int? ?? 0, false);
            var pane = InputPane.GetForCurrentView();
            pane.Showing += (s, a) =>
            {
                a.EnsuredFocusedElementInView = true;
                KeyboardSpacer.Height = new GridLength(a.OccludedRect.Height);
            };
            pane.Hiding += (s, a) =>
            {
                a.EnsuredFocusedElementInView = true;
                KeyboardSpacer.Height = new GridLength(0);
            };
            Input.Text = Sentinel;
            Input.SelectionStart = 1;
            Input.GotFocus += (s, a) => Input.SelectionStart = Input.Text.Length;
            _frame =new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(33) };
            _frame.Tick += (s, a) =>
            {
                if (_vt.Dirty)
                {
                    _vt.Dirty = false;
                    Canvas.Invalidate();
                }
                foreach (var s2 in _sessions)
                    if (s2.Title != s2.Vt.Title)
                    {
                        s2.Title = s2.Vt.Title;
                        if (!_saver) RebuildTabs();
                        break;
                    }
            };
            _frame.Start();
            Unloaded += (s, a) => Canvas.RemoveFromVisualTree();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            _launchArgs = e.Parameter as string;
            if (_launchArgs == "awake") EnterSaver();
        }

        void ApplyScheme(int i, bool save)
        {
            if (i < 0 || i >= SchemeNames.Length) i = 0;
            DefaultFg = SchemeFg[i];
            DefaultBg = SchemeBg[i];
            Canvas.ClearColor = _saver ? Colors.Black : ToColor(DefaultBg);
            if (save) ApplicationData.Current.LocalSettings.Values["scheme"] = i;
            _vt.Dirty = true;
            Canvas.Invalidate();
        }

        void ApplyFontSize(float size)
        {
            _fontSize = Math.Max(6, Math.Min(28, size));
            ApplicationData.Current.LocalSettings.Values["fontSize"] = (double)_fontSize;
            MakeFormats(Canvas);
            UpdateGrid();
            Canvas.Invalidate();
        }

        void EnterSaver()
        {
            if (_saver) return;
            _saver = true;
            Canvas.ClearColor = Colors.Black;
            if (_displayRequest == null) _displayRequest = new Windows.System.Display.DisplayRequest();
            _displayRequest.RequestActive();
            InputPane.GetForCurrentView().TryHide();
            KeyRow.Visibility = Visibility.Collapsed;
            TabStrip.Visibility = Visibility.Collapsed;
            ApplicationView.GetForCurrentView().TryEnterFullScreenMode();
            MoveDot();
            _saverTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(20) };
            _saverTimer.Tick += (s, a) => MoveDot();
            _saverTimer.Start();
        }

        void ExitSaver()
        {
            if (!_saver) return;
            _saver = false;
            Canvas.ClearColor = ToColor(DefaultBg);
            _saverTimer.Stop();
            _displayRequest.RequestRelease();
            KeyRow.Visibility = Visibility.Visible;
            TabStrip.Visibility = Visibility.Visible;
            ApplicationView.GetForCurrentView().ExitFullScreenMode();
            UpdateGrid();
            _vt.Dirty = true;
            Canvas.Invalidate();
        }

        void MoveDot()
        {
            _dotX = (float)(_rnd.NextDouble() * Canvas.ActualWidth);
            _dotY = (float)(_rnd.NextDouble() * Canvas.ActualHeight);
            Canvas.Invalidate();
        }

        async Task StartBackground()
        {
            string setup = "";
            try
            {
                File.Copy(Path.Combine(Windows.ApplicationModel.Package.Current.InstalledLocation.Path, "luminal"), Path.Combine(Engine.Root, "root", "tmp", ".luminal-new"), true);
                setup = "install -m 755 /tmp/.luminal-new /usr/bin/luminal; rm -f /tmp/.luminal-new; ";
            }
            catch (Exception) { }
            byte[] c = Encoding.UTF8.GetBytes(setup + "[ -x /etc/luminal/autostart ] && exec /etc/luminal/autostart\0");
            int id = await Task.Run(() => Engine.tx_session_open(c, 24, 80));
            if (id > 0) _backgroundId = id;
        }

        async void StartSession()
        {
            _started = true;
            _exited = false;
            string err = await Task.Run(() => Engine.Prepare());
            if (err != null)
            {
                Print("\r\n" + err + "\r\n");
                return;
            }
            string cmd = null;
            if (_launchArgs == "test")
                cmd = File.ReadAllText(Path.Combine(ApplicationData.Current.LocalFolder.Path, "testcmd.txt")).Trim();
            await OpenTab(cmd);
            if (_launchArgs != "test") await StartBackground();
            RequestBackgroundTime();
            Bridge.Start(Dispatcher);
            Application.Current.EnteredBackground += (s, a) => StartSilence();
            Application.Current.LeavingBackground += (s, a) => StopSilence();
            if (_launchArgs == "auto") { var t = AutoType(); }
        }

        MediaPlayer _silence;

        void StartSilence()
        {
            string log = Path.Combine(ApplicationData.Current.LocalFolder.Path, "ext.log");
            try
            {
                if (_silence == null)
                {
                    const int rate = 8000, seconds = 2, bytes = rate * seconds * 2;
                    var wav = new byte[44 + bytes];
                    var w = new BinaryWriter(new MemoryStream(wav));
                    w.Write(Encoding.ASCII.GetBytes("RIFF")); w.Write(36 + bytes); w.Write(Encoding.ASCII.GetBytes("WAVEfmt "));
                    w.Write(16); w.Write((short)1); w.Write((short)1); w.Write(rate); w.Write(rate * 2); w.Write((short)2); w.Write((short)16);
                    w.Write(Encoding.ASCII.GetBytes("data")); w.Write(bytes);
                    var stream = new Windows.Storage.Streams.InMemoryRandomAccessStream();
                    var dw = new Windows.Storage.Streams.DataWriter(stream);
                    dw.WriteBytes(wav);
                    dw.StoreAsync().AsTask().Wait();
                    dw.DetachStream();
                    stream.Seek(0);
                    _silence = new MediaPlayer { IsLoopingEnabled = true, AudioCategory = MediaPlayerAudioCategory.Media };
                    _silence.CommandManager.IsEnabled = false;
                    _silence.Source = MediaSource.CreateFromStream(stream, "audio/wav");
                }
                _silence.Play();
                File.AppendAllText(log, DateTime.Now + " silence playing\r\n");
            }
            catch (Exception e) { File.AppendAllText(log, DateTime.Now + " silence failed " + e.Message + "\r\n"); }
        }

        void StopSilence()
        {
            try { _silence?.Pause(); } catch (Exception) { }
        }

        async Task OpenTab(string cmd)
        {
            if (_sessions.Count >= MaxTabs) return;
            int rows = _vt.Rows, cols = _vt.Cols;
            byte[] c = cmd != null ? Encoding.UTF8.GetBytes(cmd + "\0") : null;
            int id = await Task.Run(() => Engine.tx_session_open(c, rows, cols));
            if (id <= 0)
            {
                Print("\r\nCouldn't start the shell (" + id + ").\r\n");
                return;
            }
            var s = new Session { Id = id, Vt = new Vt(rows, cols) };
            s.Vt.Reply = b => Engine.tx_session_write(id, b, b.Length);
            lock (_sessions) _sessions.Add(s);
            Activate(s);
            _sessionRunning = true;
            if (!_readerStarted)
            {
                _readerStarted = true;
                var loop = Task.Factory.StartNew(ReadLoop, TaskCreationOptions.LongRunning);
            }
        }

        void Activate(Session s)
        {
            if (_active != null) _active.ScrollOffset = _scrollOffset;
            _active = s;
            _vt = s.Vt;
            _scrollOffset = s.ScrollOffset;
            _vt.Dirty = true;
            RebuildTabs();
            Canvas.Invalidate();
        }

        void CloseTab(Session s)
        {
            Engine.tx_session_close(s.Id);
            lock (_sessions) _sessions.Remove(s);
            TabGone(s);
        }

        void TabGone(Session s)
        {
            if (_active == s)
            {
                _active = null;
                if (_sessions.Count > 0) Activate(_sessions[_sessions.Count - 1]);
                else
                {
                    _sessionRunning = false;
                    _exited = true;
                    Print("\r\n\x1b[2m[no shells running. Tap to start a new one.]\x1b[0m\r\n");
                }
            }
            RebuildTabs();
        }

        void RebuildTabs()
        {
            TabStrip.Children.Clear();
            int n = 1;
            foreach (var s in _sessions)
            {
                var b = TabButton(TabLabel(s, n), s == _active);
                var captured = s;
                b.Click += (o, a) => Activate(captured);
                b.Holding += (o, a) => { if (a.HoldingState == Windows.UI.Input.HoldingState.Started) CloseTab(captured); };
                b.RightTapped += (o, a) => CloseTab(captured);
                TabStrip.Children.Add(b);
                n++;
            }
            if (_sessions.Count < MaxTabs)
            {
                var plus = TabButton("+", false);
                plus.Click += async (o, a) => await OpenTab(null);
                TabStrip.Children.Add(plus);
            }
        }

        static string TabLabel(Session s, int n)
        {
            string title = s.Vt.Title;
            if (string.IsNullOrEmpty(title)) return n.ToString();
            int slash = title.TrimEnd('/').LastIndexOf('/');
            return slash >= 0 && title.Length > 1 ? title.TrimEnd('/').Substring(slash + 1) : title;
        }

        Button TabButton(string text, bool active)
        {
            return new Button
            {
                Content = new TextBlock { Text = text, TextTrimming = TextTrimming.CharacterEllipsis, Margin = new Thickness(6, 0, 6, 0), MaxWidth = 76 },
                Height = 30,
                MinWidth = 44,
                MinHeight = 0,
                Padding = new Thickness(0),
                FontSize = 13,
                BorderThickness = new Thickness(0),
                IsTabStop = false,
                AllowFocusOnInteraction = false,
                Foreground = new SolidColorBrush(active ? Colors.Black : Color.FromArgb(255, 150, 150, 150)),
                Background = new SolidColorBrush(active ? Accent : Colors.Transparent),
            };
        }

        void Print(string s)
        {
            var b = Encoding.UTF8.GetBytes(s);
            lock (_vt.Lock) _vt.Feed(b, b.Length);
        }

        void ReadLoop()
        {
            var buf = new byte[16384];
            string log = _launchArgs == "test" ? Path.Combine(ApplicationData.Current.LocalFolder.Path, "test.log") : null;
            for (;;)
            {
                Session[] all;
                lock (_sessions) all = _sessions.ToArray();
                bool any = false;
                foreach (var s in all)
                {
                    int n = Engine.tx_session_read(s.Id, buf, buf.Length);
                    if (n > 0)
                    {
                        any = true;
                        s.Output += n;
                        lock (s.Vt.Lock) s.Vt.Feed(buf, n);
                        if (log != null)
                        {
                            _testLog.Append(Encoding.UTF8.GetString(buf, 0, n));
                            File.WriteAllText(log, _testLog.ToString());
                        }
                    }
                    else if (n < 0)
                    {
                        int status = Engine.tx_session_status(s.Id);
                        Engine.tx_session_close(s.Id);
                        lock (_sessions) _sessions.Remove(s);
                        if (log != null)
                        {
                            _testLog.AppendLine("exit status: " + status).AppendLine("DONE");
                            File.WriteAllText(log, _testLog.ToString());
                        }
                        var ignored = Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                        {
                            if (_launchArgs == "test") Application.Current.Exit();
                            else TabGone(s);
                        });
                    }
                }
                int bg = _backgroundId;
                if (bg > 0)
                {
                    int n = Engine.tx_session_read(bg, buf, buf.Length);
                    if (n > 0) any = true;
                    else if (n < 0)
                    {
                        Engine.tx_session_close(bg);
                        _backgroundId = 0;
                    }
                }
                if (!any) Engine.tx_wait(200);
            }
        }

        async void RequestBackgroundTime()
        {

            try
            {

                string log = Path.Combine(ApplicationData.Current.LocalFolder.Path, "ext.log");
                _extended = new ExtendedExecutionSession { Reason = ExtendedExecutionReason.LocationTracking, Description = "Running shell" };
                _extended.Revoked += (s, a) => { File.AppendAllText(log, DateTime.Now + " revoked " + a.Reason + "\r\n"); _extended = null; };
                var result = await _extended.RequestExtensionAsync();
                File.AppendAllText(log, DateTime.Now + " location: " + result + "\r\n");
                if (result != ExtendedExecutionResult.Allowed)
                {
                    _extended.Dispose();
                    _extended = new ExtendedExecutionSession { Reason = ExtendedExecutionReason.Unspecified, Description = "Running shell" };
                    _extended.Revoked += (s, a) => { File.AppendAllText(log, DateTime.Now + " revoked " + a.Reason + "\r\n"); _extended = null; };
                    result = await _extended.RequestExtensionAsync();
                    File.AppendAllText(log, DateTime.Now + " unspecified: " + result + "\r\n");
                }
            }
            catch (Exception) { }
        }

        async Task AutoType()
        {
            string path = Path.Combine(ApplicationData.Current.LocalFolder.Path, "auto.txt");
            if (!File.Exists(path)) return;
            await Task.Delay(1500);
            foreach (var raw in File.ReadAllLines(path))
            {
                if (raw.StartsWith("#wait ")) { await Task.Delay(int.Parse(raw.Substring(6))); continue; }
                if (raw == "#keys") { Input.Focus(FocusState.Programmatic); continue; }
                if (raw == "#hide") { InputPane.GetForCurrentView().TryHide(); continue; }
                if (raw == "#newtab") { await OpenTab(null); continue; }
                if (raw.StartsWith("#tab ")) { Activate(_sessions[int.Parse(raw.Substring(5)) - 1]); continue; }
                if (raw.StartsWith("#copylink ")) { var v = raw.Split(' '); string got = LinkAt(new Point((int.Parse(v[2]) + 0.5) * _cellW, (int.Parse(v[1]) + 0.5) * _cellH)); SetClipboard(got ?? "NO-LINK"); continue; }
                if (raw.StartsWith("#scheme ")) { ApplyScheme(int.Parse(raw.Substring(8)), true); continue; }
                if (raw.StartsWith("#font ")) { ApplyFontSize(_fontSize + float.Parse(raw.Substring(6))); continue; }
                if (raw == "#saver") { EnterSaver(); continue; }
                if (raw == "#nosaver") { ExitSaver(); continue; }
                if (raw == "#closetab") { if (_active != null) CloseTab(_active); continue; }
                if (raw.StartsWith("#select ")) { AutoSelect(raw.Substring(8)); continue; }
                if (raw == "#selectmode") { StartSelect(); continue; }
                if (raw == "#endselect") { EndSelect(); continue; }
                if (raw == "#paste") { await PasteClipboard(); continue; }
                string line = raw.Replace("\\e", "\x1b").Replace("\\r", "\r");
                var b = Encoding.UTF8.GetBytes(line);
                if (_active != null) Engine.tx_session_write(_active.Id, b, b.Length);
                await Task.Delay(300);
            }
        }

        void Canvas_CreateResources(CanvasControl sender, CanvasCreateResourcesEventArgs args)
        {
            MakeFormats(sender);
            UpdateGrid();
            if (!_started) StartSession();
        }

        void MakeFormats(ICanvasResourceCreator device)
        {
            _format = new CanvasTextFormat { FontFamily = FontPath, FontSize = _fontSize, WordWrapping = CanvasWordWrapping.NoWrap };
            _boldFormat = new CanvasTextFormat { FontFamily = BoldFontPath, FontSize = _fontSize, FontWeight = Windows.UI.Text.FontWeights.Bold, WordWrapping = CanvasWordWrapping.NoWrap };
            using (var layout = new CanvasTextLayout(device, "MMMMMMMMMM", _format, 2000, 200))
            {
                _cellW = (float)layout.LayoutBounds.Width / 10f;
                _cellH = (float)Math.Ceiling(layout.LayoutBounds.Height);
            }
        }

        void UpdateGrid()
        {
            if (_format == null || Canvas.ActualWidth <= 0 || Canvas.ActualHeight <= 0) return;
            int cols = Math.Max(10, (int)(Canvas.ActualWidth / _cellW));
            int rows = Math.Max(3, (int)(Canvas.ActualHeight / _cellH));
            lock (_vt.Lock) _vt.Resize(rows, cols);
            Session[] all;
            lock (_sessions) all = _sessions.ToArray();
            foreach (var s in all)
            {
                lock (s.Vt.Lock) s.Vt.Resize(rows, cols);
                s.OutputAtResize = s.Output;
                Engine.tx_session_resize(s.Id, rows, cols);
            }
            Canvas.Invalidate();
            RepeatResize();
        }

        void RepeatResize()
        {
            _resizeRepeats = 2;
            if (_resizeTimer == null)
            {
                _resizeTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(700) };
                _resizeTimer.Tick += (s, a) =>
                {
                    Session[] all;
                    lock (_sessions) all = _sessions.ToArray();
                    foreach (var t in all) Engine.tx_session_resize(t.Id, t.Vt.Rows, t.Vt.Cols);
                    if (--_resizeRepeats > 0) return;
                    _resizeTimer.Stop();

                    foreach (var t in all)
                        if (t.Vt.OnAltScreen && t.Output == t.OutputAtResize) Engine.tx_session_write(t.Id, new byte[] { 12, 12 }, 2);
                };
            }
            _resizeTimer.Stop();
            _resizeTimer.Start();
        }

        void Canvas_SizeChanged(object sender, SizeChangedEventArgs e) { if (!_saver) UpdateGrid(); }

        static Color ToColor(uint v) => Color.FromArgb(255, (byte)(v >> 16), (byte)(v >> 8), (byte)v);

        void Canvas_Draw(CanvasControl sender, CanvasDrawEventArgs args)
        {
            var ds = args.DrawingSession;
            if (_saver)
            {
                ds.FillCircle(_dotX, _dotY, 2, Color.FromArgb(255, 45, 45, 45));
                return;
            }
            var text = new StringBuilder();
            lock (_vt.Lock)
            {
                int rows = _vt.Rows, cols = _vt.Cols;
                int offset = Math.Min(_scrollOffset, _vt.ScrollbackCount);
                for (int r = 0; r < rows; r++)
                {
                    var line = _vt.Line(r - offset);
                    if (line == null) continue;
                    float y = r * _cellH;
                    int n = Math.Min(cols, line.Length);
                    int c = 0;
                    while (c < n)
                    {

                        int start = c;
                        uint fg, bg;
                        Resolve(ref line[c], out fg, out bg);
                        byte attr = line[c].Attr;
                        text.Clear();
                        bool any = false;
                        for (; c < n; c++)
                        {
                            uint f2, b2;
                            Resolve(ref line[c], out f2, out b2);
                            if (f2 != fg || b2 != bg || ((line[c].Attr ^ attr) & (Cell.Bold | Cell.Underline)) != 0) break;
                            int ch = line[c].Ch;
                            if (ch == -1) continue;
                            if (ch > 0xFFFF || line[c].Marks != null)
                            {

                                if (text.Length > 0) break;
                                text.Append(char.ConvertFromUtf32(ch)).Append(line[c].Marks);
                                any = true;
                                c++;
                                if (c < n && line[c].Ch == -1) c++;
                                break;
                            }
                            if (ch != 0 && ch != ' ') any = true;
                            text.Append(ch == 0 ? ' ' : (char)ch);
                        }
                        if (c == start) c++;
                        float x = start * _cellW;
                        float w = (c - start) * _cellW;
                        if (bg != DefaultBg)
                        {

                            float fw = c >= n ? Math.Max(w, (float)sender.ActualWidth - x) : w;
                            float fh = r == rows - 1 ? Math.Max(_cellH, (float)sender.ActualHeight - y) : _cellH;
                            ds.FillRectangle(x, y, fw, fh, ToColor(bg));
                        }
                        if (any)
                            ds.DrawText(text.ToString(), x, y, ToColor(fg), (attr & Cell.Bold) != 0 ? _boldFormat : _format);
                        if ((attr & Cell.Underline) != 0) ds.DrawLine(x, y + _cellH - 1, x + w, y + _cellH - 1, ToColor(fg));
                    }
                }
                if (_vt.CursorVisible && offset == 0)
                {
                    float cx = _vt.CursorCol * _cellW, cy = _vt.CursorRow * _cellH;
                    ds.FillRectangle(cx, cy, _cellW, _cellH, Color.FromArgb(150, (byte)(DefaultFg >> 16), (byte)(DefaultFg >> 8), (byte)DefaultFg));
                }
                if (_hasSel) DrawSelection(ds, rows, cols, offset);
                if (_selecting && !_hasSel)
                {
                    float y = (rows - 1) * _cellH;
                    ds.FillRectangle(0, y, (float)sender.ActualWidth, _cellH, Color.FromArgb(255, 0, 90, 158));
                    ds.DrawText("Drag over the text to select it", 0, y, Colors.White, _format);
                }
            }
        }

        static void Resolve(ref Cell cell, out uint fg, out uint bg)
        {
            fg = cell.Fg != 0 ? cell.Fg : DefaultFg;
            bg = cell.Bg != 0 ? cell.Bg : DefaultBg;
            if ((cell.Attr & Cell.Dim) != 0)
                fg = 0xFF000000 | (((fg >> 16) & 255) * 2 / 3) << 16 | (((fg >> 8) & 255) * 2 / 3) << 8 | ((fg & 255) * 2 / 3);
            if ((cell.Attr & Cell.Inverse) != 0)
            {
                uint t = fg;
                fg = bg;
                bg = t;
            }
        }

        void Canvas_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (_saver)
            {
                ExitSaver();
                return;
            }
            if (_selecting) return;
            if (_exited && !_sessionRunning)
            {
                _launchArgs = null;
                _exited = false;
                var ignored = OpenTab(null);
            }
            _scrollOffset = 0;
            Input.Focus(FocusState.Programmatic);
            InputPane.GetForCurrentView().TryShow();
            Canvas.Invalidate();
        }

        void Canvas_ManipulationDelta(object sender, ManipulationDeltaRoutedEventArgs e)
        {
            if (Math.Abs(e.Delta.Scale - 1) > 0.001)
            {
                _scaleAccum *= e.Delta.Scale;
                return;
            }
            _scrollAccum += e.Delta.Translation.Y;
            int lines = (int)(_scrollAccum / _cellH);
            if (lines == 0) return;
            _scrollAccum -= lines * _cellH;
            bool alt;
            lock (_vt.Lock)
            {
                alt = _vt.OnAltScreen;
                if (!alt) _scrollOffset = Math.Max(0, Math.Min(_vt.ScrollbackCount, _scrollOffset + lines));
            }
            if (alt)
            {

                string key = lines > 0 ? "up" : "down";
                for (int i = Math.Abs(lines); i > 0; i--) SendKey(key);
            }
            Canvas.Invalidate();
        }

        void Canvas_ManipulationCompleted(object sender, ManipulationCompletedRoutedEventArgs e)
        {
            if (Math.Abs(_scaleAccum - 1) > 0.05)
            {
                ApplyFontSize((float)(Math.Round(_fontSize * _scaleAccum * 2) / 2));
            }
            _scaleAccum = 1;
            _scrollAccum = 0;
        }

        void Canvas_Holding(object sender, HoldingRoutedEventArgs e)
        {
            if (!_saver && !_selecting && e.HoldingState == Windows.UI.Input.HoldingState.Started) ShowMenu(e.GetPosition(Canvas));
        }

        void Canvas_RightTapped(object sender, RightTappedRoutedEventArgs e)
        {
            if (!_saver && !_selecting) ShowMenu(e.GetPosition(Canvas));
        }

        void ShowMenu(Point at)
        {
            var menu = new MenuFlyout();
            string link = LinkAt(at);
            if (link != null)
            {
                var copyLink = new MenuFlyoutItem { Text = "Copy link" };
                copyLink.Click += (s, a) => SetClipboard(link);
                menu.Items.Add(copyLink);
            }
            var paste = new MenuFlyoutItem { Text = "Paste" };
            paste.Click += async (s, a) => await PasteClipboard();
            var select = new MenuFlyoutItem { Text = "Select text" };
            select.Click += (s, a) => StartSelect();
            var copy = new MenuFlyoutItem { Text = "Copy screen" };
            copy.Click += (s, a) =>
            {
                SetClipboard(ScreenText());
            };
            var size = new MenuFlyoutSubItem { Text = "Text size" };
            foreach (var step in new[] { 1f, -1f })
            {
                var item = new MenuFlyoutItem { Text = step > 0 ? "Larger" : "Smaller" };
                item.Click += (s, a) => ApplyFontSize(_fontSize + step);
                size.Items.Add(item);
            }
            var colours = new MenuFlyoutSubItem { Text = "Colours" };
            for (int c = 0; c < SchemeNames.Length; c++)
            {
                int scheme = c;
                var item = new MenuFlyoutItem { Text = SchemeNames[c] };
                item.Click += (s, a) => ApplyScheme(scheme, true);
                colours.Items.Add(item);
            }
            var awake = new MenuFlyoutItem { Text = "Keep awake" };
            awake.Click += (s, a) => EnterSaver();
            menu.Items.Add(paste);
            menu.Items.Add(select);
            menu.Items.Add(copy);
            menu.Items.Add(size);
            menu.Items.Add(colours);
            menu.Items.Add(awake);
            menu.ShowAt(Canvas, at);
        }

        async Task PasteClipboard()
        {
            var content = Clipboard.GetContent();
            if (!content.Contains(StandardDataFormats.Text)) return;
            string t = await content.GetTextAsync();
            t = t.Replace("\r\n", "\r").Replace('\n', '\r');
            if (_vt.BracketedPaste) t = "\x1b[200~" + t + "\x1b[201~";
            SendText(t);
        }

        void StartSelect()
        {
            _selecting = true;
            _hasSel = false;
            Canvas.ManipulationMode = ManipulationModes.None;
            Canvas.Invalidate();
        }

        void EndSelect()
        {
            _selecting = _selDragging = _hasSel = false;
            Canvas.ManipulationMode = ManipulationModes.TranslateY | ManipulationModes.Scale;
            Canvas.Invalidate();
        }

        void CellAt(Point p, out int line, out int col)
        {
            lock (_vt.Lock)
            {
                int row = Math.Max(0, Math.Min(_vt.Rows - 1, (int)(p.Y / _cellH)));
                col = Math.Max(0, Math.Min(_vt.Cols - 1, (int)(p.X / _cellW)));
                line = _vt.ScrollbackCount + row - Math.Min(_scrollOffset, _vt.ScrollbackCount);
            }
        }

        void Canvas_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            if (!_selecting) return;
            CellAt(e.GetCurrentPoint(Canvas).Position, out _selStartLine, out _selStartCol);
            _selEndLine = _selStartLine;
            _selEndCol = _selStartCol;
            _selDragging = _hasSel = true;
            _selMoved = false;
            Canvas.CapturePointer(e.Pointer);
            Canvas.Invalidate();
            e.Handled = true;
        }

        void Canvas_PointerMoved(object sender, PointerRoutedEventArgs e)
        {
            if (!_selDragging) return;
            int line, col;
            var pos = e.GetCurrentPoint(Canvas).Position;

            lock (_vt.Lock)
            {
                if (pos.Y < _cellH && _scrollOffset < _vt.ScrollbackCount) _scrollOffset++;
                else if (pos.Y > Canvas.ActualHeight - _cellH && _scrollOffset > 0) _scrollOffset--;
            }
            CellAt(pos, out line, out col);
            if (line == _selEndLine && col == _selEndCol) return;
            _selEndLine = line;
            _selEndCol = col;
            _selMoved = true;
            Canvas.Invalidate();
            e.Handled = true;
        }

        void Canvas_PointerReleased(object sender, PointerRoutedEventArgs e)
        {
            if (!_selDragging) return;
            _selDragging = false;
            Canvas.ReleasePointerCapture(e.Pointer);
            if (!_selMoved) SelectWord();
            Canvas.Invalidate();
            ShowSelectionMenu(e.GetCurrentPoint(Canvas).Position);
            e.Handled = true;
        }

        void ShowSelectionMenu(Point at)
        {
            string text = SelectedText();
            var menu = new MenuFlyout();
            var copy = new MenuFlyoutItem { Text = "Copy" };
            copy.Click += (s, a) => SetClipboard(text);
            menu.Items.Add(copy);
            menu.Items.Add(new MenuFlyoutItem { Text = "Cancel" });
            menu.Closed += (s, a) => EndSelect();
            menu.ShowAt(Canvas, at);
        }

        static void SetClipboard(string text)
        {
            var dp = new DataPackage();
            dp.SetText(text);
            Clipboard.SetContent(dp);
        }

        void AutoSelect(string args)
        {
            var v = args.Split(' ');
            lock (_vt.Lock)
            {
                _selStartLine = _vt.ScrollbackCount + int.Parse(v[0]);
                _selEndLine = _vt.ScrollbackCount + int.Parse(v[2]);
            }
            _selStartCol = int.Parse(v[1]);
            _selEndCol = int.Parse(v[3]);
            _hasSel = true;
            SetClipboard(SelectedText());
            Canvas.Invalidate();
        }

        void SelectWord()
        {
            lock (_vt.Lock)
            {
                var line = _vt.Line(_selStartLine - _vt.ScrollbackCount);
                if (line == null || _selStartCol >= line.Length || IsBlank(line[_selStartCol].Ch)) return;
                int l1, c1, l2, c2;
                WordExtent(_selStartLine, _selStartCol, out l1, out c1, out l2, out c2);
                _selStartLine = l1;
                _selStartCol = c1;
                _selEndLine = l2;
                _selEndCol = c2;
            }
        }

        void OrderedSelection(out int l1, out int c1, out int l2, out int c2)
        {
            bool swap = _selStartLine > _selEndLine || (_selStartLine == _selEndLine && _selStartCol > _selEndCol);
            l1 = swap ? _selEndLine : _selStartLine;
            c1 = swap ? _selEndCol : _selStartCol;
            l2 = swap ? _selStartLine : _selEndLine;
            c2 = swap ? _selStartCol : _selEndCol;
        }

        void DrawSelection(CanvasDrawingSession ds, int rows, int cols, int offset)
        {
            int l1, c1, l2, c2;
            OrderedSelection(out l1, out c1, out l2, out c2);
            int top = _vt.ScrollbackCount - offset;
            for (int l = Math.Max(l1, top); l <= l2 && l < top + rows; l++)
            {
                int from = l == l1 ? c1 : 0, to = l == l2 ? c2 : cols - 1;
                ds.FillRectangle(from * _cellW, (l - top) * _cellH, (to - from + 1) * _cellW, _cellH, Color.FromArgb(110, 60, 150, 255));
            }
        }

        string SelectedText()
        {
            int l1, c1, l2, c2;
            OrderedSelection(out l1, out c1, out l2, out c2);
            return LinesText(l1, c1, l2, c2);
        }

        string ScreenText()
        {
            lock (_vt.Lock)
            {
                int top = _vt.ScrollbackCount - Math.Min(_scrollOffset, _vt.ScrollbackCount);
                return LinesText(top, 0, top + _vt.Rows - 1, _vt.Cols - 1).TrimEnd() + "\r\n";
            }
        }

        string LinesText(int l1, int c1, int l2, int c2)
        {
            var sb = new StringBuilder();
            lock (_vt.Lock)
            {
                bool inUrl = false, skipIndent = false;
                for (int l = l1; l <= l2; l++)
                {
                    var line = _vt.Line(l - _vt.ScrollbackCount);
                    if (line == null) continue;
                    int from = l == l1 ? c1 : 0, to = Math.Min(l == l2 ? c2 : line.Length - 1, line.Length - 1);
                    if (skipIndent) while (from < to && IsBlank(line[from].Ch)) from++;
                    var part = new StringBuilder();
                    for (int c = from; c <= to; c++)
                    {
                        if (line[c].Ch == -1) continue;
                        part.Append(line[c].Ch <= 0 ? " " : char.ConvertFromUtf32(line[c].Ch)).Append(line[c].Marks);
                    }
                    bool wrapped = (line[line.Length - 1].Attr & Cell.Wrapped) != 0;
                    var next = l < l2 ? _vt.Line(l + 1 - _vt.ScrollbackCount) : null;
                    bool joinUrl = !wrapped && next != null && UrlContinues(line, next, ref inUrl);
                    if (wrapped && l < l2) sb.Append(part);
                    else sb.Append(part.ToString().TrimEnd());
                    if (l < l2 && !wrapped && !joinUrl) sb.Append("\r\n");
                    skipIndent = joinUrl;
                    if (!joinUrl && !wrapped) inUrl = false;
                }
            }
            return sb.ToString();
        }

        static bool IsBlank(int ch) => ch == 0 || ch == ' ';

        static bool IsUrlChar(int ch) => ch > 32 && ch < 127 && ch != '"' && ch != '<' && ch != '>' && ch != '`' && ch != '|';

        static int LastUsed(Cell[] line)
        {
            int c = line.Length - 1;
            while (c >= 0 && IsBlank(line[c].Ch)) c--;
            return c;
        }

        static string LastWord(Cell[] line)
        {
            int e = LastUsed(line), s = e;
            if (e < 0) return "";
            while (s > 0 && IsUrlChar(line[s - 1].Ch)) s--;
            var w = new StringBuilder();
            for (int c = s; c <= e; c++) if (line[c].Ch > 0) w.Append((char)line[c].Ch);
            return w.ToString();
        }

        static bool UrlContinues(Cell[] line, Cell[] next, ref bool inUrl)
        {
            string word = LastWord(line);
            if (!inUrl && word.IndexOf("://", StringComparison.Ordinal) < 0) return false;
            if (LastUsed(line) < line.Length - 8) return false;
            int s = 0, end = LastUsed(next);
            while (s <= end && IsBlank(next[s].Ch)) s++;
            if (s > end) return false;
            for (int c = s; c <= end; c++) if (!IsUrlChar(next[c].Ch)) return false;
            if (LastWord(next).IndexOf("://", StringComparison.Ordinal) >= 0) return false;
            inUrl = true;
            return true;
        }

        void WordExtent(int line, int col, out int l1, out int c1, out int l2, out int c2)
        {
            l1 = l2 = line;
            c1 = c2 = col;
            var cur = _vt.Line(line - _vt.ScrollbackCount);
            if (cur == null || col >= cur.Length) return;
            while (c1 > 0 && (IsUrlChar(cur[c1 - 1].Ch) || cur[c1 - 1].Ch == -1)) c1--;
            while (c2 < cur.Length - 1 && (IsUrlChar(cur[c2 + 1].Ch) || cur[c2 + 1].Ch == -1)) c2++;

            for (;;)
            {
                var prev = _vt.Line(l1 - 1 - _vt.ScrollbackCount);
                var here = _vt.Line(l1 - _vt.ScrollbackCount);
                if (prev == null || c1 > 0 && !IsBlank(here[c1 - 1].Ch)) break;
                int first = 0;
                while (first < c1 && IsBlank(here[first].Ch)) first++;
                if (first != c1) break;
                bool wrapped = (prev[prev.Length - 1].Attr & Cell.Wrapped) != 0, url = true;
                if (!wrapped && !UrlContinues(prev, here, ref url)) break;
                l1--;
                c1 = LastUsed(prev);
                while (c1 > 0 && IsUrlChar(prev[c1 - 1].Ch)) c1--;
            }

            bool inUrl = l1 != line;
            for (;;)
            {
                var here = _vt.Line(l2 - _vt.ScrollbackCount);
                var next = _vt.Line(l2 + 1 - _vt.ScrollbackCount >= _vt.Rows ? int.MaxValue / 2 : l2 + 1 - _vt.ScrollbackCount);
                if (next == null || c2 < LastUsed(here)) break;
                bool wrapped = (here[here.Length - 1].Attr & Cell.Wrapped) != 0;
                if (!wrapped && !UrlContinues(here, next, ref inUrl)) break;
                l2++;
                c2 = 0;
                while (c2 < next.Length - 1 && IsBlank(next[c2].Ch)) c2++;
                while (c2 < next.Length - 1 && (IsUrlChar(next[c2 + 1].Ch) || next[c2 + 1].Ch == -1)) c2++;
            }
        }

        string LinkAt(Point at)
        {
            int line, col;
            CellAt(at, out line, out col);
            lock (_vt.Lock)
            {
                var l = _vt.Line(line - _vt.ScrollbackCount);
                if (l == null || col >= l.Length || !IsUrlChar(l[col].Ch)) return null;
                int l1, c1, l2, c2;
                WordExtent(line, col, out l1, out c1, out l2, out c2);
                string text = LinesText(l1, c1, l2, c2).Trim().TrimEnd('.', ',', ';', ':', ')', ']', '\'');
                return text.IndexOf("://", StringComparison.Ordinal) > 0 ? text : null;
            }
        }

        void SendText(string s)
        {
            if (!_sessionRunning || s.Length == 0 || _active == null) return;
            var b = Encoding.UTF8.GetBytes(s);
            Engine.tx_session_write(_active.Id, b, b.Length);
            _scrollOffset = 0;
        }

        void SendTyped(string s)
        {
            foreach (char ch in s)
            {
                string o = ch.ToString();
                if (_ctrl)
                {
                    char u = char.ToUpperInvariant(ch);
                    if (u >= '@' && u <= '_') o = ((char)(u - 64)).ToString();
                    else if (ch == ' ') o = "\0";
                    else if (ch == '?') o = "\x7f";
                    SetSticky(ref _ctrl, CtrlKey, false);
                }
                if (_alt)
                {
                    o = "\x1b" + o;
                    SetSticky(ref _alt, AltKey, false);
                }
                SendText(o);
            }
        }

        void Input_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_editing) return;
            string t = Input.Text;
            if (t == Sentinel) return;
            if (t.StartsWith(Sentinel)) SendTyped(t.Substring(1));
            else if (t.Length == 0) SendText("\x7f");
            else SendTyped(t.EndsWith(Sentinel) ? t.Substring(0, t.Length - 1) : t);
            _editing = true;
            Input.Text = Sentinel;
            Input.SelectionStart = 1;
            _editing = false;
        }

        void Input_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            switch (e.Key)
            {
                case VirtualKey.Enter:
                    SendTyped("\r");
                    e.Handled = true;
                    break;
                case VirtualKey.Up: SendKey("up"); e.Handled = true; break;
                case VirtualKey.Down: SendKey("down"); e.Handled = true; break;
                case VirtualKey.Left: SendKey("left"); e.Handled = true; break;
                case VirtualKey.Right: SendKey("right"); e.Handled = true; break;
                case VirtualKey.Tab: SendKey("tab"); e.Handled = true; break;
                case VirtualKey.Escape: SendKey("esc"); e.Handled = true; break;
            }
        }

        void SetSticky(ref bool flag, Button b, bool on)
        {
            flag = on;
            b.Background = on ? new SolidColorBrush(Accent) : new SolidColorBrush(Colors.Transparent);
        }

        void Key_Click(object sender, RoutedEventArgs e)
        {
            string tag = (string)((Button)sender).Tag;
            if (tag == "ctrl") SetSticky(ref _ctrl, CtrlKey, !_ctrl);
            else if (tag == "alt") SetSticky(ref _alt, AltKey, !_alt);
            else if (tag.Length == 1) SendTyped(tag);
            else SendKey(tag);
        }

        void SendKey(string key)
        {
            string csi = _vt.AppCursorKeys ? "\x1bO" : "\x1b[";
            string s = null;
            switch (key)
            {
                case "esc": s = "\x1b"; break;
                case "tab": s = "\t"; break;
                case "up": s = csi + "A"; break;
                case "down": s = csi + "B"; break;
                case "right": s = csi + "C"; break;
                case "left": s = csi + "D"; break;
                case "home": s = csi + "H"; break;
                case "end": s = csi + "F"; break;
                case "pgup": s = "\x1b[5~"; break;
                case "pgdn": s = "\x1b[6~"; break;
            }
            if (_alt && s != null)
            {
                s = "\x1b" + s;
                SetSticky(ref _alt, AltKey, false);
            }
            if (_ctrl) SetSticky(ref _ctrl, CtrlKey, false);
            if (s != null) SendText(s);
        }
    }
}
