using System;
using System.Threading;
using System.Windows;

using System.Net;
using System.Net.Sockets;

using BaseStation.Packet;
using JoystickLibrary;
using System.Timers;
using System.Windows.Media;
using System.Windows.Controls;
using System.ComponentModel;
using System.IO;
using System.Drawing;

namespace BaseStation
{
    public enum DriveMode
    {
        Direct = 1,
        Remote = 2,
        AI = 3,
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int PKT_PORT = 10000;
        const int IMG_PORT = 11000;
        const double QUERY_HEARTBEAT_INTERVAL = 1e3;
        const int JS_DEADZONE = 15;

        Logger logger;
        UdpClient packetSocket;
        TcpListener imgListener;
        Xbox360Service xboxService;
        System.Timers.Timer queryHeartbeatTimer;

        DriveMode currentDriveMode;
        IPEndPoint robotPacketEndpoint;

        Thread connectionThread;
        Thread joystickReadThread;
        Thread imgListenerThread;

        bool windowClosing;
        bool isConnected;
        bool receivedDriveModeAck;

        DateTime lastHeartbeatReceived;
        PacketHandler handler;

        public MainWindow()
        {
            InitializeComponent();
            logger = Logger.GetInstance();

            handler = new PacketHandler();
            lastHeartbeatReceived = DateTime.MinValue;
            receivedDriveModeAck = false;

            xboxService = new Xbox360Service(1);
            bool xbox_init_ok = xboxService.Initialize() && xboxService.Start();
            if (!xbox_init_ok)
                logger.Write(LoggerLevel.Warning, "Failed to initialize the Xbox360Service. Joysticks are inoperable for this session.");

            packetSocket = new UdpClient(PKT_PORT);
            packetSocket.BeginReceive(PacketReceiveCallback, null);

            imgListener = new TcpListener(IPAddress.Any, IMG_PORT);
            imgListener.Start();

            imgListenerThread = new Thread(ImageListenerThread);
            imgListenerThread.SetApartmentState(ApartmentState.STA);
            imgListenerThread.Start();

            queryHeartbeatTimer = new System.Timers.Timer(QUERY_HEARTBEAT_INTERVAL);
            queryHeartbeatTimer.Elapsed += new ElapsedEventHandler(QueryHeartbeatEvent);
            queryHeartbeatTimer.Enabled = true;

            isConnected = false;
            windowClosing = false;
            currentDriveMode = DriveMode.Direct;

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

            Opcode opcode = handler.GetPacketOpcode(buffer, buffer.Length);

            switch (opcode)
            {
                case Opcode.REPORT_HEARTBEAT:
                    lastHeartbeatReceived = DateTime.Now;
                    logger.Write(LoggerLevel.Info, "ReportHeartbeat received at " + DateTime.Now.ToString("h:mm:ss tt"));
                    break;

                case Opcode.REMOTE_SWITCH:
                    currentDriveMode = DriveMode.Remote;
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        for (int i = 0; i < robotModeListBox.Items.Count; i++)
                        {
                            var boxItem = robotModeListBox.Items[i] as ComboBoxItem;
                            var item = boxItem.Content as string;
                            if (item == "Remote Drive")
                            {
                                robotModeListBox.SelectedIndex = i;
                                break;
                            }
                        }
                    }));

                    logger.Write(LoggerLevel.Warning, "LAME detected AI timeout and has reverted to manual control.");
                    break;

                case Opcode.SWITCH_MODE_ACK:
                    receivedDriveModeAck = true;
                    break;

