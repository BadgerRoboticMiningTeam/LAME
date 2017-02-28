using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace BaseStation
{
    enum DriveMode : int
    {
        Direct = 1,
        Remote = 2,
        AI = 3
    }


    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int ROBOT_PORT = 10000;

        bool shutting_down;
        bool connected;
        UdpClient socket;
        Logger logger;
        Thread queryHeartbeatThread;
        IPEndPoint robotIP;

        public MainWindow()
        {
            InitializeComponent();
            shutting_down = false;
            logger = Logger.GetInstance();
            socket = new UdpClient(ROBOT_PORT);

            // ping robot - don't forget to call Start()
            queryHeartbeatThread = new Thread(PingRobot);
        }


        #region Reader Threads & Callbacks
        void ConnectingWorker()
        {
            // connection FSM
        }

        void PingRobot()
        { 
            while (!shutting_down)
            {
                Thread.Sleep(15000);
                if (!connected)
                    continue;

                byte[] query = Packet.GetQueryHeartbeatPacket();
                socket.Send(query, query.Length, robotIP);
            }
        }

        void OnRobotDataReceive(IAsyncResult result)
        {
            IPEndPoint recAddr = new IPEndPoint(IPAddress.Any, 0);
            byte[] buffer = socket.EndReceive(result, ref recAddr);

            int opcode = 0;
            if (!Packet.ParseHeader(buffer, buffer.Length, ref opcode)) 
            {
                logger.Write(LoggerLevel.Warning, "Dropped a packet with invalid header!");
                return;
            }
            
            switch (opcode)
            {
                case Packet.REPORT_HEARTBEAT:
                    
                    break;
                default:
                    return;
            }

            socket.BeginReceive(OnRobotDataReceive, null);
        }
        #endregion

        #region Joystick Region
        void UpdateJoystickDependentValues()
        {
            // TODO: get joystick values
        }
        #endregion

        #region GUI Event Handlers
        void GoButtonClicked(object sender, RoutedEventArgs e)
        {
            // TODO: connection process
        }

        void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            shutting_down = true;
            // TODO: cleanup

            if (queryHeartbeatThread.IsAlive)
                queryHeartbeatThread.Join();

        }
        #endregion

        #region GUI Conversion & Utility Methods
        bool GetDriveMode(ref DriveMode mode)
        {
            if (robotModeListBox.SelectedIndex < 0 || robotModeListBox.SelectedIndex >= robotModeListBox.Items.Count)
                return false;

            var boxItem = robotModeListBox.Items[robotModeListBox.SelectedIndex] as ComboBoxItem;
            var item = boxItem.Content as string;
            if (item == null)
                return false;

            switch (item)
            {
                case "Direct Drive":
                    mode = DriveMode.Direct;
                    break;
                case "Remote Drive":
                    mode = DriveMode.Remote;
                    break;
                case "AI":
                    mode = DriveMode.AI;
                    break;
                default:
                    // error
                    return false;
            }

            return true;
        }

        void SetRobotConnection(bool connected)
        {
            Dispatcher.Invoke(new Action(() => { connectedStatus.Fill = new SolidColorBrush(connected ? Colors.Green : Colors.Red); }));
        }
        #endregion
    }
}
