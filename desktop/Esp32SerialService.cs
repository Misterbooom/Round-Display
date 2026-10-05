using System;
using System.Globalization;
using System.IO.Ports;
using System.Linq;
using Microsoft.Extensions.Logging;

namespace desktop;

public class Esp32SerialService : IDisposable
{
    private SerialPort? _serialPort;
    private readonly ILogger<Esp32SerialService> _logger;

    public Esp32SerialService(ILogger<Esp32SerialService> logger)
    {
        _logger = logger;
        Connect();
    }

    public void SendTelemetry(TelemetryData telemetry, string type)
    {
        if (_serialPort is not { IsOpen: true })
        {
            Connect();
            if (_serialPort is not { IsOpen: true }) return;
            _logger.LogInformation("Serial port is closed.");
        }

        try
        {
            _logger.LogInformation("Sending telemetry.");
            
            string message = string.Create(
                CultureInfo.InvariantCulture,
                $"PC_TELEMETRY_{type}: {telemetry.LoadPercentage}, {telemetry.Temperature}");

            _serialPort.WriteLine(message);
        }
        catch (Exception)
        {
            Disconnect();
        }
    }

    private void Connect()
    {
        Disconnect();

        string[] ports = SerialPort.GetPortNames()
            .Where(p => p.StartsWith("COM") || p.Contains("ttyUSB") || p.Contains("ttyACM"))
            .ToArray();

        foreach (string portName in ports)
        {
            SerialPort? candidate = null;
            try
            {
                candidate = new SerialPort(portName, 115200)
                {
                    ReadTimeout = 400,
                    WriteTimeout = 400,
                    DtrEnable = true,
                    RtsEnable = true
                };

                candidate.Open();
                candidate.DiscardInBuffer();
                candidate.WriteLine("PING");
                _logger.LogInformation("Sending PING");

                string response = candidate.ReadLine().Trim();
                if (response == "PONG")
                {
                    _serialPort = candidate;
                    _logger.LogInformation("PONG");
                    return;
                }
                _logger.LogError("Response: {Response}", response);

                candidate.Dispose();
            }
            catch
            {
                candidate?.Dispose();
            }
        }
    }

    private void Disconnect()
    {
        try
        {
            _serialPort?.Dispose();
        }
        catch
        {
            // ignores errors on disposing device
        }
        finally
        {
            _serialPort = null;
        }
    }

    public void Dispose()
    {
        Disconnect();
    }
}