                default:
                    return;
            }
        }

        void ImageListenerThread()
        {
            while (!windowClosing)
            {
                int size = -1;

                TcpClient client;
                try
                {
                    client = imgListener.AcceptTcpClient();
                }
                catch (SocketException)
                {
                    continue;
                }

                NetworkStream stream = client.GetStream();
                logger.Write(LoggerLevel.Info, "Microscope image is incoming!");
                // read img size from connection
                byte[] hdr_pkt = new byte[16];
                int bytes = stream.Read(hdr_pkt, 0, 16);
                if (hdr_pkt[0] == 0xAA && hdr_pkt[5] == 0x7F)
                    size = (hdr_pkt[1] << 24) | (hdr_pkt[2] << 16) | (hdr_pkt[3] << 8) | hdr_pkt[4];
                else
                    size = 32768;

                // read the data - credit to Jon Skeet
                int read = 0;
                int chunk;
                byte[] buffer = new byte[size];
                while ((chunk = stream.Read(buffer, read, buffer.Length - read)) > 0)
                {
                    read += chunk;

                    // If we've reached the end of our buffer, check to see if there's
                    // any more information
                    if (read == buffer.Length)
                    {
                        int nextByte = stream.ReadByte();

                        // End of stream? If so, we're done
                        if (nextByte == -1)
                            break;

                        // Nope. Resize the buffer, put in the byte we've just
                        // read, and continue
                        byte[] newBuffer = new byte[buffer.Length * 2];
                        Array.Copy(buffer, newBuffer, buffer.Length);
                        newBuffer[read] = (byte)nextByte;
                        buffer = newBuffer;
                        read++;
                    }
                }

                // if it grew, then resize to correct size
                byte[] ret = new byte[read];
                Array.Copy(buffer, ret, read);

                // now, display the image
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    using (var ms = new MemoryStream(ret))
                    {
                        Bitmap bmpImage = (Bitmap)Bitmap.FromStream(ms);
                        cameraImage.Source = Util.ConvertBitmapToBitmapImage(bmpImage);
                    }
                }));

                // close the connection
                client.Close();
            }
        }

        void QueryHeartbeatEvent(object sender, ElapsedEventArgs e)
        {
            if (!isConnected || robotPacketEndpoint == null)
                return;

            byte[] buffer = handler.GetQueryHeartbeatPacket();
            packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
        }

        public void ConnectingWorker()
        {
            bool queryHeartbeatState = !isConnected; // if not connected, start here
            bool setDriveModeState = isConnected; // if already connected, start in this state
            bool done = false;
            lastHeartbeatReceived = DateTime.MinValue;
            receivedDriveModeAck = false;

            while (!done)
            {
                if (queryHeartbeatState)
                {
                    if (lastHeartbeatReceived != DateTime.MinValue)
                    {
                        queryHeartbeatState = false;
                        setDriveModeState = true;
                    }
                    else
                    {
                        byte[] buffer = handler.GetQueryHeartbeatPacket();
                        packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
                    }
                }
                else if (setDriveModeState)
                {
                    byte[] buffer;
                    switch (currentDriveMode)
                    {
                        case DriveMode.Remote:
                            buffer = handler.GetRemoteSwitchPacket();
                            break;
                        case DriveMode.AI:
                            buffer = handler.GetAiInitPacket();
                            break;
                        default:
                            buffer = new byte[0];
                            break;
                    }

                    packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
                    if (receivedDriveModeAck)
                    {
                        isConnected = true;
                        done = true;
                        SetRobotConnection(true);
                        Dispatcher.BeginInvoke(new Action(() => { requestCameraButton.IsEnabled = true; }));
                    }
                }
                else
                {
                    logger.Write(LoggerLevel.Error, "Invalid connection state detected. Connection thread exiting.");
                    return;
                }

                Thread.Sleep(150);
            }
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

                    Drive speeds = new Drive();
                    speeds.left = leftY;
                    speeds.right = rightY;
                    byte[] buffer = handler.GetDrivePacket(speeds);
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

            logger.Write(LoggerLevel.Info, "Starting connection sequence with BLER (" + addr + ") with drive mode: " + currentDriveMode);
            connectionThread.Start();
        }

        void RequestImageClicked(object sender, RoutedEventArgs e)
        {
            if (!isConnected)
                return;

            logger.Write(LoggerLevel.Info, "Requesting image from BLER...");
            // TODO: implement packet
        }

        void WindowClosing(object sender, CancelEventArgs e)
        {
            windowClosing = true;
            imgListener.Stop();
            
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
