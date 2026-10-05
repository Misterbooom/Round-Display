using System;
using System.Diagnostics;
using System.Globalization;
using System.Threading;
using System.Threading.Tasks;

namespace desktop;

public static class NvidiaSmiApi
{
    public static async ValueTask<TelemetryData> GetGpuTelemetryAsync(CancellationToken ct = default)
    {
        string? result = await RunNvidiaSmiAsync(
            ["--query-gpu=temperature.gpu,utilization.gpu", "--format=csv,noheader,nounits"],
            ct
        ).ConfigureAwait(false);

        if (string.IsNullOrWhiteSpace(result))
            return new TelemetryData(0, 0);

        string[] parts = result.Trim().Split(',', StringSplitOptions.TrimEntries);

        if (parts.Length >= 2 &&
            double.TryParse(parts[0], NumberStyles.Float, CultureInfo.InvariantCulture, out double temp) &&
            double.TryParse(parts[1], NumberStyles.Float, CultureInfo.InvariantCulture, out double usage))
        {
            return new TelemetryData(temp, usage);
        }
        await Console.Error.WriteLineAsync($"Failed to parse output: {result}");
        return new TelemetryData(0, 0);
    }

    private static async Task<string?> RunNvidiaSmiAsync(string[] args, CancellationToken ct = default)
    {
        var psi = new ProcessStartInfo
        {
            FileName = "nvidia-smi",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };

        foreach (var arg in args)
        {
            psi.ArgumentList.Add(arg);
        }

        try
        {
            using var process = Process.Start(psi);
            if (process == null) return null;

            string output = await process.StandardOutput.ReadToEndAsync(ct).ConfigureAwait(false);
            await process.WaitForExitAsync(ct).ConfigureAwait(false);

            return process.ExitCode == 0 ? output : null;
        }
        catch (Exception ex) when (ex is not OperationCanceledException)
        {
            await Console.Error.WriteLineAsync($"Execution error: {ex.Message}");
            return null;
        }
    }
}