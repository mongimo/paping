#include "standard.h"

using namespace std;


void printError(int error);
int printConnectInfo(host_c host);
int printSuccessfulConnection(host_c host, double time);
int printFailedConnection(int failtype);
int printStats();

void signalHandler(int id);


stats_c	stats;


int main(int argc, pc_t argv[])
{
	host_c		host;
	int			result;
	arguments_c	arguments;

	stats.Attempts	= 0;
	stats.Connects	= 0;
	stats.Failures	= 0;
	stats.Minimum	= 0.0;
	stats.Maximum	= 0.0;
	stats.Total		= 0.0;

	arguments_c::PrintBanner();

	result = arguments_c::Process(argc, argv, arguments);

	if (result != SUCCESS)
	{
		arguments_c::PrintUsage();
		return ERROR_INVALIDARGUMENTS;
	}

	result = socket_c::Resolve(arguments.Destination, host);

	if (result == SUCCESS)
	{
		socket_c::SetPortAndType(arguments.Port, arguments.Type, host);

		if (result != SUCCESS)
		{
			printError(result);
			return result;
		}

		result = printConnectInfo(host);

		if (result != SUCCESS)
		{
			printError(result);
			return result;
		}
	}
	else
	{
		printError(result);
		return result;
	}

	signal(SIGINT, &signalHandler);


	unsigned int	i		= 0;

	double			time	= 0.0;

	while (arguments.Continous || i < (unsigned int)arguments.Count)
	{
		result = socket_c::Connect(host, arguments.Timeout, time);

		stats.Attempts++;

		if (result == SUCCESS)
		{
			stats.Connects++;
			stats.Total += time;
			stats.UpdateMaxMin(time);

			printSuccessfulConnection(host, time);
		}
		else
		{
			stats.Failures++;

			printFailedConnection(result);
		}

		#ifdef WIN32	// Windows cannot sleep to that accuracy (I think!)
			if ((int)time < arguments.Timeout) Sleep((arguments.Timeout - (int)time));
		#else
			if ((int)time < arguments.Timeout) usleep((arguments.Timeout - (int)time) * 1000);
		#endif

		i++;
	}


	printStats();	

	return SUCCESS;
}


void signalHandler(int id)
{
	switch (id)
	{
		case SIGINT:
			printStats();
			exit(0);
			return;
	}
}


void printError(int error)
{
	cerr << "ERROR: " << i18n_c::GetString(error) << " (" << error << ")" << endl;
}


int printConnectInfo(host_c host)
{
	int		length	= 0;

	length = host.GetConnectInfoString(NULL);

	pc_t	info	= new (nothrow) char[length + 1];

	if (info == 0)
	{
		info = NULL;
		return ERROR_POUTOFMEMORY;
	}

	host.GetConnectInfoString(info);

	cout << info << endl << endl;

	delete[] info;

	return SUCCESS;
}


int printSuccessfulConnection(host_c host, double time)
{
	int		length	= 0;

	length = host.GetSuccessfulConnectionString(NULL, time);

	pc_t	data	= new (nothrow) char[length + 1];

	if (data == 0)
	{
		data = NULL;
		return ERROR_POUTOFMEMORY;
	}

	host.GetSuccessfulConnectionString(data, time);

	cout << data << endl;

	delete[] data;

	return SUCCESS;
}


int printStats()
{
	int	length = 0;

	length = stats.GetStatisticsString(NULL);

	pc_t	str	= new (nothrow) char[length + 1];

	if (str == 0)
	{
		str = NULL;
		return ERROR_POUTOFMEMORY;
	}

	stats.GetStatisticsString(str);

	cout << str << endl;

	delete[] str;

	return SUCCESS;
}


int printFailedConnection(int error)
{
	cout << i18n_c::GetString(error) << endl;

	return SUCCESS;
}