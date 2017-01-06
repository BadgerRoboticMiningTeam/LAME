using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Net;
using System.Net.Sockets;

using BaseStation.Packet;

namespace BaseStation
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int LAME_PORT = 15000;
        Logger logger;
        UdpClient socket;
        UdpClient imageSocket;

        public MainWindow()
        {
            InitializeComponent();
            logger = Logger.GetInstance();

            socket = new UdpClient(LAME_PORT);
            socket.BeginReceive(ReceiveCallback, null);
        }

        public void ReceiveCallback(IAsyncResult ar)
        {
            IPEndPoint endpoint = new IPEndPoint(IPAddress.Any, 0);
            byte[] buffer = socket.EndReceive(ar, ref endpoint);
            socket.BeginReceive(ReceiveCallback, null);

            PacketOpcode opcode = PacketOpcode.Drive;
            if (!Packet.Packet.GetOpcodeFromBuffer(buffer, ref opcode))
                return;

            switch (opcode)
            {
                case PacketOpcode.ReportHeartbeat:
                    logger.Write(LoggerLevel.Info, "ReportHeartbeat received at " + DateTime.Now.ToString("h:mm:ss tt"));
                    break;
            }
        }
    }
}
