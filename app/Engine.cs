using System;
using System.IO;
using System.Runtime.InteropServices;
using Windows.ApplicationModel;
using Windows.Storage;

namespace Luminal
{

    static class Engine
    {
        [DllImport("TxEngine.dll", CharSet = CharSet.Unicode)] static extern int tx_init(string root, string musl);
        [DllImport("TxEngine.dll", CharSet = CharSet.Unicode)] static extern int tx_import(string tar);
        [DllImport("TxEngine.dll")] public static extern void tx_trace(int on);
        [DllImport("TxEngine.dll")] public static extern int tx_session_open(byte[] cmd, int rows, int cols);
        [DllImport("TxEngine.dll")] public static extern void tx_wait(int timeoutMs);
        [DllImport("TxEngine.dll")] public static extern int tx_session_read(int id, byte[] buf, int max);
        [DllImport("TxEngine.dll")] public static extern void tx_session_write(int id, byte[] buf, int n);
        [DllImport("TxEngine.dll")] public static extern void tx_session_resize(int id, int rows, int cols);
        [DllImport("TxEngine.dll")] public static extern int tx_session_status(int id);
        [DllImport("TxEngine.dll")] public static extern void tx_session_close(int id);

        public static string Root => Path.Combine(ApplicationData.Current.LocalFolder.Path, "tx");

        public static string Prepare()
        {
            string pkg = Package.Current.InstalledLocation.Path;
            int r = tx_init(Root, Path.Combine(pkg, "musl.so"));
            if (r != 0) return "Engine failed to start (" + r + ").";
            string marker = Path.Combine(Root, ".imported");
            if (!File.Exists(marker))
            {
                r = tx_import(Path.Combine(pkg, "root.tar"));
                if (r != 0) return "Couldn't unpack Alpine (" + r + ").";
                File.WriteAllText(marker, "3.24.3");
            }
            return null;
        }
    }
}
