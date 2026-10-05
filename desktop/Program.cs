using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;

namespace desktop;

internal class Program
{
    private static async Task Main(string[] args)
    {
        IHost host = Host.CreateDefaultBuilder(args)
            .UseSystemd()
            .ConfigureServices(services =>
            {
                services.AddSingleton<IHardwareMonitor, LinuxHardwareMonitor>();
                services.AddSingleton<Esp32SerialService>(); 
                services.AddHostedService<Worker>(); 
            })
            .Build();

        await host.RunAsync();
    }
}
