using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;

using System.Net;
using System.Net.Sockets;

using BaseStation.Packet;
using JoystickLibrary;
using Emgu.CV;
using Emgu.CV.Structure;
using System.Timers;
using System.Windows.Media;
using System.Windows.Controls;

namespace BaseStation
{
    public enum DriveMode
    {
        Direct = 1,
        Remote = 2,
        AI = 3,
        Unknown = -1
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int PKT_PORT = 10000;
        const int IMG_PORT = 11000;
        const double QUERY_HEARTBEAT_INTERVAL = 10e3;
        const int JS_DEADZONE = 15;

        Logger logger;
        UdpClient packetSocket;
        UdpClient imageSocket;
        Xbox360Service xboxService;
        System.Timers.Timer queryHeartbeatTimer;
        bool isConnected;
        DriveMode currentDriveMode;
        IPEndPoint robotPacketEndpoint;
        Thread connectionThread;
        Thread joystickReadThread;
        bool windowClosing;

        public MainWindow()
        {
            InitializeComponent();
            logger = Logger.GetInstance();

            xboxService = new Xbox360Service(1);
            bool xbox_init_ok = xboxService.Initialize() && xboxService.Start();
            if (!xbox_init_ok)
                logger.Write(LoggerLevel.Warning, "Failed to initialize the Xbox360Service. Joysticks are inoperable for this session.");

            packetSocket = new UdpClient(PKT_PORT);
            packetSocket.BeginReceive(PacketReceiveCallback, null);

            imageSocket = new UdpClient(IMG_PORT);
            imageSocket.BeginReceive(ImageReceiveCallback, null);

            queryHeartbeatTimer = new System.Timers.Timer(QUERY_HEARTBEAT_INTERVAL);
            queryHeartbeatTimer.Elapsed += new ElapsedEventHandler(QueryHeartbeatEvent);
            queryHeartbeatTimer.Enabled = true;

            isConnected = false;
            windowClosing = false;
            currentDriveMode = DriveMode.Unknown;

            connectionThread = new Thread(ConnectingWorker);

            joystickReadThread = new Thread(ReadJoystickValues);
            joystickReadThread.IsBackground = true;
            joystickReadThread.Start();
        }

        #region Threads & Callbacks
        void PacketReceiveCallback(IAsyncResult ar)
        {
            IPEndPoint endpoint = new IPEndPoint(IPAddress.Any, 0);
            byte[] buffer = packetSocket.EndReceive(ar, ref endpoint);
            packetSocket.BeginReceive(PacketReceiveCallback, null);

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

        void ImageReceiveCallback(IAsyncResult ar)
        {
            IPEndPoint endpoint = new IPEndPoint(IPAddress.Any, 0);
            byte[] buffer = imageSocket.EndReceive(ar, ref endpoint);
            imageSocket.BeginReceive(PacketReceiveCallback, null);

            Dispatcher.BeginInvoke(new Action(() => 
            {
                Image<Gray, byte> depthImage = new Image<Gray, byte>(640, 480);
                depthImage.Bytes = buffer;

                cameraImage.Source = Util.ToBitmapSource(depthImage);
            }));
        }

        void QueryHeartbeatEvent(object sender, ElapsedEventArgs e)
        {
            if (!isConnected || robotPacketEndpoint == null)
                return;

            QueryHeartbeat qhb = new QueryHeartbeat();
            byte[] buffer = qhb.Serialize();
            packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
        }

        public void ConnectingWorker()
        {
            /* make FSM
            // remember to enable the RequestCameraImage button!
            bool queryHeartbeatState = true;
            while (!isConnected)
            {
                Thread.Sleep(250);

                // done
                isConnected = true;
                //currentDriveMode = nextDriveMode;
                SetRobotConnection(isConnected);
            }
            */
        }

        void ReadJoystickValues()
        {
            int jsID = -1;
            int leftY = 0;
            int rightY = 0;
            bool enumerate_js = true;

            while (!windowClosing)
            {
                Thread.Sleep(25);

                if (enumerate_js)
                {
                    var ids = xboxService.GetJoystickIDs();
                    if (ids.Count < 1)
                        continue;

                    // we found one
                    enumerate_js = false;
                    jsID = ids[0];

                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        jsIdTextBox.Text = jsID.ToString();
                    }));
                }
                else
                {
                    if (!xboxService.GetJoystickIDs().Contains(jsID))
                    {
                        // return to enumerate state
                        enumerate_js = true;
                        Dispatcher.BeginInvoke(new Action(() =>
                        {
                            jsIdTextBox.Text = "N/A";
                        }));

                        continue;
                    }

                    xboxService.GetLeftY(jsID, ref leftY);
                    xboxService.GetRightY(jsID, ref rightY);

                    if (Math.Abs(leftY) < JS_DEADZONE)
                        leftY = 0;
                    if (Math.Abs(rightY) < JS_DEADZONE)
                        rightY = 0;

                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        leftYTextBox.Text = leftY.ToString();
                        rightYTextBox.Text = rightY.ToString();
                    }));

                    if (!isConnected || currentDriveMode != DriveMode.Remote)
                        continue;

                    Drive speeds = new Drive(leftY, rightY);
                    byte[] buffer = speeds.Serialize();
                    packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
                }
            }
        }
        #endregion

        #region GUI Events
        void GoButtonClicked(object sender, RoutedEventArgs e)
        {
            if (isConnected)
            {
                // only update drive mode
                if (!GetDriveMode(ref currentDriveMode))
                {
                    logger.Write(LoggerLevel.Error, "Invalid drive mode selected.");
                    return;
                }

                logger.Write(LoggerLevel.Info, "Updating drive mode to " + currentDriveMode);
                isConnected = false;
                connectionThread.Start();
                return;
            }

            // get & set endpoint
            IPAddress addr;
            if (!IPAddress.TryParse(robotIpTextBox.Text, out addr))
            {
                logger.Write(LoggerLevel.Error, "Invalid IP address!");
                return;
            }

            robotPacketEndpoint = new IPEndPoint(addr, PKT_PORT);

            // set drive mode
            if (!GetDriveMode(ref currentDriveMode))
            {
                logger.Write(LoggerLevel.Error, "Invalid drive mode selected.");
                return;
            }

            logger.Write(LoggerLevel.Info, "Starting connection sequence with Ascent (" + addr + ") with drive mode: " + currentDriveMode);
            connectionThread.Start();
        }

        void RequestImageClicked(object sender, RoutedEventArgs e)
        {
            if (!isConnected)
                return;

            logger.Write(LoggerLevel.Info, "Requesting image from BLER...");
            // TODO: implement packet
        }

        void WindowClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            windowClosing = true;

            xboxService.Stop();
            if (joystickReadThread.IsAlive)
                joystickReadThread.Abort();
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
            Dispatcher.BeginInvoke(new Action(() => { connectedStatus.Fill = new SolidColorBrush(connected ? Colors.Green : Colors.Red); }));
        }
        #endregion
    }
}
