namespace desktop;

public interface IHardwareMonitor
{
    ValueTask<TelemetryData> GetCpuTelemetry(CancellationToken ct = default);
    ValueTask<TelemetryData> GetGpuTelemetry(CancellationToken ct = default);
}