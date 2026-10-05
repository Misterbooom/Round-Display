using System;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace desktop;

public class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;
    private readonly IHardwareMonitor _hardwareMonitor;
    private readonly Esp32SerialService _esp32SerialService;

    public Worker(ILogger<Worker> logger, IHardwareMonitor hardwareMonitor, Esp32SerialService esp32SerialService)
    {
        _logger = logger;
        _hardwareMonitor = hardwareMonitor;
        _esp32SerialService = esp32SerialService;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        _logger.LogInformation("Worker running at: {time}", DateTimeOffset.Now);

        try
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                var cpuData = await _hardwareMonitor.GetCpuTelemetry(stoppingToken);
                var gpuData = await _hardwareMonitor.GetGpuTelemetry(stoppingToken);

                _logger.LogInformation("CPU: {cpuData}", cpuData);
                _logger.LogInformation("GPU: {gpuData}", gpuData);

                _esp32SerialService.SendTelemetry(cpuData, "CPU");
                _esp32SerialService.SendTelemetry(gpuData, "GPU");

                await Task.Delay(1000, stoppingToken);
            }
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("Monitor cancelled");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "An error occurred in the hardware monitor worker");
        }
    }
}