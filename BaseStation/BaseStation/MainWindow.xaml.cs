using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Timers;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

using BaseStation.Packet;
using JoystickLibrarySharp;

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
        const double QUERY_HEARTBEAT_INTERVAL = 15e3;
        const int JS_DEADZONE = 15;
        const int SCOOPER_INC = 25;
        const int VIB_INC = 25;

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
        MemoryStream currentImgStream;

        public MainWindow()
        {
            InitializeComponent();
            logger = Logger.GetInstance();

            handler = new PacketHandler();
            lastHeartbeatReceived = DateTime.MinValue;
            receivedDriveModeAck = false;

            xboxService = new Xbox360Service();
            bool xbox_init_ok = xboxService.Initialize();
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
            byte[] buffer = null;
            try
            {
                buffer = packetSocket.EndReceive(ar, ref endpoint);
            }
            catch (SocketException)
            {
                logger.Write(LoggerLevel.Error, "Connection closed - Robot code is not running or has crashed!");
                connectionThread.Abort();
                return;
            }
            
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
                logger.Write(LoggerLevel.Info, "Image is incoming!");

                // read img size from connection
                
                byte[] hdr_pkt = new byte[16];
                int bytes = stream.Read(hdr_pkt, 0, 16);
                if (hdr_pkt[0] == 0xAA && hdr_pkt[6] == 0x7F)
                    size = (hdr_pkt[1] << 24) | (hdr_pkt[2] << 16) | (hdr_pkt[3] << 8) | hdr_pkt[4];
                else
                    size = 32768;

                int img_id = hdr_pkt[5];

                logger.Write(LoggerLevel.Info, string.Format("Image ({0}) is inbound from BLER!", img_id));

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

                MemoryStream localMs = new MemoryStream(ret);

                Bitmap bmpImage;
                try
                {
                    bmpImage = new Bitmap(localMs);
                }
                catch (ArgumentException)
                {
                    logger.Write(LoggerLevel.Error, "Failed to parse image!");
                    continue;
                }

                // now, display the image
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    if (img_id == 0)
                        cameraImage.Source = Util.ConvertBitmapToBitmapImage(bmpImage);
                    else
                        camera1Image.Source = Util.ConvertBitmapToBitmapImage(bmpImage);
                }));

                if (currentImgStream != null)
                    currentImgStream.Close();

                currentImgStream = localMs;

                // close the connection
                client.Close();

                logger.Write(LoggerLevel.Info, "Image updated.");
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
                        logger.Write(LoggerLevel.Info, "Mode switched successfully.");
                        isConnected = true;
                        done = true;
                        SetRobotConnection(true);
                        Dispatcher.BeginInvoke(new Action(() => 
                        {
                            requestCameraButton.IsEnabled = true;
                            requestCamera1Button.IsEnabled = true;
                        }));
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
            int leftTrigger = 0;
            int rightTrigger = 0;
            bool enumerate_js = true;
            int scooper_speed = 0;
            int vibrator_speed = 0;

            DateTime lastScooperToggle = DateTime.Now;
            DateTime lastVibratorToggle = DateTime.Now;
            Drive lastPacket = new Drive();

            while (!windowClosing)
            {
                Thread.Sleep(100);

                if (enumerate_js)
                {
                    var ids = xboxService.GetIDs();
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
                    bool a_pressed = false;
                    bool x_pressed = false;
                    int actuator_speed = 0;

                    if (!xboxService.GetIDs().Contains(jsID))
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
                    xboxService.GetLeftTrigger(jsID, ref leftTrigger);
                    xboxService.GetRightTrigger(jsID, ref rightTrigger);
                    xboxService.GetButton(jsID, Xbox360Button.A, ref a_pressed);
                    xboxService.GetButton(jsID, Xbox360Button.X, ref x_pressed);

                    if (Math.Abs(leftY) < JS_DEADZONE)
                        leftY = 0;
                    if (Math.Abs(rightY) < JS_DEADZONE)
                        rightY = 0;

                    if (leftTrigger != 0 && rightTrigger != 0)
                        actuator_speed = 0;
                    else if (leftTrigger != 0)
                        actuator_speed = -leftTrigger;
                    else if (rightTrigger != 0)
                        actuator_speed = rightTrigger;

                    if (a_pressed && (DateTime.Now - lastScooperToggle).Milliseconds > 250)
                    {
                        scooper_speed += SCOOPER_INC;
                        if (scooper_speed > 100)
                            scooper_speed = 0;
                        lastScooperToggle = DateTime.Now;
                    }

                    if (x_pressed && (DateTime.Now - lastVibratorToggle).Milliseconds > 250)
                    {
                        vibrator_speed += VIB_INC;
                        if (vibrator_speed > 100)
                            vibrator_speed = 0;
                        lastVibratorToggle = DateTime.Now;
                    }
                    
                    bool driving = (leftY != 0) || (rightY != 0);
                    bool act_on = actuator_speed != 0;
                    bool vib_on = vibrator_speed != 0;
                    bool zero_all = ((driving && act_on) || (driving && vib_on) || (act_on && vib_on));
                    zero_all = false;

                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        leftYTextBox.Text = zero_all ?  "0" : leftY.ToString();
                        rightYTextBox.Text = zero_all ? "0" : rightY.ToString();
                        binTextBox.Text = zero_all ? "0" : actuator_speed.ToString();
                        scooperTextBox.Text = zero_all ? "0" : scooper_speed.ToString();
                        vibratorTextBox.Text = zero_all ? "0" : vibrator_speed.ToString();
                    }));

                    if (!isConnected || currentDriveMode != DriveMode.Remote)
                        continue;

                    Drive speeds = new Drive();
                    if (!zero_all)
                    {
                        speeds.left = leftY;
                        speeds.right = rightY;
                        speeds.actuator = actuator_speed;
                        speeds.scooper = scooper_speed;
                        speeds.vibrator = vibrator_speed;
                    }

                    // no change? don't send another packet
                    //if (lastPacket == speeds)
                    //    continue;

                    // update last packet, and send out
                    lastPacket = speeds;
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

                StartConnection();
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

            // start connection - if can't start, ignore 
            if (StartConnection())
                logger.Write(LoggerLevel.Info, "Starting connection sequence with BLER (" + addr + ") with drive mode: " + currentDriveMode);
        }

        bool StartConnection()
        {
            if (connectionThread.IsAlive)
            {
                logger.Write(LoggerLevel.Warning, "Already attempting to connect to BLER, ignoring additional press.");
                return false;
            }

            connectionThread = new Thread(new ThreadStart(ConnectingWorker));
            connectionThread.Start();
            return true;
        }

        void RequestImageClicked(object sender, RoutedEventArgs e)
        {
            if (!isConnected)
                return;

            logger.Write(LoggerLevel.Info, "Requesting image (0) from BLER...");
            byte[] buffer = handler.GetQueryCameraImagePacket();
            packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
        }

        void RequestImage1Clicked(object sender, RoutedEventArgs e)
        {
            if (!isConnected)
                return;

            logger.Write(LoggerLevel.Info, "Requesting image (1) from BLER...");
            byte[] buffer = handler.GetQueryCamera1ImagePacket();
            packetSocket.Send(buffer, buffer.Length, robotPacketEndpoint);
        }

        void WindowClosing(object sender, CancelEventArgs e)
        {
            windowClosing = true;
            xboxService.Dispose();
            imgListener.Stop();
            
            if (connectionThread.IsAlive)
                connectionThread.Abort();

            if (joystickReadThread.IsAlive)
                joystickReadThread.Join();
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
