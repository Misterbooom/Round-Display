using System;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace desktop;

public class LinuxHardwareMonitor : IHardwareMonitor
{
    private ulong _prevIdle;
    private ulong _prevTotal;
    private readonly string? _cpuTempFilePath;
    private readonly SemaphoreSlim _gate = new(1, 1);

    public LinuxHardwareMonitor()
    {
        (_prevIdle, _prevTotal) = ReadCpuStat();
        _cpuTempFilePath = FindCpuTempSensorPath();
    }

    public async ValueTask<TelemetryData> GetCpuTelemetry(CancellationToken ct = default)
    {
        await _gate.WaitAsync(ct).ConfigureAwait(false);
        try
        {
            return await Task.Run(() =>
            {
                ct.ThrowIfCancellationRequested();
                double cpuUsage = CalculateCpuUsage();
                double cpuTemp = ReadCpuTemperature();
                return new TelemetryData(cpuTemp, cpuUsage);
            }, ct).ConfigureAwait(false);
        }
        finally
        {
            _gate.Release();
        }
    }

    public ValueTask<TelemetryData> GetGpuTelemetry(CancellationToken ct = default)
    {
        return NvidiaSmiApi.GetGpuTelemetryAsync(ct);
    }

    private double CalculateCpuUsage()
    {
        var (currentIdle, currentTotal) = ReadCpuStat();

        ulong deltaIdle = currentIdle - _prevIdle;
        ulong deltaTotal = currentTotal - _prevTotal;

        _prevIdle = currentIdle;
        _prevTotal = currentTotal;

        if (deltaTotal == 0)
            return 0.0;

        return Math.Round((1.0 - (double)deltaIdle / deltaTotal) * 100.0, 2);
    }

    private static (ulong Idle, ulong Total) ReadCpuStat()
    {
        using var reader = new StreamReader("/proc/stat");
        string? line = reader.ReadLine();

        if (string.IsNullOrEmpty(line))
            throw new InvalidOperationException("Can't read /proc/stat");

        ulong[] values = line
            .Split(' ', StringSplitOptions.RemoveEmptyEntries)
            .Skip(1)
            .Take(8)
            .Select(ulong.Parse)
            .ToArray();

        ulong idle = values[3] + values[4];
        ulong total = values.Aggregate<ulong, ulong>(0, (current, val) => current + val);

        return (idle, total);
    }

    private double ReadCpuTemperature()
    {
        if (_cpuTempFilePath != null &&
            double.TryParse(File.ReadAllText(_cpuTempFilePath).Trim(), out double millidegrees))
        {
            return Math.Round(millidegrees / 1000.0, 1);
        }
        return 0;
    }

    private static string? FindCpuTempSensorPath()
    {
        const string hwmonBase = "/sys/class/hwmon";
        if (Directory.Exists(hwmonBase))
        {
            foreach (var dir in Directory.GetDirectories(hwmonBase))
            {
                string nameFile = Path.Combine(dir, "name");
                if (!File.Exists(nameFile)) continue;

                string sensorName = File.ReadAllText(nameFile).Trim();

                if (sensorName is "coretemp" or "k10temp" or "zenpower" or "cpu_thermal")
                {
                    string tempInput = Path.Combine(dir, "temp1_input");
                    if (File.Exists(tempInput))
                        return tempInput;
                }
            }
        }

        const string thermalBase = "/sys/class/thermal";
        if (Directory.Exists(thermalBase))
        {
            foreach (var dir in Directory.GetDirectories(thermalBase, "thermal_zone*"))
            {
                string typeFile = Path.Combine(dir, "type");
                string tempFile = Path.Combine(dir, "temp");

                if (File.Exists(typeFile) && File.Exists(tempFile))
                {
                    string type = File.ReadAllText(typeFile).Trim();
                    if (type == "x86_pkg_temp")
                        return tempFile;
                }
            }
        }

        const string fallbackPath = "/sys/class/thermal/thermal_zone0/temp";
        return File.Exists(fallbackPath) ? fallbackPath : null;
    }
}